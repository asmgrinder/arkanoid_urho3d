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

#include "ball.h"

Ball::Ball(Context* context) :
    LogicComponent(context)
{
    scores_ = 0;
    // Only the physics update event is needed: unsubscribe from the rest for optimization
    SetUpdateEventMask(USE_FIXEDUPDATE);
}

void Ball::RegisterObject(Context* context)
{
    context->RegisterFactory<Ball>();
}

void Ball::Start()
{
    // Component has been inserted into its scene node.
    BoundingBox bb = GetComponent<StaticModel>()->GetBoundingBox();
    ballRadius_ = (bb.max_ - bb.min_).Length() * 0.5f / Sqrt(3.0f);
    // get sounds
    ResourceCache* cache = GetSubsystem<ResourceCache>();
    hitSound_ = cache->GetResource<Sound>("Sounds/PlayerFistHit.wav");
    // Component has been inserted into its scene node. Subscribe to events now
    SubscribeToEvent(GetNode(), E_NODECOLLISION, URHO3D_HANDLER(Ball, handleNodeCollision));
}

void Ball::FixedUpdate(float /*timeStep*/)
{
    /// \todo Could cache the components for faster access instead of finding them each frame
    RigidBody* body = GetComponent<RigidBody>();
    Vector3 ballPosition = body->GetPosition();
    ballPosition.z_ = GetRadius();
    body->SetPosition(ballPosition);
}

void Ball::playSound(Sound* sound)
{
    if (nullptr != sound)
    {
        // Create a SoundSource component for playing the sound. The SoundSource component plays
        // non-positional audio, so its 3D position in the scene does not matter. For positional sounds the
        // SoundSource3D component would be used instead
        SoundSource* soundSource = node_->GetScene()->CreateComponent<SoundSource>();
        // Component will automatically remove itself when the sound finished playing
        soundSource->SetAutoRemoveMode(REMOVE_COMPONENT);
        soundSource->Play(sound);
        // In case we also play music, set the sound volume below maximum so that we don't clip the output
        soundSource->SetGain(0.75f);
    }
}

void Ball::handleNodeCollision(StringHash /*eventType*/, VariantMap& eventData)
{
    using namespace NodeCollision;
    
    Node* otherNode = reinterpret_cast<Node*>(eventData[P_OTHERNODE].GetVoidPtr());
    if (String("Brick") == otherNode->GetName()
        || String("Paddle") == otherNode->GetName())
    {
        playSound(hitSound_);
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
