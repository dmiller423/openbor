/*
*	Pause Menu Scene
*
*/
#include <string>
#include <vector>
using namespace std;

#include "../ps4.h"
#include "scene.h"



class PauseMenu
	: public Scene
{
	u32 width, height;

public:


	PauseMenu()
	{
		width = VideoOut::Get()->Width();
		height = VideoOut::Get()->Height();
	}


	void enter() {	}
	void leave() {	}

	void render()
	{
		auto scaling = 1.f;

		ImGui::SetNextWindowPos(ImVec2(0, 0));
		ImGui::SetNextWindowSize(ImVec2(width, height));
		ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0);

		ImGui::Begin("##main", NULL, ImGuiWindowFlags_NoDecoration);

		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(20 * scaling, 8 * scaling));		// from 8, 4
		ImGui::AlignTextToFramePadding();
		ImGui::Text("Pause Menu");

		ImGui::SameLine(ImGui::GetContentRegionAvailWidth() - ImGui::CalcTextSize("Resume").x - ImGui::GetStyle().FramePadding.x * 2.0f /*+ ImGui::GetStyle().ItemSpacing.x*/);
		if (ImGui::Button("Resume"))//, ImVec2(0, 30 * scaling)))
			running=true; // gui_state = Settings;
		ImGui::PopStyleVar();


		ImGui::EndChild();
		ImGui::End();
		ImGui::PopStyleVar();

	}
};