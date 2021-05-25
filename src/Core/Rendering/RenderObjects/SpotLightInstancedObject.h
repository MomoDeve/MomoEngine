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

#include "RenderHelperObject.h"

namespace MxEngine
{
    struct SpotLightBaseData
    {
        Matrix4x4 Transform;
        Vector3 Position;
        float InnerAngle;
        Vector3 Direction;
        float OuterAngle;
        Vector3 Color;
        float AmbientIntensity;

        constexpr static size_t Size = 16 + 3 + 1 + 3 + 1 + 3 + 1;
    };

    class SpotLightInstancedObject : public RenderHelperObject
    {
        VertexBufferHandle instancedVBO;
    public:
        MxVector<SpotLightBaseData> Instances;

        SpotLightInstancedObject() = default;

        SpotLightInstancedObject(size_t vertexOffset, size_t vertexCount, size_t indexOffset, size_t indexCount)
            : RenderHelperObject(vertexOffset, vertexCount, indexOffset, indexCount, Factory<VertexArray>::Create())
        {
            this->instancedVBO = Factory<VertexBuffer>::Create(nullptr, 0, UsageType::STATIC_DRAW);

            std::array vertexLayout = {
                VertexAttribute::Entry<Vector3>(), // position
                VertexAttribute::Entry<Vector2>(), // texture uv
                VertexAttribute::Entry<Vector3>(), // normal
                VertexAttribute::Entry<Vector3>(), // tangent
                VertexAttribute::Entry<Vector3>(), // bitangent
            };
            std::array instanceLayout = {
                VertexAttribute::Entry<Matrix4x4>(), // transform
                VertexAttribute::Entry<Vector4>(),   // position + inner angle
                VertexAttribute::Entry<Vector4>(),   // direction + outer angle
                VertexAttribute::Entry<Vector4>(),   // color + ambient
            };

            VAO->AddVertexLayout(*this->GetVBO(), vertexLayout, VertexAttributeInputRate::PER_VERTEX);
            VAO->AddVertexLayout(*this->instancedVBO, instanceLayout, VertexAttributeInputRate::PER_INSTANCE);
            this->VAO->LinkIndexBuffer(*this->GetIBO());
        }

        void SubmitToVBO() { instancedVBO->BufferDataWithResize((float*)this->Instances.data(), this->Instances.size() * SpotLightBaseData::Size); }
    };
}