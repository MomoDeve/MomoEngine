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

#include "Utilities/Math/Math.h"
#include "Core/Macro/Macro.h"
#include "Utilities/STL/MxString.h"
#include "Utilities/STL/MxVector.h"
#include "Utilities/STL/MxHashMap.h"

namespace MxEngine
{
	class Shader
	{
		#if defined(MXENGINE_DEBUG)
		MxString vertexShaderPath;
		MxString geometryShaderPath;
		MxString fragmentShaderPath;
		MxVector<MxString> includedFilePaths;
		#endif
		using UniformType = int;
		using UniformCache = MxHashMap<MxString, UniformType>;
		using ShaderId = unsigned int;
		using BindableId = unsigned int;

		BindableId id = 0;
		mutable UniformCache uniformCache;

		template<typename FilePath>
		ShaderId CompileShader(unsigned int type, const MxString& source, const FilePath& name);

		BindableId CreateProgram(ShaderId vertexShader, ShaderId fragmentShader) const;
		BindableId CreateProgram(ShaderId vertexShader, ShaderId geometryShader, ShaderId fragmentShader) const;
		UniformType GetUniformLocation(const MxString& uniformName) const;
		void FreeShader();
	public:
		static MxString GetShaderVersionString();

		Shader();

		template<typename FilePath>
		Shader(const FilePath& vertexShaderPath, const FilePath& fragmentShaderPath);
		template<typename FilePath>
		Shader(const FilePath& vertexShaderPath, const FilePath& geometryShaderPath, const FilePath& fragmentShaderPath);

		Shader(const Shader&) = delete;
		Shader(Shader&& shader) noexcept;
		Shader& operator=(const Shader&) = delete;
		Shader& operator=(Shader&& shader) noexcept;
		~Shader();

		void Bind() const;
		void Unbind() const;
		void InvalidateUniformCache();
		BindableId GetNativeHandle() const;

		template<typename FilePath>
		void Load(const FilePath& vertex, const FilePath& fragment);
		template<typename FilePath>
		void Load(const FilePath& vertex, const FilePath& geometry, const FilePath& fragment);

		void IgnoreNonExistingUniform(const MxString& name) const;
		void IgnoreNonExistingUniform(const char* name) const;
		void LoadFromString(const MxString& vertex, const MxString& fragment);
		void LoadFromString(const MxString& vertex, const MxString& geometry, const MxString& fragment);
		void SetUniformFloat(const MxString& name, float f) const;
		void SetUniformVec2(const MxString& name, const Vector2& vec) const;
		void SetUniformVec3(const MxString& name, const Vector3& vec) const;
		void SetUniformVec4(const MxString& name, const Vector4& vec) const;
		void SetUniformMat4(const MxString& name, const Matrix4x4& matrix) const;
		void SetUniformMat3(const MxString& name, const Matrix3x3& matrix) const;
		void SetUniformInt(const MxString& name, int i) const;
		void SetUniformBool(const MxString& name, bool b) const;

		const MxString& GetVertexShaderDebugFilePath() const;
		const MxString& GetGeometryShaderDebugFilePath() const;
		const MxString& GetFragmentShaderDebugFilePath() const;
		const MxVector<MxString>& GetIncludedFilePaths() const;
	};
}
