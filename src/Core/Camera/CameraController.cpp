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

#include "CameraController.h"

namespace MxEngine
{
	bool CameraController::HasCamera() const
	{
		return this->camera != nullptr;
	}

	void CameraController::SetCamera(UniqueRef<ICamera> camera)
	{
		this->camera = std::move(camera);
	}

	ICamera& CameraController::GetCamera()
	{
		MX_ASSERT(this->camera != nullptr);
		return *this->camera;
	}

	const ICamera& CameraController::GetCamera() const
	{
		MX_ASSERT(this->camera != nullptr);
		return *this->camera;
	}

	const Matrix4x4& CameraController::GetMatrix() const
	{
		static Matrix4x4 defaultMatrix(0.0f);
		if (!this->HasCamera()) return defaultMatrix;

		if (updateCamera)
		{
			auto view = MakeViewMatrix(position, position + direction, up);
			this->camera->SetViewMatrix(view);
			this->updateCamera = false;
		}
		return this->camera->GetMatrix();
	}

	const Vector3& CameraController::GetPosition() const
	{
		return this->position;
	}

	const Vector3& CameraController::GetDirection() const
	{
		return this->direction;
	}

	const Vector3& CameraController::GetUp() const
	{
		return this->up;
	}

	float CameraController::GetMoveSpeed() const
	{
		return this->moveSpeed;
	}

	float CameraController::GetRotateSpeed() const
	{
		return this->rotateSpeed;
	}

	void CameraController::SetPosition(const Vector3& position)
	{
		this->position = position;
		this->updateCamera = true;
	}

	void CameraController::SetDirection(const Vector3& direction)
	{
		this->direction = direction;
		this->updateCamera = true;
	}

	void CameraController::SetUp(const Vector3& up)
	{
		this->up = up;
		this->updateCamera = true;
	}

	void CameraController::SetMoveSpeed(float speed)
	{
		this->moveSpeed = speed;
	}

	void CameraController::SetRotateSpeed(float speed)
	{
		this->rotateSpeed = speed;
	}

	void CameraController::SetZoom(float zoom)
	{
		this->camera->SetZoom(zoom);
	}

	float CameraController::GetHorizontalAngle() const
	{
		return this->horizontalAngle;
	}

	float CameraController::GetVerticalAngle() const
	{
		return this->verticalAngle;
	}

	float CameraController::GetZoom() const
	{
		return this->camera->GetZoom();
	}

	CameraController& CameraController::Translate(float x, float y, float z)
	{
		return Translate({ x, y, z });
	}

	CameraController& CameraController::Translate(const Vector3& vec)
	{
		this->position += this->moveSpeed * vec;
		this->updateCamera = true;
		return *this;
	}

	CameraController& CameraController::TranslateX(float x)
	{
		this->position.x += this->moveSpeed * x;
		this->updateCamera = true;
		return *this;
	}

	CameraController& CameraController::TranslateY(float y)
	{
		this->position.y += this->moveSpeed * y;
		this->updateCamera = true;
		return *this;
	}

	CameraController& CameraController::TranslateZ(float z)
	{
		this->position.z += this->moveSpeed * z;
		this->updateCamera = true;
		return *this;
	}

	CameraController& CameraController::Rotate(float horizontal, float vertical)
	{
		this->horizontalAngle += this->rotateSpeed * horizontal;
		this->verticalAngle += this->rotateSpeed * vertical;

		this->verticalAngle = Clamp(this->verticalAngle, 
			-HalfPi<float>() + 0.001f, HalfPi<float>() - 0.001f);
		while (this->horizontalAngle >= TwoPi<float>())
			this->horizontalAngle -= TwoPi<float>();
		while (this->horizontalAngle < 0)
			this->horizontalAngle += TwoPi<float>();

		this->SetDirection({
			std::cos(verticalAngle) * std::sin(horizontalAngle),
			std::sin(verticalAngle),
			std::cos(verticalAngle) * std::cos(horizontalAngle)
			});

		this->forward = MakeVector3(
			sin(horizontalAngle),
			0.0f,
			cos(horizontalAngle)
		);

		this->right = MakeVector3(
			sin(horizontalAngle - HalfPi<float>()),
			0.0f,
			cos(horizontalAngle - HalfPi<float>())
		);
		this->updateCamera = true;
		return *this;
	}

	void CameraController::SetForwardVector(const Vector3& forward)
	{
		this->forward = forward;
	}

	void CameraController::SetUpVector(const Vector3& up)
	{
		this->up = up;
	}

	void CameraController::SetRightVector(const Vector3& right)
	{
		this->right = right;
	}

	const Vector3& CameraController::GetForwardVector() const
	{
		return this->forward;
	}

	const Vector3& CameraController::GetUpVector() const
	{
		return this->up;
	}

	const Vector3& CameraController::GetRightVector() const
	{
		return this->right;
	}

	CameraController& CameraController::TranslateForward(float dist)
	{
		return Translate(this->forward * dist);
	}

	CameraController& CameraController::TranslateRight(float dist)
	{
		return Translate(this->right * dist);
	}

	CameraController& CameraController::TranslateUp(float dist)
	{
		return Translate(this->up * dist);
	}
}