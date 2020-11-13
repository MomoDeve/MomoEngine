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

#include "Utilities/Json/Json.h"

namespace MxEngine
{
    class MxObject;
    class TransformComponent;
    class Behaviour;
    class DirectionalLight;
    class PointLight;
    class SpotLight;
    class InstanceFactory;
    class Instance;
    class CameraController;
    class CameraSSR;
    class CameraEffects;
    class CameraToneMapping;
    class VRCameraController;
    class InputController;

    void Serialize(JsonFile& json, MxObject&);
    void Serialize(JsonFile& json, TransformComponent&);
    void Serialize(JsonFile& json, Behaviour&);
    void Serialize(JsonFile& json, DirectionalLight&);
    void Serialize(JsonFile& json, PointLight&);
    void Serialize(JsonFile& json, SpotLight&);
    void Serialize(JsonFile& json, InstanceFactory&);
    void Serialize(JsonFile& json, Instance&);
    void Serialize(JsonFile& json, CameraController&);
    void Serialize(JsonFile& json, CameraSSR&);
    void Serialize(JsonFile& json, CameraToneMapping&);
    void Serialize(JsonFile& json, CameraEffects&);
    void Serialize(JsonFile& json, VRCameraController&);
    void Serialize(JsonFile& json, InputController&);

    class SceneSerializer
    {
        static void SerializeGlobals(JsonFile& json);
        static void SerializeObjects(JsonFile& json);
    public:
        static JsonFile Serialize();
    };
}