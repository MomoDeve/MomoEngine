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

#include "Core/Components/Camera/CameraController.h"
#include "Core/Components/Camera/InputControl.h"
#include "Core/Components/Camera/VRCameraController.h"
#include "Core/Components/Camera/FrustrumCamera.h"
#include "Core/Components/Camera/OrthographicCamera.h"
#include "Core/Components/Camera/PerspectiveCamera.h"

#include "Utilities/ImGui/ImGuiUtils.h"

namespace MxEngine::GUI
{
	#define REMOVE_COMPONENT_BUTTON(comp) \
	if(ImGui::Button("remove component")) {\
		MxObject::GetByComponent(comp).RemoveComponent<std::remove_reference_t<decltype(comp)>>(); return; }

	void CameraControllerEditor(CameraController& cameraController)
	{
		TREE_NODE_PUSH("CameraController");
		REMOVE_COMPONENT_BUTTON(cameraController);

		bool perspective = cameraController.GetCameraType() == CameraType::PERSPECTIVE;
		bool orthographic = cameraController.GetCameraType() == CameraType::ORTHOGRAPHIC;
		bool frustrum = cameraController.GetCameraType() == CameraType::FRUSTRUM;

		if (ImGui::BeginCombo("camera type", perspective ? "perspective" : (orthographic ? "orthographic" : "frustrum")))
		{
			if (ImGui::Selectable("perspective", &perspective))
			{
				cameraController.SetCameraType(CameraType::PERSPECTIVE);
				ImGui::SetItemDefaultFocus();
			}
			if (ImGui::Selectable("orthographic", &orthographic))
			{
				cameraController.SetCameraType(CameraType::ORTHOGRAPHIC);
				ImGui::SetItemDefaultFocus();
			}
			if (ImGui::Selectable("frustrum", &frustrum))
			{
				cameraController.SetCameraType(CameraType::FRUSTRUM);
				ImGui::SetItemDefaultFocus();
			}
			ImGui::EndCombo();
		}

		if (cameraController.GetCameraType() == CameraType::PERSPECTIVE)
		{
			float fov = cameraController.GetCamera<PerspectiveCamera>().GetFOV();
			if (ImGui::DragFloat("fov", &fov, 0.3f, 1.0f, 179.0f))
				cameraController.GetCamera<PerspectiveCamera>().SetFOV(fov);
		}
		else if (cameraController.GetCameraType() == CameraType::ORTHOGRAPHIC)
		{
			auto size = cameraController.GetCamera<OrthographicCamera>().GetSize();
			if (ImGui::DragFloat("size", &size, 0.3f, 0.01f, 10000.0f))
				cameraController.GetCamera<OrthographicCamera>().SetSize(size);
		}
		else if (cameraController.GetCameraType() == CameraType::FRUSTRUM)
		{
			auto bounds = cameraController.GetCamera<FrustrumCamera>().GetBounds();
			if (ImGui::DragFloat2("center", &bounds[0], 0.01f))
				cameraController.GetCamera<FrustrumCamera>().SetBounds(bounds.x, bounds.y, bounds.z);
			if (ImGui::DragFloat("size", &bounds.z, 0.01f, 0.0f, 10000.0f))
				cameraController.GetCamera<FrustrumCamera>().SetBounds(bounds.x, bounds.y, bounds.z);
		}

		int bloomIterations = (int)cameraController.GetBloomIterations();
		float exposure = cameraController.GetExposure();
		float gamma = cameraController.GetGamma();
		float bloomWeight = cameraController.GetBloomWeight();
		auto direction = cameraController.GetDirectionDenormalized();
		float moveSpeed = cameraController.GetMoveSpeed();
		float rotateSpeed = cameraController.GetRotateSpeed();

		if (ImGui::DragFloat("move speed", &moveSpeed))
			cameraController.SetMoveSpeed(moveSpeed);
		if (ImGui::DragFloat("rotate speed", &rotateSpeed, 0.01f))
			cameraController.SetRotateSpeed(rotateSpeed);

		if (ImGui::DragFloat3("direction", &direction[0], 0.01f))
			cameraController.SetDirection(direction);
		if (ImGui::DragFloat("exposure", &exposure, 0.1f))
			cameraController.SetExposure(exposure);
		if (ImGui::DragFloat("gamma", &gamma, 0.01f, 0.0f, 5.0f))
			cameraController.SetGamma(gamma);
		if (ImGui::DragFloat("bloom weight", &bloomWeight, 0.1f))
			cameraController.SetBloomWeight(bloomWeight);
		if (ImGui::DragInt("bloom iterations", &bloomIterations))
			cameraController.SetBloomIterations((size_t)Max(0, bloomIterations));

		bool isRendered = cameraController.IsRendered();
		if (ImGui::Checkbox("enable rendering", &isRendered))
			cameraController.ToggleRendering(isRendered);

		ImGui::SameLine();
		if (ImGui::Button("listen window resize"))
		{
			cameraController.ListenWindowResizeEvent();
		}
		ImGui::SameLine();
		if (ImGui::Button("set as main viewport"))
		{
			auto cameraComponent = MxObject::GetComponentHandle(cameraController);
			Application::Get()->GetRenderAdaptor().Viewport = cameraComponent;
		}

		auto texture = cameraController.GetRenderTexture();
		DrawTextureEditor("output texture", texture, false);

		{
			SCOPE_TREE_NODE("attached camera");
			ImGui::PushID(0xFFFF);

			auto forward = cameraController.GetForwardVector();
			auto right = cameraController.GetRightVector();
			auto up = cameraController.GetUpVector();

			if (ImGui::DragFloat3("forward vec", &forward[0], 0.01f))
				cameraController.SetForwardVector(forward);
			if (ImGui::DragFloat3("right vec", &right[0], 0.01f))
				cameraController.SetRightVector(right);
			if (ImGui::DragFloat3("up vec", &up[0], 0.01f))
				cameraController.SetUpVector(up);

			float aspect = cameraController.Camera.GetAspectRatio();
			float zfar = cameraController.Camera.GetZFar();
			float znear = cameraController.Camera.GetZNear();
			float zoom = cameraController.Camera.GetZoom();

			ImGui::Separator();
			if (ImGui::DragFloat("aspect ratio", &aspect, 0.01f, 0.01f, 10.0f))
				cameraController.Camera.SetAspectRatio(aspect);
			if (ImGui::DragFloat("Z far", &zfar, 1.0f, 0.01f, std::numeric_limits<float>::max()))
				cameraController.Camera.SetZFar(zfar);
			if (ImGui::DragFloat("Z near", &znear, 0.001f, 0.001f, 10000.0f))
				cameraController.Camera.SetZNear(znear);
			if (ImGui::DragFloat("zoom", &zoom, 0.01f, 0.01f, 10000.0f))
				cameraController.Camera.SetZoom(zoom);


			ImGui::PopID();
		}
	}

	void VRCameraControllerEditor(VRCameraController& vrCameraController)
	{
		TREE_NODE_PUSH("VRCameraController");
		REMOVE_COMPONENT_BUTTON(vrCameraController);

		ImGui::DragFloat("eye distance", &vrCameraController.EyeDistance, 0.001f, 0.0f, 10.0f);
		ImGui::DragFloat("eye focus distance", &vrCameraController.FocusDistance, 0.01f, 0.1f, 100.0f);

		if (ImGui::TreeNode("left eye"))
		{
			if (vrCameraController.LeftEye.IsValid())
				CameraControllerEditor(*vrCameraController.LeftEye);
			else
				ImGui::Text("- no camera attached");
			ImGui::TreePop();
		}

		if (ImGui::TreeNode("right eye"))
		{
			if (vrCameraController.RightEye.IsValid())
				CameraControllerEditor(*vrCameraController.RightEye);
			else
				ImGui::Text("- no camera attached");
			ImGui::TreePop();
		}
	}

	void InputControlEditor(InputControl& inputControl)
	{
		TREE_NODE_PUSH("InputControl");
		REMOVE_COMPONENT_BUTTON(inputControl);

		// TODO: custom keys input
		if (ImGui::Button("bind movement WASD"))
			inputControl.BindMovement(KeyCode::W, KeyCode::A, KeyCode::S, KeyCode::D);

		if (ImGui::Button("bind movement ESDF"))
			inputControl.BindMovement(KeyCode::E, KeyCode::S, KeyCode::D, KeyCode::F);

		if (ImGui::Button("bind movement QWES"))
			inputControl.BindMovement(KeyCode::Q, KeyCode::W, KeyCode::E, KeyCode::S);

		if (ImGui::Button("bind rotation"))
			inputControl.BindRotation();

		if (ImGui::Button("bind horizontal rotation"))
			inputControl.BindHorizontalRotation();

		if (ImGui::Button("bind vertical rotation"))
			inputControl.BindVerticalRotation();

		if (ImGui::Button("bind movement WASD SPACE/LSHIFT"))
			inputControl.BindMovement(KeyCode::W, KeyCode::A, KeyCode::S, KeyCode::D, KeyCode::SPACE, KeyCode::LEFT_SHIFT);

		if (ImGui::Button("bind movement ESDF SPACE/LSHIFT"))
			inputControl.BindMovement(KeyCode::E, KeyCode::S, KeyCode::D, KeyCode::F, KeyCode::SPACE, KeyCode::LEFT_SHIFT);

		if (ImGui::Button("bind movement QWES SPACE/LSHIFT"))
			inputControl.BindMovement(KeyCode::Q, KeyCode::W, KeyCode::E, KeyCode::S, KeyCode::SPACE, KeyCode::LEFT_SHIFT);

		if (ImGui::Button("unbind all"))
		{
			auto& object = MxObject::GetByComponent(inputControl);
			object.RemoveComponent<InputControl>();
			object.AddComponent<InputControl>();
		}
	}
}