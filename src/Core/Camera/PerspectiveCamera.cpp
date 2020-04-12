// Copyright(c) 2019 - 2020, #Momo
// All rights reserved.
// 
// Redistributionand use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met :
// 
// 1. Redistributions of source code must retain the above copyright notice, this
// list of conditionsand the following disclaimer.
// 
// 2. Redistributions in binary form must reproduce the above copyright notice,
// this list of conditionsand the following disclaimer in the documentation
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

#include "PerspectiveCamera.h"

namespace MxEngine
{
	const Matrix4x4& PerspectiveCamera::GetMatrix() const
	{
		if (this->updateMatrix)
		{
			if (this->updateProjection)
			{
				const float fov = Clamp(zoom * FOV, Radians(10.0f), Radians(150.0f));
				this->projection = MakeReversedPerspectiveMatrix(fov, aspectRatio, zNear, zFar);
				this->updateProjection = false;
			}
			this->matrix = this->projection * this->view;
			this->updateMatrix = false;
		}
		return this->matrix;
	}

    const Matrix4x4& PerspectiveCamera::GetViewMatrix() const
    {
		auto& _ = this->GetMatrix();
		return this->view;
    }

	const Matrix4x4& PerspectiveCamera::GetProjectionMatrix() const
	{
		auto& _ = this->GetMatrix();
		return this->projection;
	}

	void PerspectiveCamera::SetViewMatrix(const Matrix4x4& view)
	{
		this->view = view;
		this->updateMatrix = true;
	}

	void PerspectiveCamera::SetFOV(float angle)
	{
		updateMatrix = true;
		updateProjection = true;
		this->FOV = Radians(angle);
	}

	float PerspectiveCamera::GetFOV() const
	{
		return Degrees(this->FOV);
	}

	void PerspectiveCamera::SetAspectRatio(float w, float h)
	{
		updateMatrix = true;
		updateProjection = true;
		this->aspectRatio = w / h;
	}

	float PerspectiveCamera::GetAspectRatio() const
	{
		return this->aspectRatio;
	}

	float PerspectiveCamera::GetZFar() const
	{
		return this->zFar;
	}

	void PerspectiveCamera::SetZFar(float zFar)
	{
		updateMatrix = true;
		updateProjection = true;
		this->zFar = zFar;
	}
	
	float PerspectiveCamera::GetZNear() const
	{
		return this->zNear;
	}

	void PerspectiveCamera::SetZNear(float zNear)
	{
		updateMatrix = true;
		updateProjection = true;
		this->zNear = zNear;
	}

	float PerspectiveCamera::GetZoom() const
	{
		return this->zoom;
	}

	void PerspectiveCamera::SetZoom(float zoom)
	{
		this->zoom = zoom;
		this->updateProjection = true;
		this->updateMatrix = true;
	}
}