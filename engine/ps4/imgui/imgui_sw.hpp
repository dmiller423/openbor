// By Emil Ernerfeldt 2018
// LICENSE:
//   This software is dual-licensed to the public domain and under the following
//   license: you are granted a perpetual, irrevocable license to copy, modify,
//   publish, and distribute this file as you see fit.
// WHAT:
//   This is a software renderer for Dear ImGui.
//   It is decently fast, but has a lot of room for optimization.
//   The goal was to get something fast and decently accurate in not too many lines of code.
// LIMITATIONS:
//   * It is not pixel-perfect, but it is good enough for must use cases.
//   * It does not support painting with any other texture than the default font texture.
#pragma once

#include "../ps4.h"
#include "../GpuAlloc.h"
#include "../gui/Renderer.h"


namespace imgui_sw
{
	struct SwOptions
	{
		bool optimize_text = true;  // No reason to turn this off.
		bool optimize_rectangles = true; // No reason to turn this off.
	};
} // namespace imgui_sw





class SoftRend1
	: public IRenderer
{
	GpuAlloc buffAlloc;	// use onion backbuffer for SoftRend*, it needs to read!
	addr_t rendBuffer;

	bool initBuffer();

public:

	~SoftRend1()
	{
		buffAlloc.term();
	}

	SoftRend1()
		: rendBuffer(0)
	{
		if (!buffAlloc.init(VideoOut::Get()->BufferSize(), MB<size_t>(2))) {
			printf("Error, failed to init render buffer allocator!\n");
			return;
		}
		rendBuffer = buffAlloc.getBaseAddr();
	}


	// imgui //

	IMGUI_API bool	ImGui_Init(void *initData = nullptr);
	IMGUI_API void	ImGui_Shutdown();
	IMGUI_API void	ImGui_NewFrame(void *frameData = nullptr);
	//	IMGUI_API bool	ImGui_ProcessEvent(void* eventData = nullptr);

		// Use if you want to reset your rendering device without losing ImGui state.
	IMGUI_API bool	ImGui_CreateDeviceObjects();
	IMGUI_API void	ImGui_InvalidateDeviceObjects();

	//	virtual IMGUI_API bool	ImGui_CreateFontsTexture() = 0;
	//	virtual IMGUI_API void	ImGui_DestroyFontsTexture() = 0;
};



