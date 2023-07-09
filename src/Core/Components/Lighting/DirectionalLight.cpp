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

#include "DirectionalLight.h"
#include "Core/Application/Rendering.h"
#include "Core/Config/GlobalConfig.h"
#include "Core/Application/Timer.h"
#include "Core/Components/Camera/CameraController.h"
#include "Core/Runtime/Reflection.h"

namespace MxEngine
{
    DirectionalLight::DirectionalLight()
    { 
        auto depthTextureSize = (int)GlobalConfig::GetDirectionalLightTextureSize();
        this->DepthMap = Factory<Texture>::Create();
        this->DepthMap->LoadDepth(DirectionalLight::TextureCount * depthTextureSize, depthTextureSize);
        this->DepthMap->SetInternalEngineTag(MXENGINE_MAKE_INTERNAL_TAG("directional light"));
    }

    Matrix4x4 DirectionalLight::GetMatrix(size_t index) const
    {
        MX_ASSERT(index < DirectionalLight::TextureCount);

        auto& self = MxObject::GetByComponent(*this);
        float distance = 0.0f;
        for (size_t i = 0; i < index; i++)
        {
            distance += this->Projections[i];
        }
        distance += this->Projections[index] * 0.5;
        auto center = self.LocalTransform.GetPosition() + distance * this->CascadeDirection;

        constexpr auto floor = [](const Vector3 & v) -> Vector3
        {
            return { std::floor(v.x), std::floor(v.y), std::floor(v.z) };
        };

        Matrix4x4 LightView = MakeViewMatrix(
            Normalize(this->Direction),
            MakeVector3(0.0f, 0.0f, 0.0f),
            MakeVector3(0.001f, 1.0f, 0.001f)
        );
        center = (Matrix3x3)LightView * center;

        auto lowPlane = MakeVector3(-this->Projections[index]) + center;
        auto highPlane = MakeVector3( this->Projections[index]) + center;

        auto shadowMapSize = float(this->DepthMap->GetHeight() + 1);
        auto worldUnitsPerText = (highPlane - lowPlane) / shadowMapSize;
        lowPlane = floor(lowPlane / worldUnitsPerText) * worldUnitsPerText;
        highPlane = floor(highPlane / worldUnitsPerText) * worldUnitsPerText;
        center = (highPlane + lowPlane) * -0.5f;

        Matrix4x4 OrthoProjection = MakeOrthographicMatrix(lowPlane.x, highPlane.x, lowPlane.y, highPlane.y, lowPlane.z, highPlane.z);
        return OrthoProjection * LightView;
    }

    void DirectionalLight::OnUpdate(float dt)
    {
        if (this->IsFollowingViewport)
        {
            auto viewport = Rendering::GetViewport();
            if (viewport.IsValid())
            {
                auto& object = MxObject::GetByComponent(*this);
                object.LocalTransform.SetPosition(MxObject::GetByComponent(*viewport).LocalTransform.GetPosition());
                auto direction = viewport->GetDirection();
                this->CascadeDirection = Normalize(Vector3(direction.x, 0.0f, direction.z));
            }
        }
    }

    MXENGINE_REFLECT_TYPE
    {
        rttr::registration::class_<DirectionalLight>("DirectionalLight")
            (
                rttr::metadata(MetaInfo::FLAGS, MetaInfo::CLONE_COPY | MetaInfo::CLONE_INSTANCE)
            )
            .constructor<>()
            .property("color", &DirectionalLight::GetColor, &DirectionalLight::SetColor)
            (
                rttr::metadata(MetaInfo::FLAGS, MetaInfo::SERIALIZABLE | MetaInfo::EDITABLE),
                rttr::metadata(EditorInfo::INTERPRET_AS, InterpretAsInfo::COLOR)
            )
            .property("intensity", &DirectionalLight::GetIntensity, &DirectionalLight::SetIntensity)
            (
                rttr::metadata(MetaInfo::FLAGS, MetaInfo::SERIALIZABLE | MetaInfo::EDITABLE),
                rttr::metadata(EditorInfo::EDIT_PRECISION, 0.1f),
                rttr::metadata(EditorInfo::EDIT_RANGE, Range { 0.0f, 10000000.0f })
            )
            .property("ambient intensity", &DirectionalLight::GetAmbientIntensity, &DirectionalLight::SetAmbientIntensity)
            (
                rttr::metadata(MetaInfo::FLAGS, MetaInfo::SERIALIZABLE | MetaInfo::EDITABLE),
                rttr::metadata(EditorInfo::EDIT_PRECISION, 0.01f),
                rttr::metadata(EditorInfo::EDIT_RANGE, Range { 0.0f, 1.0f })
            )
            .property("is following viewport", &DirectionalLight::IsFollowingViewport)
            (
                rttr::metadata(MetaInfo::FLAGS, MetaInfo::SERIALIZABLE | MetaInfo::EDITABLE)
            )
            .property("direction", &DirectionalLight::Direction)
            (
                rttr::metadata(MetaInfo::FLAGS, MetaInfo::SERIALIZABLE | MetaInfo::EDITABLE),
                rttr::metadata(EditorInfo::EDIT_PRECISION, 0.01f)
            )
            .property_readonly("depth map", &DirectionalLight::DepthMap)
            (
                rttr::metadata(MetaInfo::FLAGS, MetaInfo::EDITABLE)
            )
            .property("projections", &DirectionalLight::Projections)
            (
                rttr::metadata(MetaInfo::FLAGS, MetaInfo::SERIALIZABLE | MetaInfo::EDITABLE)
            )
            .property("cascade direction", &DirectionalLight::CascadeDirection)
            (
                rttr::metadata(MetaInfo::FLAGS, MetaInfo::SERIALIZABLE | MetaInfo::EDITABLE),
                rttr::metadata(MetaInfo::CONDITION, +([](const rttr::instance& v) { return !v.try_convert<DirectionalLight>()->IsFollowingViewport; }))
            );
    }
}