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

#include "RenderController.h"
#include "Utilities/Format/Format.h"
#include "Core/Components/Rendering/MeshRenderer.h"
#include "Core/Components/Camera/CameraController.h"
#include "Core/Components/Camera/CameraEffects.h"
#include "Core/Components/Camera/CameraToneMapping.h"
#include "Core/Components/Camera/CameraSSR.h"
#include "Core/Components/Lighting/DirectionalLight.h"
#include "Core/Components/Lighting/SpotLight.h"
#include "Core/Components/Lighting/PointLight.h"
#include "Core/Components/Rendering/Skybox.h"
#include "Utilities/Profiler/Profiler.h"
#include "RenderUtilities/ShadowMapGenerator.h"

namespace MxEngine
{
	constexpr size_t MaxDirLightCount = 4;

	void RenderController::PrepareShadowMaps()
	{
		MAKE_SCOPE_PROFILER("RenderController::PrepareShadowMaps()");

		ShadowMapGenerator generator(this->Pipeline.ShadowCasterUnits, this->Pipeline.MaterialUnits);

		{
			MAKE_SCOPE_PROFILER("RenderController::PrepareDirectionalLightMaps()");
			generator.GenerateFor(*this->Pipeline.Environment.Shaders["DepthTexture"_id], this->Pipeline.Lighting.DirectionalLights);
		}

		{
			MAKE_SCOPE_PROFILER("RenderController::PrepareSpotLightMaps()");
			generator.GenerateFor(*this->Pipeline.Environment.Shaders["DepthTexture"_id], this->Pipeline.Lighting.SpotLights);
		}

		{
			MAKE_SCOPE_PROFILER("RenderController::PreparePointLightMaps()");
			generator.GenerateFor(*this->Pipeline.Environment.Shaders["DepthCubeMap"_id], this->Pipeline.Lighting.PointLights);
		}
	}

	void RenderController::DrawObjects(const CameraUnit& camera, const Shader& shader, const MxVector<RenderUnit>& objects)
	{
		MAKE_SCOPE_PROFILER("RenderController::DrawObjects()");

		if (objects.empty()) return;
		shader.SetUniformMat4("ViewProjMatrix", camera.ViewProjectionMatrix);
		shader.SetUniformFloat("gamma", camera.Gamma);

		for (const auto& unit : objects)
		{
			bool isUnitVisible = unit.InstanceCount > 0 || camera.Culler.IsAABBVisible(unit.MinAABB, unit.MaxAABB);

			if (isUnitVisible) this->DrawObject(unit, shader);
		}
	}

	void RenderController::DrawObject(const RenderUnit& unit, const Shader& shader)
	{
		Texture::TextureBindId textureBindIndex = 0;
		const auto& material = this->Pipeline.MaterialUnits[unit.materialIndex];
		shader.IgnoreNonExistingUniform("material.transparency");

		material.AlbedoMap->Bind(textureBindIndex++);
		material.MetallicMap->Bind(textureBindIndex++);
		material.RoughnessMap->Bind(textureBindIndex++);
		material.EmmisiveMap->Bind(textureBindIndex++);
		material.NormalMap->Bind(textureBindIndex++);
		material.HeightMap->Bind(textureBindIndex++);
		material.AmbientOcclusionMap->Bind(textureBindIndex++);

		shader.SetUniformInt("map_albedo", material.AlbedoMap->GetBoundId());
		shader.SetUniformInt("map_metallic", material.MetallicMap->GetBoundId());
		shader.SetUniformInt("map_roughness", material.RoughnessMap->GetBoundId());
		shader.SetUniformInt("map_emmisive", material.EmmisiveMap->GetBoundId());
		shader.SetUniformInt("map_normal", material.NormalMap->GetBoundId());
		shader.SetUniformInt("map_height", material.HeightMap->GetBoundId());
		shader.SetUniformInt("map_occlusion", material.AmbientOcclusionMap->GetBoundId());

		shader.SetUniformFloat("material.roughness", material.RoughnessFactor);
		shader.SetUniformFloat("material.metallic", material.MetallicFactor);
		shader.SetUniformFloat("material.emmisive", material.Emmision);
		shader.SetUniformFloat("material.transparency", material.Transparency);

		shader.SetUniformFloat("displacement", material.Displacement);
		shader.SetUniformVec2("uvMultipliers", material.UVMultipliers);

		this->GetRenderEngine().SetDefaultVertexAttribute(5, unit.ModelMatrix); //-V807
		this->GetRenderEngine().SetDefaultVertexAttribute(9, unit.NormalMatrix);
		this->GetRenderEngine().SetDefaultVertexAttribute(12, material.BaseColor);
		
		this->GetRenderEngine().DrawTrianglesInstanced(*unit.VAO, *unit.IBO, shader, unit.InstanceCount);
	}

	void RenderController::ComputeBloomEffect(CameraUnit& camera)
	{
		if (camera.Effects == nullptr) return;
		auto iterations = 2 * camera.Effects->GetBloomIterations();
		if (iterations == 0) return;
		MAKE_SCOPE_PROFILER("RenderController::PerformBloomIterarations()");

		auto& bloomBuffers = this->Pipeline.Environment.BloomBuffers;
		auto& splitShader = this->Pipeline.Environment.Shaders["BloomSplit"_id];
		auto& iterShader = this->Pipeline.Environment.Shaders["BloomIteration"_id];

		float fogReduceFactor = this->Pipeline.Environment.FogDistance * std::exp(-25.0f * this->Pipeline.Environment.FogDensity);
		float bloomWeight = camera.Effects->GetBloomWeight() * fogReduceFactor;

		camera.AlbedoTexture->Bind(0);
		camera.MaterialTexture->Bind(1);
		splitShader->SetUniformInt("albedoTex", camera.AlbedoTexture->GetBoundId());
		splitShader->SetUniformInt("materialTex", camera.MaterialTexture->GetBoundId());
		splitShader->SetUniformFloat("weight", bloomWeight);
		this->RenderToFrameBuffer(bloomBuffers.back(), splitShader);

		// perform bloom iterations
		iterShader->SetUniformInt("BloomTexture", 0);
		for (uint8_t i = 0; i < iterations; i++)
		{
			auto& target = bloomBuffers[(i & 1)];
			auto& source = bloomBuffers[!(i & 1)];
			iterShader->SetUniformInt("horizontalKernel", int(i & 1));
			GetAttachedTexture(source)->Bind(0);
			this->RenderToFrameBuffer(target, iterShader);
		}
		auto result = GetAttachedTexture(bloomBuffers.back());
		result->GenerateMipmaps();
		
		// use additive blending to apply bloom to camera HDR image
		this->GetRenderEngine().UseBlending(BlendFactor::ONE, BlendFactor::ONE);
		this->AttachFrameBufferNoClear(this->Pipeline.Environment.PostProcessFrameBuffer);
		this->SubmitImage(result);
		this->GetRenderEngine().UseBlending(BlendFactor::ONE, BlendFactor::ZERO);
	}

	void RenderController::ApplyAmbientOcclusion(CameraUnit& camera, TextureHandle& input, TextureHandle& output)
	{
		if (camera.Effects == nullptr || camera.Effects->GetAmbientOcclusionSamples() == 0) return;
		MAKE_SCOPE_PROFILER("RenderController::ComputeAmbientOcclusion()");

		auto& computeShader = this->Pipeline.Environment.Shaders["AmbientOcclusion"_id];
		computeShader->IgnoreNonExistingUniform("materialTex");
		computeShader->IgnoreNonExistingUniform("albedoTex");
		this->BindGBuffer(camera, *computeShader);
		this->BindCameraInformation(camera, *computeShader);

		computeShader->SetUniformInt("sampleCount", (int)camera.Effects->GetAmbientOcclusionSamples());
		computeShader->SetUniformFloat("radius", camera.Effects->GetAmbientOcclusionRadius());
		computeShader->SetUniformFloat("intensity", camera.Effects->GetAmbientOcclusionIntensity());

		this->RenderToTexture(this->Pipeline.Environment.AmbientOcclusionTexture, computeShader);
		this->Pipeline.Environment.AmbientOcclusionTexture->GenerateMipmaps();

		auto& applyShader = this->Pipeline.Environment.Shaders["ApplyAmbientOcclusion"_id];
		input->Bind(0);
		this->Pipeline.Environment.AmbientOcclusionTexture->Bind(1);
		applyShader->SetUniformInt("inputTex", input->GetBoundId());
		applyShader->SetUniformInt("aoTex", this->Pipeline.Environment.AmbientOcclusionTexture->GetBoundId());

		this->RenderToTexture(output, applyShader);
		std::swap(input, output);
	}

	TextureHandle RenderController::ComputeAverageWhite(CameraUnit& camera)
	{
		MAKE_SCOPE_PROFILER("RenderController::ComputeAverageWhite()");
		MX_ASSERT(camera.ToneMapping != nullptr);
		camera.HDRTexture->GenerateMipmaps();

		float eyeAdaptation = 1.0f - std::exp(-camera.ToneMapping->GetEyeAdaptation() * this->Pipeline.Environment.TimeDelta);

		auto& shader = this->Pipeline.Environment.Shaders["AverageWhite"_id];
		auto& output = this->Pipeline.Environment.AverageWhiteTexture;
		camera.HDRTexture->Bind(0);
		camera.AverageWhiteTexture->Bind(1);
		shader->SetUniformInt("curFrameHDR", 0);
		shader->SetUniformInt("prevFrameWhite", 1);
		shader->SetUniformFloat("eyeAdaptation", eyeAdaptation);
		this->RenderToTexture(output, shader);
		output->GenerateMipmaps();
		this->CopyTexture(output, camera.AverageWhiteTexture);
		return output;
	}

	void RenderController::PerformPostProcessing(CameraUnit& camera)
	{
		MAKE_SCOPE_PROFILER("RenderController::PerformPostProcessing()");

		camera.AlbedoTexture->GenerateMipmaps();
		camera.MaterialTexture->GenerateMipmaps();
		camera.NormalTexture->GenerateMipmaps();
		camera.DepthTexture->GenerateMipmaps();

		this->ApplyAmbientOcclusion(camera, camera.HDRTexture, camera.SwapTexture);
		this->ApplySSR(camera, camera.HDRTexture, camera.SwapTexture);

		// render skybox & debug buffer (HDR texture is already attached)
		this->Pipeline.Environment.PostProcessFrameBuffer->AttachTexture(camera.HDRTexture, Attachment::COLOR_ATTACHMENT0);
		this->Pipeline.Environment.PostProcessFrameBuffer->AttachTexture(camera.DepthTexture, Attachment::DEPTH_ATTACHMENT);
		this->AttachFrameBufferNoClear(this->Pipeline.Environment.PostProcessFrameBuffer);
		this->GetRenderEngine().UseDepthBufferMask(false);
		this->DrawSkybox(camera);
		this->DrawTransparentObjects(camera);
		this->DrawDebugBuffer(camera);
		this->GetRenderEngine().UseDepthBufferMask(true);
		this->Pipeline.Environment.PostProcessFrameBuffer->DetachExtraTarget(Attachment::DEPTH_ATTACHMENT);

		this->ComputeBloomEffect(camera);
		this->ApplyChromaticAbberation(camera, camera.HDRTexture, camera.SwapTexture);
		this->ApplyFogEffect(camera, camera.HDRTexture, camera.SwapTexture);

		this->ApplyHDRToLDRConversion(camera, camera.HDRTexture, camera.SwapTexture);

		this->ApplyFXAA(camera, camera.HDRTexture, camera.SwapTexture);
		this->ApplyVignette(camera, camera.HDRTexture, camera.SwapTexture);
	}

	void RenderController::DrawDirectionalLights(CameraUnit& camera)
	{
		MAKE_SCOPE_PROFILER("RenderController::DrawDirectionalLights()");
		auto& shader = this->Pipeline.Environment.Shaders["GlobalIllumination"_id];

		shader->IgnoreNonExistingUniform("camera.viewProjMatrix");
		this->BindGBuffer(camera, *shader);
		this->BindCameraInformation(camera, *shader);

		Texture::TextureBindId textureId = 4;

		// submit directional light information
		constexpr size_t MaxDirLightCount = 4;
		const auto& dirLights = this->Pipeline.Lighting.DirectionalLights;
		size_t lightCount = Min(MaxDirLightCount, dirLights.size());

		camera.SkyboxTexture->Bind(textureId++);
		camera.IrradianceTexture->Bind(textureId++);
		shader->SetUniformInt("environment.skybox", camera.SkyboxTexture->GetBoundId());
		shader->SetUniformInt("environment.irradiance", camera.IrradianceTexture->GetBoundId());
		shader->SetUniformMat3("environment.skyboxRotation", camera.InverseSkyboxRotation);

		shader->SetUniformInt("lightCount", (int)lightCount);
		shader->SetUniformInt("pcfDistance", this->Pipeline.Environment.ShadowBlurIterations);
		shader->SetUniformInt("lightSamples", this->Pipeline.Environment.LightSamples);

		for (size_t i = 0; i < lightCount; i++)
		{
			auto& dirLight = this->Pipeline.Lighting.DirectionalLights[i];

			Vector4 colorPacked = Vector4(dirLight.Color * dirLight.Intensity, dirLight.AmbientIntensity);
			shader->SetUniformVec4(MxFormat("lights[{}].color", i), colorPacked);
			shader->SetUniformVec3(MxFormat("lights[{}].direction", i), dirLight.Direction);

			for (size_t j = 0; j < dirLight.ShadowMaps.size(); j++)
			{
				dirLight.ShadowMaps[j]->Bind(textureId++);
				shader->SetUniformInt(MxFormat("lightDepthMaps[{}][{}]", i, j), dirLight.ShadowMaps[j]->GetBoundId());
				shader->SetUniformMat4(MxFormat("lights[{}].transform[{}]", i, j), dirLight.BiasedProjectionMatrices[j]);
			}
		}

		this->Pipeline.Environment.DefaultShadowMap->Bind(textureId);
		for (size_t i = lightCount; i < MaxDirLightCount; i++)
		{
			for (size_t j = 0; j < DirectionalLight::TextureCount; j++)
			{
				shader->SetUniformInt(MxFormat("lightDepthMaps[{}][{}]", i, j),
					this->Pipeline.Environment.DefaultShadowMap->GetBoundId());
			}
		}
		// render global illumination
		this->RenderToTexture(camera.HDRTexture, shader);
	}

	void RenderController::PerformLightPass(CameraUnit& camera)
	{
		this->DrawDirectionalLights(camera);

		this->GetRenderEngine().UseBlending(BlendFactor::ONE, BlendFactor::ONE);
		this->ToggleFaceCulling(true, true, false);

		this->DrawShadowedSpotLights(camera);
		this->DrawNonShadowedSpotLights(camera);
		this->DrawShadowedPointLights(camera);
		this->DrawNonShadowedPointLights(camera);

		this->ToggleFaceCulling(true, true, true);
		this->GetRenderEngine().UseBlending(BlendFactor::ONE, BlendFactor::ZERO);
	}

	void RenderController::DrawTransparentObjects(CameraUnit& camera)
	{
		if (this->Pipeline.TransparentRenderUnits.empty()) return;
		MAKE_SCOPE_PROFILER("RenderController::DrawTransparentObjects()");

		this->GetRenderEngine().UseBlending(BlendFactor::SRC_ALPHA, BlendFactor::ONE_MINUS_SRC_ALPHA);
		this->ToggleFaceCulling(false);

		auto& shader = this->Pipeline.Environment.Shaders["Transparent"_id];

		shader->SetUniformVec3("viewportPosition", camera.ViewportPosition);
		shader->SetUniformFloat("gamma", camera.Gamma);

		Texture::TextureBindId textureId = Material::TextureCount;

		// submit directional light information
		const auto& dirLights = this->Pipeline.Lighting.DirectionalLights;
		size_t lightCount = Min(MaxDirLightCount, dirLights.size());

		camera.SkyboxTexture->Bind(textureId++);
		camera.IrradianceTexture->Bind(textureId++);
		shader->SetUniformInt("environment.skybox", camera.SkyboxTexture->GetBoundId());
		shader->SetUniformInt("environment.irradiance", camera.IrradianceTexture->GetBoundId());
		shader->SetUniformMat3("environment.skyboxRotation", camera.InverseSkyboxRotation);

		shader->SetUniformInt("lightCount", (int)lightCount);
		shader->SetUniformInt("pcfDistance", this->Pipeline.Environment.ShadowBlurIterations);
		shader->SetUniformInt("lightSamples", this->Pipeline.Environment.LightSamples);

		for (size_t i = 0; i < lightCount; i++)
		{
			auto& dirLight = this->Pipeline.Lighting.DirectionalLights[i];

			Vector4 colorPacked = Vector4(dirLight.Color * dirLight.Intensity, dirLight.AmbientIntensity);
			shader->SetUniformVec4(MxFormat("lights[{}].color", i), colorPacked);
			shader->SetUniformVec3(MxFormat("lights[{}].direction", i), dirLight.Direction);

			for (size_t j = 0; j < dirLight.ShadowMaps.size(); j++)
			{
				dirLight.ShadowMaps[j]->Bind(textureId++);
				shader->SetUniformInt(MxFormat("lightDepthMaps[{}][{}]", i, j), dirLight.ShadowMaps[j]->GetBoundId());
				shader->SetUniformMat4(MxFormat("lights[{}].transform[{}]", i, j), dirLight.BiasedProjectionMatrices[j]);
			}
		}

		this->Pipeline.Environment.DefaultShadowMap->Bind(textureId);
		for (size_t i = lightCount; i < MaxDirLightCount; i++)
		{
			for (size_t j = 0; j < DirectionalLight::TextureCount; j++)
			{
				shader->SetUniformInt(MxFormat("lightDepthMaps[{}][{}]", i, j),
					this->Pipeline.Environment.DefaultShadowMap->GetBoundId());
			}
		}

		this->DrawObjects(camera, *shader, this->Pipeline.TransparentRenderUnits);

		this->ToggleFaceCulling(true);
		this->GetRenderEngine().UseBlending(BlendFactor::ONE, BlendFactor::ZERO);
	}

	void RenderController::ApplyFogEffect(CameraUnit& camera, TextureHandle& input, TextureHandle& output)
	{
		if (this->Pipeline.Environment.FogDistance == 1.0 && this->Pipeline.Environment.FogDensity == 0.0f)
			return; // such parameters produce no fog. Do not do extra work calling this shader

		MAKE_SCOPE_PROFILER("RenderController::ApplyFogEffect()");

		auto fogShader = this->Pipeline.Environment.Shaders["Fog"_id];
		fogShader->IgnoreNonExistingUniform("camera.viewProjMatrix");
		fogShader->IgnoreNonExistingUniform("normalTex");
		fogShader->IgnoreNonExistingUniform("albedoTex");
		fogShader->IgnoreNonExistingUniform("materialTex");

		input->Bind(4);
		fogShader->SetUniformInt("cameraOutput", input->GetBoundId());

		this->BindGBuffer(camera, *fogShader);
		this->BindFogInformation(*fogShader);
		this->BindCameraInformation(camera, *fogShader);

		this->RenderToTexture(output, fogShader);
		std::swap(input, output);
	}

	void RenderController::ApplyChromaticAbberation(CameraUnit& camera, TextureHandle& input, TextureHandle& output)
	{
		if (camera.Effects == nullptr || camera.Effects->GetChromaticAberrationIntensity() <= 0.0f) return;
		MAKE_SCOPE_PROFILER("RenderController::ApplyChromaticAbberation()");

		auto& shader = this->Pipeline.Environment.Shaders["ChromaticAbberation"_id];
		input->Bind(0);
		shader->SetUniformInt("tex", input->GetBoundId());
		shader->SetUniformVec3("chromaticAbberationParams", {
			camera.Effects->GetChromaticAberrationMinDistance(),
			camera.Effects->GetChromaticAberrationIntensity(),
			camera.Effects->GetChromaticAberrationDistortion()
		});

		this->RenderToTexture(output, shader);
		std::swap(input, output);
	}

	void RenderController::ApplySSR(CameraUnit& camera, TextureHandle& input, TextureHandle& output)
	{
		if (camera.SSR == nullptr) return;
		MAKE_SCOPE_PROFILER("RenderController::ApplySSR()");
		input->GenerateMipmaps();

		auto& SSRShader = this->Pipeline.Environment.Shaders["SSR"_id];
		SSRShader->IgnoreNonExistingUniform("albedoTex");
		
		this->BindGBuffer(camera, *SSRShader);
		this->BindCameraInformation(camera, *SSRShader);
		input->Bind(4);
		SSRShader->SetUniformInt("HDRTex", input->GetBoundId());

		SSRShader->SetUniformFloat("thickness", camera.SSR->GetThickness());
		SSRShader->SetUniformFloat("maxCosAngle", camera.SSR->GetMaxCosAngle());
		SSRShader->SetUniformInt("steps", (int)camera.SSR->GetSteps());
		SSRShader->SetUniformFloat("maxDistance", camera.SSR->GetMaxDistance());

		this->RenderToTexture(output, SSRShader);
		std::swap(input, output);
	}

	void RenderController::ApplyHDRToLDRConversion(CameraUnit& camera, TextureHandle& input, TextureHandle& output)
	{
		if (camera.ToneMapping == nullptr) return;
		MAKE_SCOPE_PROFILER("RenderController::ApplyHDRToLDRConversion()");

		auto& HDRToLDRShader = this->Pipeline.Environment.Shaders["HDRToLDR"_id];
		auto averageWhite = this->ComputeAverageWhite(camera);
		auto aces = camera.ToneMapping->GetACESCoefficients();

		input->Bind(0);
		averageWhite->Bind(1);
		HDRToLDRShader->SetUniformInt("HDRTex", input->GetBoundId());
		HDRToLDRShader->SetUniformInt("averageWhiteTex", averageWhite->GetBoundId());

		HDRToLDRShader->SetUniformFloat("exposure", camera.ToneMapping->GetExposure());
		HDRToLDRShader->SetUniformFloat("colorMultiplier", camera.ToneMapping->GetColorScale());
		HDRToLDRShader->SetUniformFloat("whitePoint", camera.ToneMapping->GetWhitePoint());
		HDRToLDRShader->SetUniformFloat("minLuminance", camera.ToneMapping->GetMinLuminance());
		HDRToLDRShader->SetUniformFloat("maxLuminance", camera.ToneMapping->GetMaxLuminance());
		HDRToLDRShader->SetUniformVec3("ABCcoefsACES", { aces.A, aces.B, aces.C });
		HDRToLDRShader->SetUniformVec3("DEFcoefsACES", { aces.D, aces.E, aces.F });

		HDRToLDRShader->SetUniformFloat("gamma", camera.Gamma);

		this->RenderToTexture(output, HDRToLDRShader);
		std::swap(input, output);
	}

	void RenderController::ApplyFXAA(CameraUnit& camera, TextureHandle& input, TextureHandle& output)
	{
		if (camera.Effects == nullptr || !camera.Effects->IsFXAAEnabled()) return;
		MAKE_SCOPE_PROFILER("RenderController::ApplyFXAA");

		auto& fxaaShader = this->Pipeline.Environment.Shaders["FXAA"_id];
		input->Bind(0);
		fxaaShader->SetUniformInt("tex", input->GetBoundId());
		
		this->RenderToTexture(output, fxaaShader);
		std::swap(input, output);
	}

	void RenderController::ApplyVignette(CameraUnit& camera, TextureHandle& input, TextureHandle& output)
	{
		if (camera.Effects == nullptr || camera.Effects->GetVignetteRadius() <= 0.0f) return;
		MAKE_SCOPE_PROFILER("RenderController::ApplyVignette");

		auto& vignetteShader = this->Pipeline.Environment.Shaders["Vignette"_id];
		input->Bind(0);
		vignetteShader->SetUniformInt("tex", input->GetBoundId());

		vignetteShader->SetUniformFloat("radius", camera.Effects->GetVignetteRadius());
		vignetteShader->SetUniformFloat("intensity", camera.Effects->GetVignetteIntensity());

		this->RenderToTexture(output, vignetteShader);
		std::swap(input, output);
	}

	void RenderController::DrawShadowedSpotLights(CameraUnit& camera)
	{
		const auto& spotLights = this->Pipeline.Lighting.SpotLights;
		if (spotLights.empty()) return;
		MAKE_SCOPE_PROFILER("RenderController::DrawShadowedSpotLights()");

		auto shader = this->Pipeline.Environment.Shaders["SpotLight"_id];
		auto& pyramid = this->Pipeline.Lighting.PyramidLight;
		auto viewportSize = MakeVector2((float)camera.OutputTexture->GetWidth(), (float)camera.OutputTexture->GetHeight());

		shader->SetUniformVec2("viewportSize", viewportSize);
		shader->SetUniformInt("lightSamples", this->Pipeline.Environment.LightSamples);
		shader->SetUniformInt("pcfDistance", this->Pipeline.Environment.ShadowBlurIterations);
		shader->SetUniformInt("castsShadows", true);
		this->BindGBuffer(camera, *shader);
		this->BindCameraInformation(camera, *shader);

		Texture::TextureBindId textureId = 4;

		camera.SkyboxTexture->Bind(textureId++);
		camera.IrradianceTexture->Bind(textureId++);
		shader->SetUniformInt("environment.skybox", camera.SkyboxTexture->GetBoundId());
		shader->SetUniformInt("environment.irradiance", camera.IrradianceTexture->GetBoundId());
		shader->SetUniformMat3("environment.skyboxRotation", camera.InverseSkyboxRotation);

		shader->SetUniformInt("lightDepthMap", textureId);

		for (size_t i = 0; i < spotLights.size(); i++)
		{
			const auto& spotLight = spotLights[i];

			spotLight.ShadowMap->Bind(textureId);

			shader->SetUniformMat4("worldToLightTransform", spotLight.BiasedProjectionMatrix);

			this->GetRenderEngine().SetDefaultVertexAttribute(5,  spotLight.Transform);
			this->GetRenderEngine().SetDefaultVertexAttribute(9,  Vector4(spotLight.Position, spotLight.InnerAngle));
			this->GetRenderEngine().SetDefaultVertexAttribute(10, Vector4(spotLight.Direction, spotLight.OuterAngle));
			this->GetRenderEngine().SetDefaultVertexAttribute(11, Vector4(spotLight.Color, spotLight.AmbientIntensity));

			this->GetRenderEngine().DrawTriangles(pyramid.GetVAO(), pyramid.GetIBO(), *shader);
		}
	}

	void RenderController::DrawShadowedPointLights(CameraUnit& camera)
	{
		const auto& pointLights = this->Pipeline.Lighting.PointLights;
		if (pointLights.empty()) return;
		MAKE_SCOPE_PROFILER("RenderController::DrawShadowedPointLights()");

		auto shader = this->Pipeline.Environment.Shaders["PointLight"_id];
		auto& sphere = this->Pipeline.Lighting.SphereLight;
		auto viewportSize = MakeVector2((float)camera.OutputTexture->GetWidth(), (float)camera.OutputTexture->GetHeight());

		shader->SetUniformVec2("viewportSize", viewportSize);
		shader->SetUniformInt("lightSamples", this->Pipeline.Environment.LightSamples);
		shader->SetUniformInt("castsShadows", true);
		this->BindGBuffer(camera, *shader);
		this->BindCameraInformation(camera, *shader);

		Texture::TextureBindId textureId = 4;

		camera.SkyboxTexture->Bind(textureId++);
		camera.IrradianceTexture->Bind(textureId++);
		shader->SetUniformInt("environment.skybox", camera.SkyboxTexture->GetBoundId());
		shader->SetUniformInt("environment.irradiance", camera.IrradianceTexture->GetBoundId());
		shader->SetUniformMat3("environment.skyboxRotation", camera.InverseSkyboxRotation);

		shader->SetUniformInt("lightDepthMap", textureId);

		for (size_t i = 0; i < pointLights.size(); i++)
		{
			const auto& pointLight = pointLights[i];

			pointLight.ShadowMap->Bind(textureId);

			this->GetRenderEngine().SetDefaultVertexAttribute(5,  pointLight.Transform);
			this->GetRenderEngine().SetDefaultVertexAttribute(9,  Vector4(pointLight.Position, pointLight.Radius));
			this->GetRenderEngine().SetDefaultVertexAttribute(10, Vector4(pointLight.Color, pointLight.AmbientIntensity));

			this->GetRenderEngine().DrawTriangles(sphere.GetVAO(), sphere.GetIBO(), *shader);
		}
	}

	void RenderController::DrawNonShadowedPointLights(CameraUnit& camera)
	{
		auto& instancedPointLights = this->Pipeline.Lighting.PointLigthsInstanced;
		if (instancedPointLights.Instances.empty()) return;
		MAKE_SCOPE_PROFILER("RenderController::DrawNonShadowedPointLights()");

		auto shader = this->Pipeline.Environment.Shaders["PointLight"_id];
		auto viewportSize = MakeVector2((float)camera.OutputTexture->GetWidth(), (float)camera.OutputTexture->GetHeight());

		this->BindGBuffer(camera, *shader);
		this->BindCameraInformation(camera, *shader);

		Texture::TextureBindId textureId = 4;

		camera.SkyboxTexture->Bind(textureId++);
		camera.IrradianceTexture->Bind(textureId++);
		shader->SetUniformInt("environment.skybox", camera.SkyboxTexture->GetBoundId());
		shader->SetUniformInt("environment.irradiance", camera.IrradianceTexture->GetBoundId());
		shader->SetUniformMat3("environment.skyboxRotation", camera.InverseSkyboxRotation);

		this->Pipeline.Environment.DefaultBlackCubeMap->Bind(textureId);
		shader->SetUniformInt("lightDepthMap", this->Pipeline.Environment.DefaultBlackCubeMap->GetBoundId());
		shader->SetUniformInt("lightSamples", this->Pipeline.Environment.LightSamples);
		shader->SetUniformVec2("viewportSize", viewportSize);
		shader->SetUniformInt("castsShadows", false);

		instancedPointLights.SubmitToVBO();
		this->GetRenderEngine().DrawTrianglesInstanced(instancedPointLights.GetVAO(), instancedPointLights.GetIBO(), *shader, instancedPointLights.Instances.size());
	}

	void RenderController::DrawNonShadowedSpotLights(CameraUnit& camera)
	{
		auto& instancedSpotLights = this->Pipeline.Lighting.SpotLightsInstanced;
		if (instancedSpotLights.Instances.empty()) return;
		MAKE_SCOPE_PROFILER("RenderController::DrawNonShadowedSpotLights()");

		auto shader = this->Pipeline.Environment.Shaders["SpotLight"_id];
		auto viewportSize = MakeVector2((float)camera.OutputTexture->GetWidth(), (float)camera.OutputTexture->GetHeight());

		this->BindGBuffer(camera, *shader);
		this->BindCameraInformation(camera, *shader);

		Texture::TextureBindId textureId = 4;

		camera.SkyboxTexture->Bind(textureId++);
		camera.IrradianceTexture->Bind(textureId++);
		shader->SetUniformInt("environment.skybox", camera.SkyboxTexture->GetBoundId());
		shader->SetUniformInt("environment.irradiance", camera.IrradianceTexture->GetBoundId());
		shader->SetUniformMat3("environment.skyboxRotation", camera.InverseSkyboxRotation);

		this->Pipeline.Environment.DefaultBlackCubeMap->Bind(textureId);
		shader->SetUniformInt("lightDepthMap", this->Pipeline.Environment.DefaultBlackCubeMap->GetBoundId());
		shader->SetUniformInt("lightSamples", this->Pipeline.Environment.LightSamples);
		shader->SetUniformVec2("viewportSize", viewportSize);
		shader->SetUniformInt("castsShadows", false);

		instancedSpotLights.SubmitToVBO();
		this->GetRenderEngine().DrawTrianglesInstanced(instancedSpotLights.GetVAO(), instancedSpotLights.GetIBO(), *shader, instancedSpotLights.Instances.size());
	}

	void RenderController::BindFogInformation(const Shader& shader)
	{
		shader.SetUniformFloat("fog.distance", this->Pipeline.Environment.FogDistance);
		shader.SetUniformFloat("fog.density", this->Pipeline.Environment.FogDensity);
		shader.SetUniformVec3("fog.color", this->Pipeline.Environment.FogColor);
	}

	void RenderController::BindCameraInformation(const CameraUnit& camera, const Shader& shader)
	{
		shader.SetUniformVec3("camera.position", camera.ViewportPosition);
		shader.SetUniformMat4("camera.viewProjMatrix", camera.ViewProjectionMatrix);
		shader.SetUniformMat4("camera.invViewProjMatrix", camera.InverseViewProjMatrix);
	}

	void RenderController::BindGBuffer(const CameraUnit& camera, const Shader& shader)
	{
		camera.AlbedoTexture->Bind(0);
		camera.NormalTexture->Bind(1);
		camera.MaterialTexture->Bind(2);
		camera.DepthTexture->Bind(3);

		shader.SetUniformInt("albedoTex", camera.AlbedoTexture->GetBoundId());
		shader.SetUniformInt("normalTex", camera.NormalTexture->GetBoundId());
		shader.SetUniformInt("materialTex", camera.MaterialTexture->GetBoundId());
		shader.SetUniformInt("depthTex", camera.DepthTexture->GetBoundId());
	}

	const Renderer& RenderController::GetRenderEngine() const
	{
		return this->renderer;
	}

	Renderer& RenderController::GetRenderEngine()
	{
		return this->renderer;
	}

	void RenderController::Render() const
	{
		this->GetRenderEngine().Flush();
	}

	void RenderController::Clear() const
	{
		this->GetRenderEngine().Clear();
	}

	void RenderController::AttachDepthMap(const TextureHandle& texture)
	{
		auto& framebuffer = this->Pipeline.Environment.DepthFrameBuffer;
		framebuffer->AttachTexture(texture, Attachment::DEPTH_ATTACHMENT);
		this->AttachFrameBuffer(framebuffer);
	}

	void RenderController::AttachDepthMap(const CubeMapHandle& cubemap)
	{
		auto& framebuffer = this->Pipeline.Environment.DepthFrameBuffer;
		framebuffer->AttachCubeMap(cubemap, Attachment::DEPTH_ATTACHMENT);
		this->AttachFrameBuffer(framebuffer);
	}

	void RenderController::AttachFrameBuffer(const FrameBufferHandle& framebuffer)
	{
		this->AttachFrameBufferNoClear(framebuffer);
		this->Clear();
	}

	void RenderController::AttachFrameBufferNoClear(const FrameBufferHandle& framebuffer)
	{
		framebuffer->Bind();
		this->SetViewport(0, 0, (int)framebuffer->GetWidth(), (int)framebuffer->GetHeight());
	}

	void RenderController::AttachDefaultFrameBuffer()
	{
		// we can unbind any buffer to set target framebuffer to default one (id = 0)
		// doing this allows us to render to application window
		this->Pipeline.Environment.DepthFrameBuffer->Unbind();
		this->SetViewport(0, 0, this->Pipeline.Environment.Viewport.x, this->Pipeline.Environment.Viewport.y);
		this->Clear();
	}

	void RenderController::RenderToFrameBuffer(const FrameBufferHandle& framebuffer, const ShaderHandle& shader)
	{
		this->AttachFrameBuffer(framebuffer);
		auto& rectangle = this->Pipeline.Environment.RectangularObject;
		this->GetRenderEngine().DrawTriangles(rectangle.GetVAO(), rectangle.VertexCount, *shader);
	}

	void RenderController::RenderToTexture(const TextureHandle& texture, const ShaderHandle& shader, Attachment attachment)
	{
		this->Pipeline.Environment.PostProcessFrameBuffer->AttachTexture(texture, attachment);
		this->RenderToFrameBuffer(this->Pipeline.Environment.PostProcessFrameBuffer, shader);
	}

	void RenderController::CopyTexture(const TextureHandle& input, const TextureHandle& output)
	{
		MAKE_SCOPE_PROFILER("RenderController::CopyTexture");
		this->Pipeline.Environment.PostProcessFrameBuffer->AttachTexture(output);
		this->AttachFrameBufferNoClear(this->Pipeline.Environment.PostProcessFrameBuffer);
		this->SubmitImage(input);
	}

    void RenderController::ToggleDepthOnlyMode(bool value)
    {
		bool useColor = !value;
		this->GetRenderEngine().UseColorMask(useColor, useColor, useColor, useColor);
    }

	void RenderController::ToggleReversedDepth(bool value)
	{
		this->GetRenderEngine().UseReversedDepth(value);
	}

	void RenderController::ToggleFaceCulling(bool value, bool counterClockWise, bool cullBack)
	{
		this->GetRenderEngine().UseCulling(value, counterClockWise, cullBack);
	}

	void RenderController::SetAnisotropicFiltering(float value)
	{
		this->GetRenderEngine().UseAnisotropicFiltering(value);
	}

	void RenderController::SetViewport(int x, int y, int width, int height)
	{
		this->GetRenderEngine().SetViewport(x, y, width, height);
	}

	void RenderController::DrawSkybox(const CameraUnit& camera)
	{
		MAKE_SCOPE_PROFILER("RenderController::DrawSkybox()");

		auto& shader = *this->Pipeline.Environment.Shaders["Skybox"_id];
		auto& skybox = this->Pipeline.Environment.SkyboxCubeObject;

		float skyLuminance = 0.0f;
		for (size_t i = 0; i < Min(this->Pipeline.Lighting.DirectionalLights.size(), MaxDirLightCount); i++)
		{
			const auto& dirLight = this->Pipeline.Lighting.DirectionalLights[i];
			skyLuminance += dirLight.Intensity;
		}

		shader.SetUniformMat4("StaticViewProjection", camera.StaticViewProjectionMatrix);
		shader.SetUniformMat3("Rotation", Transpose(camera.InverseSkyboxRotation));
		shader.SetUniformFloat("gamma", camera.Gamma);
		shader.SetUniformFloat("luminance", skyLuminance);

		camera.SkyboxTexture->Bind(0);
		shader.SetUniformInt("skybox", camera.SkyboxTexture->GetBoundId());

		this->GetRenderEngine().DrawTriangles(skybox.GetVAO(), skybox.VertexCount, shader);
	}

	void RenderController::DrawDebugBuffer(const CameraUnit& camera)
	{
		if (this->Pipeline.Environment.DebugBufferObject.VertexCount == 0) return;
		MAKE_SCOPE_PROFILER("RenderController::DrawDebugBuffer()");

		this->GetRenderEngine().UseDepthBuffer(!this->Pipeline.Environment.OverlayDebugDraws); //-V807

		auto& shader = *this->Pipeline.Environment.Shaders["DebugDraw"_id];
		shader.SetUniformMat4("ViewProjMatrix", camera.ViewProjectionMatrix);

		this->GetRenderEngine().DrawLines(*this->Pipeline.Environment.DebugBufferObject.VAO, this->Pipeline.Environment.DebugBufferObject.VertexCount, shader);

		this->GetRenderEngine().UseDepthBuffer(true);
	}

	EnvironmentUnit& RenderController::GetEnvironment()
	{
		return this->Pipeline.Environment;
	}

	const EnvironmentUnit& RenderController::GetEnvironment() const
	{
		return this->Pipeline.Environment;
	}

	LightingSystem& RenderController::GetLightInformation()
	{
		return this->Pipeline.Lighting;
	}

	const LightingSystem& RenderController::GetLightInformation() const
	{
		return this->Pipeline.Lighting;
	}

	void RenderController::ResetPipeline()
	{
		this->Pipeline.Lighting.DirectionalLights.clear();
		this->Pipeline.Lighting.PointLigthsInstanced.Instances.clear();
		this->Pipeline.Lighting.SpotLightsInstanced.Instances.clear();
		this->Pipeline.Lighting.PointLights.clear();
		this->Pipeline.Lighting.SpotLights.clear();
		this->Pipeline.OpaqueRenderUnits.clear();
		this->Pipeline.TransparentRenderUnits.clear();
		this->Pipeline.ShadowCasterUnits.clear();
		this->Pipeline.MaterialUnits.clear();
		this->Pipeline.Cameras.clear();
	}

	void RenderController::SubmitLightSource(const DirectionalLight& light, const TransformComponent& parentTransform)
	{
		auto& dirLight = this->Pipeline.Lighting.DirectionalLights.emplace_back();
		MX_ASSERT(dirLight.ShadowMaps.size() == DirectionalLight::TextureCount);

		dirLight.AmbientIntensity = light.GetAmbientIntensity();
		dirLight.Intensity = light.GetIntensity();
		dirLight.Color = light.GetColor();
		dirLight.Direction = light.Direction;

		for (size_t i = 0; i < DirectionalLight::TextureCount; i++)
		{
			dirLight.ShadowMaps[i] = light.GetDepthTexture(i);
			dirLight.ProjectionMatrices[i] = light.GetMatrix(parentTransform.GetPosition(), i);
			dirLight.BiasedProjectionMatrices[i] = MakeBiasMatrix() * dirLight.ProjectionMatrices[i];
		}
	}

	void RenderController::SubmitLightSource(const PointLight& light, const TransformComponent& parentTransform)
	{
		PointLightBaseData* baseLightData = nullptr;
		if (light.IsCastingShadows())
		{
			auto& pointLight = this->Pipeline.Lighting.PointLights.emplace_back();
			baseLightData = &pointLight;

			pointLight.ShadowMap = light.GetDepthCubeMap();
			for (size_t i = 0; i < std::size(pointLight.ProjectionMatrices); i++)
				pointLight.ProjectionMatrices[i] = light.GetMatrix(i, parentTransform.GetPosition());
		}
		else
		{
			auto& pointLight = this->Pipeline.Lighting.PointLigthsInstanced.Instances.emplace_back();
			baseLightData = &pointLight;
		}

		baseLightData->AmbientIntensity = light.GetAmbientIntensity();
		baseLightData->Color = light.GetIntensity() * light.GetColor();
		baseLightData->Position = parentTransform.GetPosition();
		baseLightData->Radius = light.GetRadius();
		baseLightData->Transform = light.GetSphereTransform(parentTransform.GetPosition());
	}

	void RenderController::SubmitLightSource(const SpotLight& light, const TransformComponent& parentTransform)
	{
		SpotLightBaseData* baseLightData = nullptr;
		if (light.IsCastingShadows())
		{
			auto& spotLight = this->Pipeline.Lighting.SpotLights.emplace_back();
			baseLightData = &spotLight;

			spotLight.ProjectionMatrix = light.GetMatrix(parentTransform.GetPosition());
			spotLight.BiasedProjectionMatrix = MakeBiasMatrix() * light.GetMatrix(parentTransform.GetPosition());
			spotLight.ShadowMap = light.GetDepthTexture();
		}
		else
		{
			auto& spotLight = this->Pipeline.Lighting.SpotLightsInstanced.Instances.emplace_back();
			baseLightData = &spotLight;
		}

		baseLightData->AmbientIntensity = light.GetAmbientIntensity();
		baseLightData->Color = light.GetIntensity() * light.GetColor();
		baseLightData->Position = parentTransform.GetPosition();
		baseLightData->Direction = light.Direction;
		baseLightData->Transform = light.GetPyramidTransform(parentTransform.GetPosition());
		baseLightData->InnerAngle = light.GetInnerCos();
		baseLightData->OuterAngle = light.GetOuterCos();
	}

	void RenderController::SubmitCamera(const CameraController& controller, const TransformComponent& parentTransform, 
		const Skybox& skybox, const CameraEffects* effects, const CameraToneMapping* toneMapping, const CameraSSR* ssr)
	{
		auto& camera = this->Pipeline.Cameras.emplace_back();

		camera.ViewportPosition           = parentTransform.GetPosition();
		camera.StaticViewProjectionMatrix = controller.GetMatrix(MakeVector3(0.0f));
		camera.ViewProjectionMatrix       = controller.GetMatrix(parentTransform.GetPosition());
		camera.InverseViewProjMatrix      = Inverse(camera.ViewProjectionMatrix);
		camera.Culler                     = controller.GetFrustrumCuller();
		camera.IsPerspective              = controller.GetCameraType() == CameraType::PERSPECTIVE;
		camera.GBuffer                    = controller.GetGBuffer();
		camera.AlbedoTexture              = controller.GetAlbedoTexture();
		camera.NormalTexture              = controller.GetNormalTexture();
		camera.MaterialTexture            = controller.GetMaterialTexture();
		camera.DepthTexture               = controller.GetDepthTexture();
		camera.AverageWhiteTexture        = controller.GetAverageWhiteTexture();
		camera.HDRTexture                 = controller.GetHDRTexture();
		camera.SwapTexture                = controller.GetSwapHDRTexture();
		camera.OutputTexture              = controller.GetRenderTexture();
		camera.RenderToTexture            = controller.IsRendered();
		camera.InverseSkyboxRotation      = Transpose(ToMatrix(skybox.GetRotation()));
		camera.SkyboxTexture              = skybox.CubeMap.IsValid() ? skybox.CubeMap : this->Pipeline.Environment.DefaultBlackCubeMap;
		camera.IrradianceTexture          = skybox.Irradiance.IsValid() ? skybox.Irradiance : camera.SkyboxTexture;
		camera.Gamma                      = toneMapping == nullptr ? 1.0f : toneMapping->GetGamma();
		camera.Effects                    = effects;
		camera.ToneMapping                = toneMapping;
		camera.SSR                        = ssr;
	}

    void RenderController::SubmitPrimitive(const SubMesh& object, const Material& material, const TransformComponent& parentTransform, size_t instanceCount)
    {
		RenderUnit* primitivePtr = nullptr;
		// filter transparent object to render in separate order
		if (material.Transparency < 1.0f)
			primitivePtr = &this->Pipeline.TransparentRenderUnits.emplace_back();
		else
			primitivePtr = &this->Pipeline.OpaqueRenderUnits.emplace_back();
		auto& primitive = *primitivePtr;

		primitive.VAO = object.Data.GetVAO();
		primitive.IBO = object.Data.GetIBO();
		primitive.materialIndex = this->Pipeline.MaterialUnits.size();
		primitive.ModelMatrix  = parentTransform.GetMatrix() * object.GetTransform()->GetMatrix(); //-V807
		primitive.NormalMatrix = parentTransform.GetNormalMatrix() * object.GetTransform()->GetNormalMatrix();
		primitive.InstanceCount = instanceCount;

		// compute aabb of primitive object for later frustrum culling
		auto aabb = object.Data.GetBoundingBox() * primitive.ModelMatrix;
		primitive.MinAABB = aabb.Min;
		primitive.MaxAABB = aabb.Max;

		auto& renderMaterial = this->Pipeline.MaterialUnits.emplace_back(material); // create a copy of material for future work

		// we need to change displacement to account object scale, so we take average of object scale components as multiplier
		renderMaterial.Displacement *= Dot(parentTransform.GetScale() * object.GetTransform()->GetScale(), MakeVector3(1.0f / 3.0f));

		if (renderMaterial.RoughnessMap.IsValid())         renderMaterial.RoughnessFactor = 1.0f;
		if (renderMaterial.MetallicMap.IsValid())          renderMaterial.MetallicFactor = 1.0f;

		// set default textures if they are not exist
		if (!renderMaterial.AlbedoMap.IsValid())           renderMaterial.AlbedoMap           = this->Pipeline.Environment.DefaultMaterialMap;
		if (!renderMaterial.RoughnessMap.IsValid())        renderMaterial.RoughnessMap        = this->Pipeline.Environment.DefaultMaterialMap;
		if (!renderMaterial.MetallicMap.IsValid())         renderMaterial.MetallicMap         = this->Pipeline.Environment.DefaultMaterialMap;
		if (!renderMaterial.EmmisiveMap.IsValid())         renderMaterial.EmmisiveMap         = this->Pipeline.Environment.DefaultMaterialMap;
		if (!renderMaterial.AmbientOcclusionMap.IsValid()) renderMaterial.AmbientOcclusionMap = this->Pipeline.Environment.DefaultMaterialMap;
		if (!renderMaterial.NormalMap.IsValid())           renderMaterial.NormalMap           = this->Pipeline.Environment.DefaultNormalMap;
		if (!renderMaterial.HeightMap.IsValid())           renderMaterial.HeightMap           = this->Pipeline.Environment.DefaultBlackMap;

		if (renderMaterial.CastsShadow) this->Pipeline.ShadowCasterUnits.push_back(primitive);
    }

	void RenderController::SubmitImage(const TextureHandle& texture)
	{
		auto& finalShader = *this->Pipeline.Environment.Shaders["ImageForward"_id];
		auto& rectangle = this->Pipeline.Environment.RectangularObject;

		finalShader.SetUniformInt("tex", 0);
		texture->Bind(0);

		this->GetRenderEngine().DrawTriangles(rectangle.GetVAO(), rectangle.VertexCount, finalShader);
	}

	void RenderController::StartPipeline()
	{
		MAKE_SCOPE_PROFILER("RenderController::StartPipeline()");
		if (this->Pipeline.Cameras.empty())
		{
			if(this->Pipeline.Environment.RenderToDefaultFrameBuffer)
				this->AttachDefaultFrameBuffer();
			return;
		}

		this->PrepareShadowMaps();

		for (auto& camera : this->Pipeline.Cameras)
		{
			if (!camera.RenderToTexture) continue;

			this->GetRenderEngine().UseBlending(BlendFactor::ONE, BlendFactor::ZERO);
			this->ToggleReversedDepth(camera.IsPerspective);
			this->AttachFrameBuffer(camera.GBuffer);

			this->DrawObjects(camera, *this->Pipeline.Environment.Shaders["GBuffer"_id], this->Pipeline.OpaqueRenderUnits);
			this->PerformLightPass(camera);
			this->PerformPostProcessing(camera);

			this->CopyTexture(camera.HDRTexture, camera.OutputTexture);
			//this->CopyTexture(this->Pipeline.Environment.AmbientOcclusionTexture, camera.OutputTexture);
			camera.OutputTexture->GenerateMipmaps();
		}
	}

	void RenderController::EndPipeline()
	{
		MAKE_SCOPE_PROFILER("RenderController::SubmitFinalImage");
		this->AttachDefaultFrameBuffer();
		if (this->Pipeline.Environment.RenderToDefaultFrameBuffer && this->Pipeline.Environment.MainCameraIndex < this->Pipeline.Cameras.size())
		{
			const auto& mainCamera = this->Pipeline.Cameras[this->Pipeline.Environment.MainCameraIndex];
			MX_ASSERT(mainCamera.OutputTexture.IsValid());

			this->SubmitImage(mainCamera.OutputTexture);
		}
	}
}