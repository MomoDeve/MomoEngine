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

#include "Json.h"
#include <iomanip>

namespace MxEngine
{
    JsonFile LoadJson(File& file)
    {
        return JsonFile::parse(file.ReadAllText().c_str());
    }

    void SaveJson(File& file, const JsonFile& json)
    {
        file << std::setw(2) << json;
    }
}

using namespace MxEngine;

namespace glm
{
    void to_json(MxEngine::JsonFile& j, const MxEngine::Vector4& v)
    {
        j = JsonFile{ v.x, v.y, v.z, v.w };
    }

    void from_json(const MxEngine::JsonFile& j, MxEngine::Vector4& v)
    {
        auto a = j.get < std::array<float, 4>>();
        v = MakeVector4(a[0], a[1], a[2], a[3]);
    }

    void to_json(JsonFile& j, const Vector3& v)
    {
        j = JsonFile{ v.x, v.y, v.z };
    }

    void from_json(const JsonFile& j, Vector3& v)
    {
        auto a = j.get<std::array<float, 3>>();
        v = MakeVector3(a[0], a[1], a[2]);
    }

    void to_json(JsonFile& j, const Vector2& v)
    {
        j = JsonFile{ v.x, v.y };
    }

    void from_json(const JsonFile& j, Vector2& v)
    {
        auto a = j.get<std::array<float, 2>>();
        v = MakeVector2(a[0], a[1]);
    }

    void to_json(MxEngine::JsonFile& j, const MxEngine::Quaternion& q)
    {
        j = JsonFile{ q.x, q.y, q.z, q.w };
    }

    void from_json(const MxEngine::JsonFile& j, MxEngine::Quaternion& q)
    {
        auto a = j.get<std::array<float, 4>>();
        q = Quaternion{ a[0], a[1], a[2], a[3] };
    }
}

namespace eastl
{
    void to_json(JsonFile& j, const MxString& s)
    {
        j = s.c_str();
    }

    void from_json(const JsonFile& j, MxString& s)
    {
        s = j.get<std::string>().c_str();
    }
}

namespace std::filesystem
{
    void to_json(MxEngine::JsonFile& j, const MxEngine::FilePath& p)
    {
        j = p.string();
    }

    void from_json(const JsonFile& j, MxEngine::FilePath& p)
    {
        p = j.get<std::string>();
    }
}