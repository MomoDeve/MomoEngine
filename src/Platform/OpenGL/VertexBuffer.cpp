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

#include "VertexBuffer.h"
#include "Platform/OpenGL/GLUtilities.h"
#include "Utilities/Logging/Logger.h"

namespace MxEngine
{		
    void VertexBuffer::FreeVertexBuffer()
    {
		if (this->id != 0)
		{
			GLCALL(glDeleteBuffers(1, &id));
			MXLOG_DEBUG("OpenGL::VertexBuffer", "deleted vertex buffer with id = " + ToMxString(id));
		}
		this->id = 0;
    }

    VertexBuffer::VertexBuffer()
	{
		this->size = 0;
		GLCALL(glGenBuffers(1, &id));
		MXLOG_DEBUG("OpenGL::VertexBuffer", "created vertex buffer with id = " + ToMxString(id));
	}

	VertexBuffer::VertexBuffer(BufferData data, size_t count, UsageType type) 
		: VertexBuffer()
	{
		Load(data, count, type);
	}

	VertexBuffer::~VertexBuffer()
	{
		this->FreeVertexBuffer();
	}

	VertexBuffer::VertexBuffer(VertexBuffer&& vbo) noexcept
	{
		this->id = vbo.id;
		this->size = vbo.size;
		vbo.id = 0;
		vbo.size = 0;
	}

	VertexBuffer& VertexBuffer::operator=(VertexBuffer&& vbo) noexcept
	{
		this->FreeVertexBuffer();

		this->id = vbo.id;
		this->size = vbo.size;
		vbo.id = 0;
		vbo.size = 0;
		return *this;
	}

	void VertexBuffer::Load(BufferData data, size_t count, UsageType usageType)
	{
		this->size = count;
		GLCALL(glBindBuffer(GL_ARRAY_BUFFER, id));
		GLCALL(glBufferData(GL_ARRAY_BUFFER, count * sizeof(float), data, (GLenum)UsageTypeToNative(usageType)));
	}

    void VertexBuffer::BufferSubData(BufferData data, size_t count, size_t offset)
    {
		this->Bind();
		GLCALL(glBufferSubData(GL_ARRAY_BUFFER, sizeof(float) * offset, count * sizeof(float), data));
    }

    void VertexBuffer::BufferDataWithResize(BufferData data, size_t sizeInFloats)
    {
		if (this->GetSize() < sizeInFloats)
			this->Load(data, sizeInFloats, UsageType::DYNAMIC_DRAW);
		else
			this->BufferSubData(data, sizeInFloats);
    }

	void VertexBuffer::GetBufferedData(BufferData data, size_t sizeInFloats, size_t offsetInFloats) const
	{
		this->Bind();
		GLCALL(glGetBufferSubData(GL_ARRAY_BUFFER, offsetInFloats * sizeof(float), sizeInFloats * sizeof(float), (void*)data));
	}

    size_t VertexBuffer::GetSize() const
    {
		return this->size;
    }

	VertexBuffer::BindableId VertexBuffer::GetNativeHandle() const
	{
		return id;
	}

	void VertexBuffer::Bind() const
	{
		GLCALL(glBindBuffer(GL_ARRAY_BUFFER, id));
	}

	void VertexBuffer::Unbind() const
	{
		GLCALL(glBindBuffer(GL_ARRAY_BUFFER, 0));
	}
}