#pragma once

#include "../ps4.h"
#include "../imgui/imgui.h"



class IRenderer
{
public:

	virtual ~IRenderer() {};



//	virtual bool init()=0;
//	virtual void term()=0;





	// imgui //

	virtual IMGUI_API bool	ImGui_Init(void *initData = nullptr) = 0;
	virtual IMGUI_API void	ImGui_Shutdown() = 0;
	virtual IMGUI_API void	ImGui_NewFrame(void *frameData = nullptr) = 0;
//	virtual IMGUI_API bool	ImGui_ProcessEvent(void* eventData = nullptr) = 0;

	// Use if you want to reset your rendering device without losing ImGui state.
	virtual IMGUI_API bool	ImGui_CreateDeviceObjects() = 0;
	virtual IMGUI_API void	ImGui_InvalidateDeviceObjects() = 0;

//	virtual IMGUI_API bool	ImGui_CreateFontsTexture() = 0;
//	virtual IMGUI_API void	ImGui_DestroyFontsTexture() = 0;
};





