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

#include "Utilities/ImGui/ImGuiBase.h"
#include "Core/Application/Event.h"
#include "Core/Events/FpsUpdateEvent.h"

namespace MxEngine::GUI
{
	/*!
	draws fps graph in currenly active window. Listens to FpsUpdateEvent in global (Application) context
	\param graphRecordSize how many fps updates to track before refreshing (clearing). Each update happens each second.
	*/
	inline void DrawProfiler()
	{		
		static MxVector<float> fpsData;

		INVOKE_ONCE(Event::AddEventListener<FpsUpdateEvent>(
			"FpsGraph", [](FpsUpdateEvent& e) mutable
		{		
			constexpr size_t ProfilerGraphRecordSize = 128;
			fpsData.push_back((float)e.FPS);
			if (fpsData.size() > ProfilerGraphRecordSize)
				fpsData.erase(fpsData.begin());
		}));

		ImGui::PlotLines("", fpsData.data(), (int)fpsData.size(), 0, "FPS profiler",
			FLT_MAX, FLT_MAX, { ImGui::GetWindowWidth() - 15.0f, (float)fpsData.size() + 15.0f });
	}
}