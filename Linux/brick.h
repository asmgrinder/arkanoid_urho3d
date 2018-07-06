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

#pragma once

#include <Urho3D/Input/Controls.h>
#include <Urho3D/Scene/LogicComponent.h>

#include "bonus.h"

const int BRICK_SCORES = 10;
const float SHRINK_TIME = 0.5f;

using namespace Urho3D;

class Brick : public LogicComponent
{
    URHO3D_OBJECT(Brick, LogicComponent);
public:
    Brick(Context* context);
    /// Register object factory and attributes.
    static void RegisterObject(Context* context);
    /// Handle startup. Called by LogicComponent base class.
    virtual void Start();
    /// Handle physics world update. Called by LogicComponent base class.
    virtual void Update(float timeStep);
    virtual bool IsCollapsed() { return isCollapsed_; }
    virtual bool IsCollapsing();
    virtual int GetScores();

private:
    /// Handle physics collision event.
    void handleNodeCollision(StringHash eventType, VariantMap& eventData);
    
    int scores_;
    bool isCollapsing_, isCollapsed_;
    float shrinkTime_;
};
