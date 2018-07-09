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
#include <Urho3D/Audio/Audio.h>
#include <Urho3D/Audio/AudioEvents.h>

#include "ball.h"
#include "brick.h"
#include "paddle.h"
#include "bonus.h"

using namespace Urho3D;
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
    SharedPtr<SoundSource> musicSource_;
    Vector<SharedPtr<Node> > bricks_;
    Vector<SharedPtr<Node> > bonuses_;

    Vector3 ballOffsetOriginal_;
    Vector3 ballOffset_;
    float velocity_;
    bool paused_;
    unsigned scores_;
public:
    Arkanoid(Context * context);
    virtual void Setup();
    virtual void Start();
    virtual void Stop();
protected:
    void setupPhysicalProperties(RigidBody* rigidBody);
    Node* setupNode(const String& model, const String& material, const String& nodeName = String::EMPTY, bool setupShape = true);
    void clearLevel();
    void clearActiveBonuses();
    void clearBonuses();
    void prepareLevel();
    void startMusic();
    void handlePause(StringHash eventType, VariantMap& eventData);
    void handleKeyDown(StringHash eventType,VariantMap& eventData);
    void handleUpdate(StringHash eventType,VariantMap& eventData);
};
