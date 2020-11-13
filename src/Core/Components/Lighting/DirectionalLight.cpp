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

namespace MxEngine
{
    bool DirectionalLight::IsFollowingViewport() const
    {
        return std::launder(reinterpret_cast<const MxObject::Handle*>(&this->timerHandle))->IsValid();
    }

    const MxObject& DirectionalLight::GetUpdateTimerHandle() const
    {
        return **std::launder(reinterpret_cast<const MxObject::Handle*>(&this->timerHandle));
    }

    DirectionalLight::DirectionalLight()
    { 
        for (size_t i = 0; i < DirectionalLight::TextureCount; i++)
        {
            auto depthTextureSize = (int)GlobalConfig::GetDirectionalLightTextureSize();
            auto texture = GraphicFactory::Create<Texture>();
            texture->LoadDepth(depthTextureSize, depthTextureSize);
            texture->SetPath("[[directional light]]");
            this->SetDepthTexture(texture, i);
        }
        // create empty reference to timer
        (void)new(&this->timerHandle) MxObject::Handle();
    }

    DirectionalLight::~DirectionalLight()
    {
        // destroy reference to timer & timer itself
        auto* timer = std::launder(reinterpret_cast<MxObject::Handle*>(&this->timerHandle));
        MxObject::Destroy(*timer);
        timer->~Resource();
    }

    TextureHandle DirectionalLight::GetDepthTexture(size_t index) const
    {
        MX_ASSERT(index < this->textures.size());
        return this->textures[index];
    }

    void DirectionalLight::SetDepthTexture(const TextureHandle& texture, size_t index)
    {
        MX_ASSERT(index < this->textures.size());
        this->textures[index] = texture;
    }

    Matrix4x4 DirectionalLight::GetMatrix(const Vector3& center, size_t index) const
    {
        Vector3 Low  = MakeVector3(-this->Projections[index]);
        Vector3 High = MakeVector3( this->Projections[index]);

        Matrix4x4 OrthoProjection = MakeOrthographicMatrix(Low.x, High.x, Low.y, High.y, Low.z, High.z);
        Matrix4x4 LightView = MakeViewMatrix(
            center + this->Direction,
            center + MakeVector3(0.0f, 0.0f, 0.00001f),
            MakeVector3(0.0f, 1.0f, 0.00001f)
        );
        return OrthoProjection * LightView;
    }

    void DirectionalLight::FollowViewport(float updateInterval)
    {
        // get reference to timer and replace it with new one
        auto& timer = *std::launder(reinterpret_cast<MxObject::Handle*>(&this->timerHandle));
        MxObject::Destroy(timer);

        timer = Timer::CallEachDelta([object = MxObject::GetHandleByComponent(*this)]() mutable
        {
            auto viewport = Rendering::GetViewport();
            if (viewport.IsValid()) 
                object->Transform.SetPosition(MxObject::GetByComponent(*viewport).Transform.GetPosition());
        }, updateInterval);
    }
}