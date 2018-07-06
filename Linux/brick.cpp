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
#include <Urho3D/Graphics/AnimationController.h>
#include <Urho3D/IO/MemoryBuffer.h>
#include <Urho3D/Physics/PhysicsEvents.h>
#include <Urho3D/Physics/PhysicsWorld.h>
#include <Urho3D/Physics/RigidBody.h>
#include <Urho3D/Scene/Scene.h>
#include <Urho3D/Scene/SceneEvents.h>

#include "brick.h"

Brick::Brick(Context* context) :
    LogicComponent(context)
{
    scores_ = BRICK_SCORES;
    isCollapsing_ = false;
    isCollapsed_ = false;
//     bonusType_ = BONUS_NONE;
    shrinkTime_ = 0;
    // Only the physics update event is needed: unsubscribe from the rest for optimization
    SetUpdateEventMask(USE_UPDATE);
}

void Brick::RegisterObject(Context* context)
{
    context->RegisterFactory<Brick>();
}

void Brick::Start()
{
    // Component has been inserted into its scene node. Subscribe to events now
    SubscribeToEvent(GetNode(), E_NODECOLLISION, URHO3D_HANDLER(Brick, handleNodeCollision));
}

void Brick::Update(float timeStep)
{
    if (false == isCollapsed_
        && 0 < shrinkTime_)
    {
        shrinkTime_ -= Min(timeStep, shrinkTime_);
        node_->SetScale(shrinkTime_ / SHRINK_TIME);
        isCollapsed_ = (Abs(shrinkTime_) < 1e-6f);
    }
}

int Brick::GetScores()
{
    if (false != isCollapsed_
        || 0 != shrinkTime_)
    {
        float retVal = scores_;
        scores_ = 0;
        return retVal;
    }
    return 0;
}

bool Brick::IsCollapsing()
{
    bool result = isCollapsing_;
    isCollapsing_ = false;
    return result;
}

void Brick::handleNodeCollision(StringHash eventType, VariantMap& eventData)
{
    using namespace NodeCollision;

    Node* otherNode = reinterpret_cast<Node*>(eventData[P_OTHERNODE].GetVoidPtr());;
    if (String("Ball") == otherNode->GetName())
    {
        if (0 == shrinkTime_)
        {
            isCollapsing_ = true;
            shrinkTime_ = SHRINK_TIME;
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
