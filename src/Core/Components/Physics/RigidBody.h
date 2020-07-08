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

#pragma once

#include "Platform/PhysicsAPI.h"
#include "Utilities/ECS/Component.h"

namespace MxEngine
{
    enum class AnisotropicFriction
    {
        DISABLED = 0,
        ENABLED = 1,
        ROLLING_FRICTION = 2
    };

    class RigidBody
    {
        MAKE_COMPONENT(RigidBody);

        NativeRigidBodyHandle rigidBody;
    public:
        MXENGINE_MAKE_MOVEONLY(RigidBody);

        NativeRigidBodyHandle GetNativeHandle() const;

        void Init();
        void OnUpdate(float dt);
        void UpdateCollider();
        ~RigidBody();

        void MakeKinematic();
        bool IsKinematic();
        float GetMass() const;
        void SetMass(float mass);
        float GetFriction() const;
        void SetFriction(float value);
        float GetSpinningFriction() const;
        void SetSpinningFriction(float value);
        float GetRollingFriction() const;
        void SetRollingFriction(float value);
        void SetLinearVelocity(const Vector3& velocity);
        float GetBounceFactor() const;
        void SetBounceFactor(float value);
        Vector3 GetGravity() const;
        void SetGravity(const Vector3& gravity);
        Vector3 GetLinearVelocity() const;
        void SetAngularVelocity(const Vector3& velocity);
        Vector3 GetAngularVelocity() const;
        float GetInverseMass() const;
        Vector3 GetMassFactor() const;
        void SetMassFactor(const Vector3& factor);
        Vector3 GetAngularForceFactor() const;
        void SetAngularForceFactor(const Vector3& factor);
        Vector3 GetLinearForceFactor() const;
        void SetLinearForceFactor(const Vector3& factor);
        void SetAngularAirResistance(float value);
        float GetAngularAirResistance() const;
        void SetLinearAirResistance(float value);
        float GetLinearAirResistance() const;
        Vector3 GetTotalForce() const;
        Vector3 GetTotalTorque() const;
        Vector3 GetInertia() const;
        Vector3 GetVelocityInPoint(const Vector3& relativePosition);
        Vector3 GetPushVelocityInPoint(const Vector3& relativePosition);
        void ApplyCentralImpulse(const Vector3& impulse);
        void ApplyCentralPushImpulse(const Vector3& impulse);
        void ApplyForce(const Vector3& force, const Vector3& relativePosition);
        void ApplyImpulse(const Vector3& impulse, const Vector3& relativePosition);
        void ApplyPushImpulse(const Vector3& impulse, const Vector3& relativePosition);
        void ApplyTorque(const Vector3& force);
        void ApplyTorqueImpulse(const Vector3& impulse);
        void ApplyTorqueTurnImpulse(const Vector3& impulse);
        void ApplyCentralForce(const Vector3& force);
        void ApplyCenteralForce(const Vector3& force);
        void SetPushVelocity(const Vector3& velocity);
        Vector3 GetPushVelocity() const;
        void SetTurnVelocity(const Vector3& velocity);
        Vector3 GetTurnVelocity() const;
        void SetAnisotropicFriction(const Vector3& friction, AnisotropicFriction mode = AnisotropicFriction::ENABLED);
        bool HasAnisotropicFriction() const;
        Vector3 GetAnisotropicFriction() const;
    };
}