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

using namespace Urho3D;

enum { BONUS_NONE, BONUS_SHRINKPADDLE, BONUS_EXTENDPADDLE,
        BONUS_100, BONUS_200, BONUS_500,
        BONUS_1000, BONUS_2000, BONUS_5000, BONUS_10000, BONUS_COUNT };

const float BONUS_SPEED = 0.2f;
const float FIELD_WIDTH = 2;
const float FIELD_HEIGHT = 2;

class Bonus : public LogicComponent
{
    URHO3D_OBJECT(Bonus, LogicComponent);
public:
    Bonus(Context* context);
    /// Register object factory and attributes.
    static void RegisterObject(Context* context);
    /// Handle startup. Called by LogicComponent base class.
    virtual void Start();
    /// Handle physics world update. Called by LogicComponent base class.
    virtual void FixedUpdate(float timeStep);
    virtual void SetBonusType(unsigned bonusType) { bonusType_ = bonusType; }
    virtual unsigned GetBonusType() { return bonusType_; }
private:
    /// Handle physics collision event.
    void handleNodeCollision(StringHash eventType, VariantMap& eventData);

    unsigned bonusType_;
    float bonusSpeed_;
};
