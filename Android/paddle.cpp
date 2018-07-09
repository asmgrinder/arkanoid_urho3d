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

#include <Urho3D/Core/Context.h>
#include <Urho3D/Graphics/StaticModel.h>
#include <Urho3D/IO/MemoryBuffer.h>
#include <Urho3D/Physics/PhysicsEvents.h>
#include <Urho3D/Physics/PhysicsWorld.h>
#include <Urho3D/Physics/RigidBody.h>
#include <Urho3D/Scene/Scene.h>
#include <Urho3D/Scene/SceneEvents.h>

#include "bonus.h"
#include "paddle.h"

Paddle::Paddle(Context* context) :
    LogicComponent(context)
{
    scores_ = 0;
    paddleScale_ = 1;
    // Only the physics update event is needed: unsubscribe from the rest for optimization
    SetUpdateEventMask(USE_UPDATE);
}

void Paddle::RegisterObject(Context* context)
{
    context->RegisterFactory<Paddle>();
}

void Paddle::Start()
{
    ResetScale();
    targetX_ = node_->GetPosition().x_;
    // Component has been inserted into its scene node. Subscribe to events now
    SubscribeToEvent(GetNode(), E_NODECOLLISION, URHO3D_HANDLER(Paddle, handleNodeCollision));
}

void Paddle::Update(float timeStep)
{
    /// \todo Could cache the components for faster access instead of finding them each frame
    RigidBody* body = GetComponent<RigidBody>();
    Vector3 pos = body->GetPosition();
    StaticModel* model = node_->GetComponent<StaticModel>();
    BoundingBox bb = model->GetBoundingBox();
    float paddleWidth = bb.max_.x_ * node_->GetScale().x_;
    targetX_ = Clamp(targetX_, -0.5f * FIELD_WIDTH + paddleWidth, 0.5f * FIELD_WIDTH - paddleWidth);
    float delta = timeStep * PADDLE_SPEED;
    float diff = targetX_ - pos.x_;
    if (delta > Abs(diff))
    {
        delta = Abs(diff);
    }
    pos.x_ += delta * Sign(diff);
    node_->SetPosition(pos);

    float scale = node_->GetScale().x_;
    float diffScale = getTargetScale() - scale;
    float deltaScale = timeStep * PADDLE_SCALE_SPEED;
    if (deltaScale > Abs(diffScale))
    {
        deltaScale = Abs(diffScale);
    }
    scale += deltaScale * Sign(diffScale);
    node_->SetScale(Vector3(scale, 1, 1));
}

void Paddle::ResetScale()
{
    paddleScale_ = 1;
    node_->SetScale(Vector3(getTargetScale(), 1, 1));
}

void Paddle::MovePaddle(float targetX)
{
    targetX_ = targetX;
}

int Paddle::GetScores()
{
    int result = scores_;
    scores_ = 0;
    return result;
}

void Paddle::handleNodeCollision(StringHash eventType, VariantMap& eventData)
{
    using namespace NodeCollision;

    Node* otherNode = reinterpret_cast<Node*>(eventData[P_OTHERNODE].GetVoidPtr());;
    String otherName = otherNode->GetName();
    if (String("Bonus") == otherName)
    {
        Bonus* bonus = otherNode->GetComponent<Bonus>();
        unsigned bonusType = bonus->GetBonusType();
        otherNode->SetEnabled(false);
        switch (bonusType)
        {
            case BONUS_SHRINKPADDLE:
                if (paddleScale_ > 0)
                {
                    paddleScale_ --;
                }
                break;
            case BONUS_EXTENDPADDLE:
                if (paddleScale_ < 4)
                {
                    paddleScale_ ++;
                }
                break;
            case BONUS_100:
                scores_ += 100;
                break;
            case BONUS_200:
                scores_ += 200;
                break;
            case BONUS_500:
                scores_ += 500;
                break;
            case BONUS_1000:
                scores_ += 1000;
                break;
            case BONUS_2000:
                scores_ += 2000;
                break;
            case BONUS_5000:
                scores_ += 5000;
                break;
            case BONUS_10000:
                scores_ += 10000;
                break;
        }
    }

    MemoryBuffer contacts(eventData[P_CONTACTS].GetBuffer());

    while (!contacts.IsEof())
    {
        /*Vector3 contactPosition = */contacts.ReadVector3();
        /*Vector3 contactNormal = */contacts.ReadVector3();
        /*float contactDistance = */contacts.ReadFloat();
        /*float contactImpulse = */contacts.ReadFloat();

    }
}
