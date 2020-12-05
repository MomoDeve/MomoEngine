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

#include "Core/MxObject/MxObject.h"
#include "Core/Components/Behaviour.h"
#include "Core/Components/Lighting/DirectionalLight.h"
#include "Core/Components/Lighting/PointLight.h"
#include "Core/Components/Lighting/SpotLight.h"
#include "Core/Serialization/SceneSerializer.h"

namespace MxEngine
{
    void Serialize(JsonFile& json, const LightBase& light)
    {
        json["ambient-intensity"] = light.GetAmbientIntensity();
        json["intensity"] = light.GetIntensity();
        json["color"] = light.GetColor();
    }

    void Serialize(JsonFile& json, const DirectionalLight& light)
    {
        Serialize(json, (LightBase&)light);
        json["direction"] = light.Direction;
        json["projections"] = light.Projections;

        json["follow-viewport"] = light.IsFollowingViewport();
        if (light.IsFollowingViewport())
        {
            json["update-interval"] = light.GetUpdateTimerHandle().GetComponent<Behaviour>()->GetTimeRequest();
        }
    }

    void Serialize(JsonFile& json, const PointLight& light)
    {
        Serialize(json, (LightBase&)light);
        json["radius"] = light.GetRadius();
        json["casts-shadows"] = light.IsCastingShadows();
    }

    void Serialize(JsonFile& json, const SpotLight& light)
    {
        Serialize(json, (LightBase&)light);
        json["outer-angle"] = light.GetOuterAngle();
        json["inner-angle"] = light.GetInnerAngle();
        json["max-distance"] = light.GetMaxDistance();
        json["casts-shadows"] = light.IsCastingShadows();
        json["direction"] = light.Direction;
    }
}