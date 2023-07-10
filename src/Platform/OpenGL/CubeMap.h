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

#include "Utilities/STL/MxString.h"
#include "Utilities/Image/Image.h"

namespace MxEngine
{
    class CubeMap
    {
    public:
        using CubeMapBindId = int;

    private:
        using BindableId = unsigned int;
        MxString filepath;
        size_t width = 0, height = 0, channels = 0;
        BindableId id = 0;
        mutable CubeMapBindId activeId = 0;
        void FreeCubeMap();
    public:
        CubeMap();
         
        CubeMap(const CubeMap&) = delete;
        CubeMap& operator=(const CubeMap&) = delete;
        CubeMap(CubeMap&&) noexcept;
        CubeMap& operator=(CubeMap&&) noexcept;
        ~CubeMap();

        template<typename FilePath>
        CubeMap(const FilePath& filepath);

        void Bind() const;
        void Unbind() const;
        void Bind(CubeMapBindId id) const;
        CubeMapBindId GetBoundId() const;
        BindableId GetNativeHandle() const;

        void Load(const MxString& filepath);
        template<typename FilePath> void Load(const FilePath& filepath);
        template<typename FilePath> void Load(
            const FilePath& right,  const FilePath& left,  const FilePath& top,
            const FilePath& bottom, const FilePath& front, const FilePath& back
        );

        void Load(const std::array<Image, 6>& images);
        void Load(const std::array<uint8_t*, 6>& RawDataRGB, size_t width, size_t height);
        void LoadDepth(int width, int height);
        size_t GetWidth() const;
        size_t GetHeight() const;
        size_t GetChannelCount() const;
        void SetMaxLOD(size_t lod);
        void SetMinLOD(size_t lod);
        void GenerateMipmaps();

        const MxString& GetFilePath() const;
        void SetInternalEngineTag(const MxString& tag);
        bool IsInternalEngineResource() const;
    };
}