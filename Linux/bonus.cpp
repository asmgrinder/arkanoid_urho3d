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

#include "bonus.h"

Bonus::Bonus(Context* context) :
    LogicComponent(context)
{
    bonusType_ = BONUS_NONE;
    bonusSpeed_ = 0;
    // Only the physics update event is needed: unsubscribe from the rest for optimization
    SetUpdateEventMask(USE_FIXEDUPDATE);
}

void Bonus::RegisterObject(Context* context)
{
    context->RegisterFactory<Bonus>();

//     // These macros register the class attributes to the Context for automatic load / save handling.
//     // We specify the Default attribute mode which means it will be used both for saving into file, and network replication
//     URHO3D_ATTRIBUTE("Controls Yaw", float, controls_.yaw_, 0.0f, AM_DEFAULT);
//     URHO3D_ATTRIBUTE("Controls Pitch", float, controls_.pitch_, 0.0f, AM_DEFAULT);
}

void Bonus::Start()
{
    // Component has been inserted into its scene node. Subscribe to events now
    SubscribeToEvent(GetNode(), E_NODECOLLISION, URHO3D_HANDLER(Bonus, handleNodeCollision));
}

void Bonus::FixedUpdate(float /*timeStep*/)
{
    RigidBody* body = GetComponent<RigidBody>();
    body->SetLinearVelocity(Vector3(0, -bonusSpeed_, 0));
    Vector3 bonusPosition = body->GetPosition();
    if (bonusPosition.y_ < -FIELD_HEIGHT * 0.75f)
    {
        node_->SetEnabled(false);
    }
//     body->GetPosition();
    bonusSpeed_ = BONUS_SPEED;
}

void Bonus::handleNodeCollision(StringHash /*eventType*/, VariantMap& eventData)
{
    using namespace NodeCollision;
    Node* otherNode = reinterpret_cast<Node*>(eventData[P_OTHERNODE].GetVoidPtr());
    if (/*nullptr != otherNode
        && nullptr != node_
        && false != node_->IsEnabled()
        && */String("Bonus") == otherNode->GetName()
        && node_->GetPosition().y_ > otherNode->GetPosition().y_)
    {
        bonusSpeed_ = 0.5f * BONUS_SPEED;
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
