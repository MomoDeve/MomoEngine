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

#include "Core/BoundingObjects/AABB.h"

namespace MxEngine
{
    class BoundingSphere
    {
        float radius = 0.0f;
    public:
        Vector3 Center = MakeVector3(0.0f);

        constexpr BoundingSphere() = default;

        constexpr BoundingSphere(const Vector3& center, float r)
            : radius(Max(0.0f, r)), Center(center) { }

        constexpr float GetRedius() const { return radius; }
        constexpr void SetRadius(float r) { this->radius = Max(0.0f, r); }
    };

    // TODO: sphere must be created more precisely, as it is almost always less in size than AABB

    inline constexpr BoundingSphere ToSphere(const AABB& box)
    {
        auto center = box.GetCenter();
        auto length = box.Length();
        float radius = Max(length.x, length.y, length.z) / RootFive<float>(); // * 0.5f;
        return BoundingSphere(center, radius);
    }
}