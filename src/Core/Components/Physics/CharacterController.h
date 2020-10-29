// Copyright(c) 2019 - 2020, #Momo
// All rights reserved.
// 
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met :
// 
// 1. Redistributions of source code must retain the above copyright notice, this
// list of conditions and the following disclaimer.
// 
// 2. Redistributions in binary form must reproduce the above copyright notice,
// this list of conditions and the following disclaimer in the documentation
// and /or other materials provided with the distribution.
// 
// 3. Neither the name of the copyright holder nor the names of its
// contributors may be used to endorse or promote products derived from
// this software without specific prior written permission.
// 
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED.IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include "Utilities/ECS/Component.h"
#include "Utilities/Math/Math.h"

namespace MxEngine
{
    class KeyEvent;

    class CharacterController
    {
        MAKE_COMPONENT(CharacterController);

        bool AreAllComponentsPresent() const;
        const Vector3& GetMotionVector() const;
        
        float jumpPower = 1.0f;
        float jumpSpeed = 0.25f;
        bool isGrounded = false;

    public:
        void OnUpdate(float dt);

        bool IsGrounded() const;
        Vector3 GetCurrentMotion() const;

        void SetJumpPower(float power);
        void SetJumpSpeed(float speed);
        void SetMoveSpeed(float speed);
        void SetRotateSpeed(float speed);
        void SetMass(float mass);

        float GetJumpPower() const;
        float GetJumpSpeed() const;
        float GetMoveSpeed() const;
        float GetRotateSpeed() const;
        float GetMass() const;
    };
}