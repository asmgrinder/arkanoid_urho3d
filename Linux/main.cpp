//
// Copyright (c) 2008-2017 the Arkanoid project.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//

#include <string>
#include <sstream>

#include <Urho3D/Core/CoreEvents.h>
#include <Urho3D/Engine/Application.h>
#include <Urho3D/Engine/Engine.h>
#include <Urho3D/Engine/EngineDefs.h>
#include <Urho3D/IO/FileSystem.h>
#include <Urho3D/Input/Input.h>
#include <Urho3D/Input/InputEvents.h>
#include <Urho3D/Resource/ResourceCache.h>
#include <Urho3D/Resource/XMLFile.h>
#include <Urho3D/IO/Log.h>
#include <Urho3D/UI/UI.h>
#include <Urho3D/UI/Text.h>
#include <Urho3D/UI/Font.h>
#include <Urho3D/UI/Window.h>
#include <Urho3D/UI/Button.h>
#include <Urho3D/UI/UIEvents.h>
#include <Urho3D/Scene/Scene.h>
#include <Urho3D/Scene/SceneEvents.h>
#include <Urho3D/Graphics/Graphics.h>
#include <Urho3D/Graphics/Camera.h>
#include <Urho3D/Graphics/Geometry.h>
#include <Urho3D/Graphics/Renderer.h>
#include <Urho3D/Graphics/Zone.h>
#include <Urho3D/Graphics/DebugRenderer.h>
#include <Urho3D/Graphics/Octree.h>
#include <Urho3D/Graphics/Light.h>
#include <Urho3D/Graphics/Model.h>
#include <Urho3D/Graphics/StaticModel.h>
#include <Urho3D/Graphics/Material.h>
#include <Urho3D/Graphics/Skybox.h>
#include <Urho3D/Physics/CollisionShape.h>
#include <Urho3D/Physics/PhysicsWorld.h>
#include <Urho3D/Physics/RigidBody.h>
#include <Urho3D/Physics/Constraint.h>

#include "ball.h"
#include "brick.h"
#include "paddle.h"
#include "bonus.h"

using namespace Urho3D;

const int BASE_WIDTH = 1280;
const int BASE_HEIGHT = 720;
const float SPEED_NORMAL = 1;
const float SPEED_TURBO = 2;
/**
* Using the convenient Application API we don't have
* to worry about initializing the engine or writing a main.
* You can probably mess around with initializing the engine
* and running a main manually, but this is convenient and portable.
*/
class Arkanoid : public Application
{
protected:
    int framecount_;
    float time_;
    SharedPtr<PhysicsWorld> physicsWorld_;
    SharedPtr<Scene> scene_;
    SharedPtr<Node> skyNode_, fieldNode_, fieldBordersNode_, ballNode_, paddleNode_;
    SharedPtr<Node> cameraNode_;
    SharedPtr<Button> pauseButton_;
    SharedPtr<Window> scoresPanel_;
    SharedPtr<Text> scoresText_;
    Vector<SharedPtr<Node> > bricks_;
    Vector<SharedPtr<Node> > bonuses_;

    Vector3 ballOffsetOriginal_;
    Vector3 ballOffset_;
    float velocity_;
    bool paused_;
    unsigned scores_;
public:
    /**
    * This happens before the engine has been initialized
    * so it's usually minimal code setting defaults for
    * whatever instance variables you have.
    * You can also do this in the Setup method.
    */
    Arkanoid(Context * context) : Application(context), framecount_(0), time_(0), velocity_(SPEED_NORMAL), paused_(false), scores_(0)
    {
        Ball::RegisterObject(context);
        Bonus::RegisterObject(context);
        Brick::RegisterObject(context);
        Paddle::RegisterObject(context);
    }

    void handlePause(StringHash eventType, VariantMap& eventData)
    {
        paused_ = !paused_;
        physicsWorld_->SetUpdateEnabled(!paused_);
    }

    void setupPhysicalProperties(RigidBody* rigidBody)
    {
        rigidBody->SetFriction(0);
        rigidBody->SetRollingFriction(0);
        rigidBody->SetLinearDamping(0);
        rigidBody->SetAngularDamping(0);
        rigidBody->SetAngularRestThreshold(0.01f);
        rigidBody->SetLinearRestThreshold(0.01f);
        rigidBody->SetRestitution(1);
        rigidBody->SetUseGravity(false);
    }

    Node* setupNode(const String& model, const String& material, const String& nodeName = String::EMPTY, bool setupShape = true)
    {
        ResourceCache* cache = GetSubsystem<ResourceCache>();
        Node* node = scene_->CreateChild(nodeName);
        //node->SetPosition(Vector3(0, 0, 0));
        Model* objectModel = cache->GetResource<Model>(model);
        StaticModel* staticModel = node->CreateComponent<StaticModel>();
        staticModel->SetModel(objectModel);
        staticModel->SetMaterial(cache->GetResource<Material>(material));
        if (false != setupShape)
        {
            CollisionShape* shape = node->CreateComponent<CollisionShape>();
            shape->SetMargin(0.00001f);
            shape->SetConvexHull(objectModel);
        }
        setupPhysicalProperties(node->CreateComponent<RigidBody>());
        
        return node;
    }

    void clearLevel()
    {
        for (unsigned i = 0; i < bricks_.Size(); i ++)
        {
            if (nullptr != bricks_[i])
            {
                bricks_[i]->Remove();
                bricks_[i].Reset();
            }
        }
        bricks_.Clear();
    }

    void clearActiveBonuses()
    {
        for (unsigned i = 0; i < bonuses_.Size(); i ++)
        {
            SharedPtr<Node>& bonusNode = bonuses_[i];
            if (nullptr != bonusNode
                && false != bonusNode->IsEnabled())
            {
                bonusNode->Remove();
                bonusNode.Reset();
            }
        }
    }
    
    void clearBonuses()
    {
        for (unsigned i = 0; i < bonuses_.Size(); i ++)
        {
            SharedPtr<Node>& bonusNode = bonuses_[i];
            if (nullptr != bonusNode)
            {
                bonusNode->Remove();
                bonusNode.Reset();
            }
        }
        bonuses_.Clear();
    }
    
    void prepareLevel()
    {
        clearBonuses();
        clearLevel();

        Paddle* paddle = paddleNode_->GetComponent<Paddle>();
        paddle->ResetScale();

        ResourceCache* cache = GetSubsystem<ResourceCache>();
        Model* objectModel = cache->GetResource<Model>("Models/Brick_Blue.mdl");
        BoundingBox bbbb = objectModel->GetBoundingBox();
        float width = bbbb.max_.x_ - bbbb.min_.x_;
        float height = bbbb.max_.y_ - bbbb.min_.y_;
        if (width > 0
            && height > 0)
        {
            int countX = int(FIELD_WIDTH / width);
            int countY = int(FIELD_HEIGHT / height);
            float shiftX = 0.5f * width * (countX - 1);
            float shiftY = 0.5f * height * (countY - 1);
            Vector<String> models;
            models.Push(String("Models/Brick_Yellow.mdl"));
            models.Push(String("Models/Brick_Red.mdl"));
            models.Push(String("Models/Brick_Green.mdl"));
            models.Push(String("Models/Brick_Blue.mdl"));
            Vector<String> materials;
            materials.Push(String("Materials/Brick_Yellow.xml"));
            materials.Push(String("Materials/Brick_Red.xml"));
            materials.Push(String("Materials/Brick_Green.xml"));
            materials.Push(String("Materials/Brick_Blue.xml"));
            for (int j = 0; j < countY * 11 / 16; j ++)
            {
                for (int i = 0; i < countX; i ++)
                {
                    float x = shiftX - i * width;
                    float y = shiftY - j * height;
                    int brickIndex = Random(0, models.Size());
                    String model = models[brickIndex];
                    String material = materials[brickIndex];
                    Node* brickNode = setupNode(model, material, "Brick");
                    brickNode->SetPosition(Vector3(x, y, 0));
                    brickNode->CreateComponent<Brick>();
                    bricks_.Push(SharedPtr<Node>(brickNode));

                    unsigned bonusType = Random(BONUS_NONE, BONUS_COUNT);
                    SharedPtr<Node> bonusNode;
                    switch (bonusType)
                    {
                        case BONUS_EXTENDPADDLE:
                            bonusNode = SharedPtr<Node>(setupNode("Models/ExtendPaddle.mdl", "Materials/ExtendPaddle.xml", "Bonus"));
                            break;
                        case BONUS_SHRINKPADDLE:
                            bonusNode = SharedPtr<Node>(setupNode("Models/ShrinkPaddle.mdl", "Materials/ShrinkPaddle.xml", "Bonus"));
                            break;
                        case BONUS_100:
                            bonusNode = SharedPtr<Node>(setupNode("Models/Bonus100.mdl", "Materials/Bonus100.xml", "Bonus"));
                            break;
                        case BONUS_200:
                            bonusNode = SharedPtr<Node>(setupNode("Models/Bonus200.mdl", "Materials/Bonus200.xml", "Bonus"));
                            break;
                        case BONUS_500:
                            bonusNode = SharedPtr<Node>(setupNode("Models/Bonus500.mdl", "Materials/Bonus500.xml", "Bonus"));
                            break;
                        case BONUS_1000:
                            bonusNode = SharedPtr<Node>(setupNode("Models/Bonus1000.mdl", "Materials/Bonus1000.xml", "Bonus"));
                            break;
                        case BONUS_2000:
                            bonusNode = SharedPtr<Node>(setupNode("Models/Bonus2000.mdl", "Materials/Bonus2000.xml", "Bonus"));
                            break;
                        case BONUS_5000:
                            bonusNode = SharedPtr<Node>(setupNode("Models/Bonus5000.mdl", "Materials/Bonus5000.xml", "Bonus"));
                            break;
                        case BONUS_10000:
                            bonusNode = SharedPtr<Node>(setupNode("Models/Bonus10000.mdl", "Materials/Bonus10000.xml", "Bonus"));
                            break;
                    }
                    if (nullptr != bonusNode)
                    {
                        Bonus* bonus = bonusNode->CreateComponent<Bonus>();
                        bonus->SetBonusType(bonusType);
                        RigidBody* bonusBody = bonusNode->GetComponent<RigidBody>();
                        bonusBody->SetTrigger(true);
                        bonusBody->SetMass(0.01f);
                        Vector3 bonusPosition = brickNode->GetPosition();
                        bonusPosition.z_ = 0;
                        bonusNode->SetPosition(bonusPosition);
                        bonusNode->SetEnabled(false);
                    }
                    bonuses_.Push(bonusNode);
                }
            }
        }
    }

    /**
    * This method is called before the engine has been initialized.
    * Thusly, we can setup the engine parameters before anything else
    * of engine importance happens (such as windows, search paths,
    * resolution and other things that might be user configurable).
    */
    virtual void Setup()
    {
        // These parameters should be self-explanatory.
        // See http://urho3d.github.io/documentation/1.7/_main_loop.html
        // for a more complete list.
        engineParameters_[EP_WINDOW_TITLE]      = GetTypeName();
        engineParameters_[EP_LOG_NAME]          = GetSubsystem<FileSystem>()->GetAppPreferencesDir("urho3d", "logs") + GetTypeName() + ".log";
        engineParameters_[EP_FULL_SCREEN]       = false;
        engineParameters_[EP_WINDOW_WIDTH]      = BASE_WIDTH;
        engineParameters_[EP_WINDOW_HEIGHT]     = BASE_HEIGHT;
        engineParameters_[EP_WINDOW_RESIZABLE]  = true;
        //engineParameters_[EP_VSYNC]             = true;
        engineParameters_[EP_TRIPLE_BUFFER]     = true;
        engineParameters_[EP_MATERIAL_QUALITY]  = 2;
        if (!engineParameters_.Contains(EP_RESOURCE_PREFIX_PATHS))
        {
            engineParameters_[EP_RESOURCE_PREFIX_PATHS] = ";../share/Resources;../share/Urho3D/Resources";
        }
    }

    /**
    * This method is called after the engine has been initialized.
    * This is where you set up your actual content, such as scenes,
    * models, controls and what not. Basically, anything that needs
    * the engine initialized and ready goes in here.
    */
    virtual void Start()
    {
        engine_->SetMaxFps(40);
        engine_->SetMaxInactiveFps(10);
        if (GetPlatform() == "Android" || GetPlatform() == "iOS")
        {
            // On mobile platform, enable touch
            engine_->SetMaxFps(40);
        }
        else
        {
            Input* input = GetSubsystem<Input>();
            input->SetTouchEmulation(true);
        }
        // We need to load resources.
        // If the engine can't find them, check the ResourcePrefixPath (see http://urho3d.github.io/documentation/1.7/_main_loop.html).
        ResourceCache* cache = GetSubsystem<ResourceCache>();

        // Let's use the default style that comes with Urho3D.
        UIElement* root = GetSubsystem<UI>()->GetRoot();
        root->SetDefaultStyle(cache->GetResource<XMLFile>("UI/DefaultStyle.xml"));

        pauseButton_ = SharedPtr<Button>(root->CreateChild<Button>());
        pauseButton_->SetStyleAuto();
        pauseButton_->SetSize(220, 55);
        Text* pauseText = pauseButton_->CreateChild<Text>();
        pauseText->SetAlignment(HA_CENTER, VA_CENTER);
        pauseText->SetFont(cache->GetResource<Font>("Fonts/Anonymous Pro.ttf"), 24);
        pauseText->SetText("Pause");
        pauseText->SetTextEffect(TE_SHADOW);
        pauseText->SetEffectShadowOffset(IntVector2(1, 1));
        SubscribeToEvent(pauseButton_, E_PRESSED, URHO3D_HANDLER(Arkanoid, handlePause));

        scoresPanel_ = SharedPtr<Window>(root->CreateChild<Window>());
        scoresPanel_->SetSize(360, 60);
        scoresPanel_->SetColor(Color(1, 1, 1, 0.7f));
        scoresPanel_->SetStyleAuto();
        scoresText_ = SharedPtr<Text>(scoresPanel_->CreateChild<Text>());
        scoresText_->SetText("Score: 0");
        scoresText_->SetColor(Color(0.1f, 0.5f, 0.1f));
        scoresText_->SetFont(cache->GetResource<Font>("Fonts/Anonymous Pro.ttf"), 28);
        scoresText_->SetHorizontalAlignment(HA_CENTER);
        scoresText_->SetVerticalAlignment(VA_CENTER);
        scoresText_->SetTextEffect(TE_STROKE);
        scoresText_->SetEffectStrokeThickness(1);
        scoresText_->SetEffectColor(Color(1, 1, 1, 0.5f));

        // Let's setup a scene to render.
        scene_ = new Scene(context_);
        // add PhysicsWorld component to scene
        physicsWorld_ = scene_->CreateComponent<PhysicsWorld>();
        physicsWorld_->SetGravity(Vector3(0, 0, 0));
        // Let the scene have an Octree component!
        scene_->CreateComponent<Octree>();

        Node* zoneNode = scene_->CreateChild("Zone");
        Zone* zone = zoneNode->CreateComponent<Zone>();
        zone->SetAmbientColor(Color(0.15f, 0.15f, 0.15f));
        zone->SetFogColor(Color(0, 0, 0));
        zone->SetFogStart(1000.0f);
        zone->SetFogEnd(3000.0f);
        zone->SetBoundingBox(BoundingBox(-30.0f, 30.0f));

//         // Let's put some sky in there.
        skyNode_ = scene_->CreateChild("Sky");
        skyNode_->SetScale(8);
        RigidBody* skyBody = skyNode_->CreateComponent<RigidBody>();
        skyBody->SetRollingFriction(0);
        skyBody->SetAngularDamping(0);
        skyBody->SetAngularRestThreshold(0.001f);
        skyBody->SetMass(1);    // should have some mass to rotate
        skyBody->SetUseGravity(false);
        skyBody->SetAngularVelocity(Vector3(0, 0, 0.01f));

        Node* starsPosXNode = skyNode_->CreateChild("StarsPosX");
        StaticModel* starsPosXModel = starsPosXNode->CreateComponent<StaticModel>();
        starsPosXModel->SetModel(cache->GetResource<Model>("Models/Stars_PosX.mdl"));
        starsPosXModel->SetMaterial(cache->GetResource<Material>("Materials/Stars_PosX.xml"));

        Node* starsPosYNode = skyNode_->CreateChild("StarsPosY");
        StaticModel* starsPosYModel = starsPosYNode->CreateComponent<StaticModel>();
        starsPosYModel->SetModel(cache->GetResource<Model>("Models/Stars_PosY.mdl"));
        starsPosYModel->SetMaterial(cache->GetResource<Material>("Materials/Stars_PosY.xml"));

        Node* starsPosZNode = skyNode_->CreateChild("StarsPosZ");
        StaticModel* starsPosZModel = starsPosZNode->CreateComponent<StaticModel>();
        starsPosZModel->SetModel(cache->GetResource<Model>("Models/Stars_PosZ.mdl"));
        starsPosZModel->SetMaterial(cache->GetResource<Material>("Materials/Stars_PosZ.xml"));

        Node* starsNegXNode = skyNode_->CreateChild("StarsNegX");
        StaticModel* starsNegXModel = starsNegXNode->CreateComponent<StaticModel>();
        starsNegXModel->SetModel(cache->GetResource<Model>("Models/Stars_NegX.mdl"));
        starsNegXModel->SetMaterial(cache->GetResource<Material>("Materials/Stars_NegX.xml"));

        Node* starsNegYNode = skyNode_->CreateChild("StarsNegY");
        StaticModel* starsNegYModel = starsNegYNode->CreateComponent<StaticModel>();
        starsNegYModel->SetModel(cache->GetResource<Model>("Models/Stars_NegY.mdl"));
        starsNegYModel->SetMaterial(cache->GetResource<Material>("Materials/Stars_NegY.xml"));

        Node* starsNegZNode = skyNode_->CreateChild("StarsNegZ");
        StaticModel* starsNegZModel = starsNegZNode->CreateComponent<StaticModel>();
        starsNegZModel->SetModel(cache->GetResource<Model>("Models/Stars_NegZ.mdl"));
        starsNegZModel->SetMaterial(cache->GetResource<Material>("Materials/Stars_NegZ.xml"));

//         Skybox* skyModel = skyNode_->CreateComponent<Skybox>();
//         skyModel->SetModel(cache->GetResource<Model>("Models/Box.mdl"));
//         skyModel->SetMaterial(cache->GetResource<Material>("Materials/SkyBox.xml"));

        // skybox rotation causes glitches on my phone, so another "skybox" will be rotated
//         skyNode2_ = scene_->CreateChild("Sky2");        
//         skyNode2_->SetScale(3);
//         StaticModel* sky2Model = skyNode2_->CreateComponent<StaticModel>();
//         sky2Model->SetModel(cache->GetResource<Model>("Models/Stars.mdl"));
//         sky2Model->SetMaterial(cache->GetResource<Material>("Materials/Stars.xml"));

        paddleNode_ = setupNode("Models/Paddle.mdl", "Materials/Paddle.xml", "Paddle");
        paddleNode_->CreateComponent<Paddle>();
        paddleNode_->SetPosition(Vector3(0, -0.9f, 0));

        ballNode_ = setupNode("Models/Ball.mdl", "Materials/Ball.xml", "Ball", false);
        Ball* ball = ballNode_->CreateComponent<Ball>();
        RigidBody* sphereBody = ballNode_->GetComponent<RigidBody>();
        sphereBody->SetMass(1);
        ballNode_->CreateComponent<CollisionShape>()->SetSphere(ball->GetRadius());
        ballNode_->SetPosition(paddleNode_->GetPosition()
                                + Vector3(0, 0.075f, ball->GetRadius()));

        ballOffsetOriginal_ = ballOffset_ = ballNode_->GetPosition() - paddleNode_->GetPosition();

        // have some depth write (?) artefacts on my phone
        fieldNode_ = setupNode("Models/FieldFloor.mdl", "Materials/FieldFloor.xml", "FieldFloor", false);
        fieldBordersNode_ = setupNode("Models/FieldBorders.mdl", "Materials/FieldBorders.xml", "FieldBorders", false);
        CollisionShape* fbShape1 = fieldBordersNode_->CreateComponent<CollisionShape>();
        fbShape1->SetStaticPlane(Vector3(0, 0, 0), Quaternion(90, 0, 0));
        fbShape1->SetMargin(0.001f);
        CollisionShape* fbShape2 = fieldBordersNode_->CreateComponent<CollisionShape>();
        fbShape2->SetStaticPlane(Vector3(0, 0, ball->GetRadius() * 2 + 0.02f), Quaternion(-90, 0, 0));
        fbShape2->SetMargin(0.001f);
        CollisionShape* fbShape3 = fieldBordersNode_->CreateComponent<CollisionShape>();
        fbShape3->SetStaticPlane(Vector3(0.5f * FIELD_WIDTH, 0, 0), Quaternion(0, 0, 90));
        fbShape3->SetMargin(0.001f);
        CollisionShape* fbShape4 = fieldBordersNode_->CreateComponent<CollisionShape>();
        fbShape4->SetStaticPlane(Vector3(-0.5f * FIELD_WIDTH, 0, 0), Quaternion(0, 0, -90));
        fbShape4->SetMargin(0.001f);
        CollisionShape* fbShape5 = fieldBordersNode_->CreateComponent<CollisionShape>();
        fbShape5->SetStaticPlane(Vector3(0, 0.5f * FIELD_HEIGHT, 0), Quaternion(180, 0, 0));
        fbShape5->SetMargin(0.001f);

        // We need a camera from which the viewport can render.
        cameraNode_ = scene_->CreateChild("Camera");
        cameraNode_->SetDirection(Vector3::FORWARD);
        cameraNode_->SetPosition(Vector3(0, -1.5f, 1.5f));
        cameraNode_->Yaw(180);
        cameraNode_->Pitch(-40.9f);
        Camera* camera = cameraNode_->CreateComponent<Camera>();
        camera->SetNearClip(0.1f);
        camera->SetFarClip(20);

        // Create directional light
        Node* lightNode = skyNode_->CreateChild();
        lightNode->SetDirection(Vector3::FORWARD);
        lightNode->Yaw(180);      // horizontal
        lightNode->Pitch(70);   // vertical
        Light* light = lightNode->CreateComponent<Light>();
        light->SetLightType(LIGHT_DIRECTIONAL);
        light->SetBrightness(1.6);
        light->SetColor(Color(1.0f, 1.0f, 0.5f, 1));
        light->SetCastShadows(true);

        // Setup the viewport.
        Renderer* renderer = GetSubsystem<Renderer>();
        SharedPtr<Viewport> viewport(new Viewport(context_, scene_, cameraNode_->GetComponent<Camera>()));
        renderer->SetViewport(0, viewport);

        // Subscribe to the events to handle.
        SubscribeToEvent(E_KEYDOWN, URHO3D_HANDLER(Arkanoid, HandleKeyDown));
        SubscribeToEvent(E_UPDATE, URHO3D_HANDLER(Arkanoid, HandleUpdate));
        
        prepareLevel();
    }

    /**
    * Good place to get rid of any system resources that requires the
    * engine still initialized. You could do the rest in the destructor,
    * but there's no need, this method will get called when the engine stops,
    * for whatever reason (short of a segfault).
    */
    virtual void Stop()
    {
        clearBonuses();
        clearLevel();
    }

    // Input from keyboard is handled here.
    void HandleKeyDown(StringHash eventType,VariantMap& eventData)
    {
        using namespace KeyDown;
        int key = eventData[P_KEY].GetInt();
        if(key == KEY_ESCAPE)
        {
            engine_->Exit();
        }
    }

    // Non-rendering logic should be handled here.
    // This could be moving objects, checking collisions and reaction, etc.
    void HandleUpdate(StringHash eventType,VariantMap& eventData)
    {
        UI* ui = GetSubsystem<UI>();
        Graphics* graphics = GetSubsystem<Graphics>();
        float scaleX = graphics->GetWidth() / float(BASE_WIDTH);
        float scaleY = graphics->GetHeight() / float(BASE_HEIGHT);
        float sc = Min(scaleX, scaleY);
        ui->SetScale(sc);

        pauseButton_->SetPosition(graphics->GetWidth() / sc - pauseButton_->GetWidth(), 0);
        scoresPanel_->SetPosition((graphics->GetWidth() / sc - scoresPanel_->GetWidth()) / 2, 0);

        float timeStep = eventData[Update::P_TIMESTEP].GetFloat();
// skybox rotation causes rendering artefacts on my phone
//         if (false == paused_)
//         {
//             skyNode_->Rotate(Quaternion(0, 0, 0.25f * timeStep));
//         }
        framecount_ ++;
        time_ += timeStep;

        // setup ball speed
        velocity_ = SPEED_NORMAL;
        Input* input = GetSubsystem<Input>();
        unsigned n = input->GetNumTouches();
        if (n > 0
            && nullptr == ui->GetFocusElement()
            && false == paused_)
        {
            if (0 != ballOffset_.LengthSquared())
            {
                ballNode_->SetPosition(paddleNode_->GetPosition() + ballOffset_);
                if (n > 1
                    && nullptr == input->GetTouch(1)->touchedElement_)
                {
                    velocity_ = SPEED_NORMAL;
                    ballOffset_ = Vector3(0, 0, 0);
                    RigidBody* sphereBody = ballNode_->GetComponent<RigidBody>();
                    sphereBody->ApplyImpulse(Vector3(0, velocity_, 0));
                }
            }
#ifdef _DEBUG
            else
            {
                if (n > 1
                    && nullptr == input->GetTouch(1)->touchedElement_)
                {
                    velocity_ = SPEED_TURBO;
                }
            }
#endif
            // process touches
            TouchState* ts = input->GetTouch(0);
            if (nullptr == ts->touchedElement_)
            {
                IntVector2 touchPos = ts->position_;
                
                Camera* camera = cameraNode_->GetComponent<Camera>();
                Graphics* graphics = GetSubsystem<Graphics>();
                Vector2 paddleScreenPos = camera->WorldToScreenPoint(paddleNode_->GetPosition());
                Ray ray = camera->GetScreenRay(float(touchPos.x_) / graphics->GetWidth(), paddleScreenPos.y_);
                Plane(Vector3(0, 0, 1), Vector3(0, 0, 0));
                float hitDistance = ray.HitDistance(Plane(Vector3(0, 0, 1), Vector3(0, 0, 0)));
                Vector3 hitPoint = ray.origin_ + ray.direction_ * hitDistance;
                Paddle* paddle = paddleNode_->GetComponent<Paddle>();
                paddle->MovePaddle(hitPoint.x_);
            }
        }

        RigidBody* ballBody = ballNode_->GetComponent<RigidBody>();
        if (0 == ballOffset_.LengthSquared())
        {
            Vector3 ballVelocity = ballBody->GetLinearVelocity();
            Vector3 ballVelocity2 = ballVelocity;
            int sign = Sign(ballVelocity.y_);
            ballVelocity2.y_ = Max(Abs(ballVelocity.y_), 0.05f) * (0 == sign ? 1 : sign);
            ballVelocity2.z_ = 0;
//             sphereBody->ApplyImpulse((sphereVelocity2.Normalized() * velocity_ - sphereVelocity));
            ballBody->SetLinearVelocity(ballVelocity2.Normalized() * velocity_);
        }
        else
        {
            ballNode_->SetPosition(paddleNode_->GetPosition() + ballOffset_);
        }
        
        if (ballBody->GetPosition().y_ < -0.5f * FIELD_HEIGHT)
        {
            ballOffset_ = ballOffsetOriginal_;
            ballBody->SetPosition(paddleNode_->GetPosition() + ballOffset_);
            ballBody->SetLinearVelocity(Vector3(0, 0, 0));
            ballBody->SetAngularVelocity(Vector3(0, 0, 0));
            clearActiveBonuses();
            Paddle* paddle = paddleNode_->GetComponent<Paddle>();
            paddle->ResetScale();
        }
        
        Paddle* paddle = paddleNode_->GetComponent<Paddle>();
        scores_ += paddle->GetScores();
        bool roundOver = true;
        for (unsigned i = 0; i < bricks_.Size(); i ++)
        {
            SharedPtr<Node>& brickNode = bricks_[i];
            if (nullptr != brickNode)
            {
                Brick* brick = brickNode->GetComponent<Brick>();
                if (nullptr != brick)
                {
                    if (false != brick->IsCollapsing())
                    {
                        scores_ += brick->GetScores();
                        SharedPtr<Node>& bonusNode = bonuses_[i];
                        if (nullptr != bonusNode)
                        {
                            bonusNode->SetEnabled(true);
                        }
                    }
                    else if (false != brick->IsCollapsed())
                    {
                        brickNode->Remove();
                        brickNode.Reset();
                    }
                    else
                    {
                        roundOver = false;
                    }
                }
            }
        }
        if (false != roundOver)
        {
            ballOffset_ = ballOffsetOriginal_;
            ballBody->SetPosition(paddleNode_->GetPosition() + ballOffset_);
            ballBody->SetLinearVelocity(Vector3(0, 0, 0));
            ballBody->SetAngularVelocity(Vector3(0, 0, 0));
            prepareLevel();
        }

        std::ostringstream scoreStream;
        scoreStream << std::fixed << "Scores: " << scores_;
        std::string scoresStr = scoreStream.str();
        String s(scoresStr.c_str(), scoresStr.size());
        scoresText_->SetText(s);
    }
};

/**
* This macro is expanded to (roughly, depending on OS) this:
*
* > int RunApplication()
* > {
* > Urho3D::SharedPtr<Urho3D::Context> context(new Urho3D::Context());
* > Urho3D::SharedPtr<className> application(new className(context));
* > return application->Run();
* > }
* >
* > int main(int argc, char** argv)
* > {
* > Urho3D::ParseArguments(argc, argv);
* > return function;
* > }
*/
URHO3D_DEFINE_APPLICATION_MAIN(Arkanoid)
