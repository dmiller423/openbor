#pragma once

/*
*	Filesystem Browser Scene
*
*/
#include <string>
#include <vector>
using namespace std;

#include "../ps4.h"
#include "scene.h"

bool tryToRun(string);

class FSBrowser
	: public Scene
{

	string currPath;
	vector<char> dentBuff;
	vector<string> entries;

	u32 width, height;

	ImGuiTextFilter filter;
public:

	FSBrowser(const char* homePath = nullptr)
		: currPath(defPath())
	{
		if (homePath)
			currPath = string(homePath);

		width  = VideoOut::Get()->Width();
		height = VideoOut::Get()->Height();

		//filter.	*FIXME* todo add per console filter file extensions ?
		filter.Clear();
	}




	void enter() {
		refresh();
	}

	void leave() {
		// don't care //
	}

	void render()
	{
		auto scaling = 1.f;

		ImGui::SetNextWindowPos(ImVec2(0, 0));
		ImGui::SetNextWindowSize(ImVec2(width, height));
		ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0);

		ImGui::Begin("##main", NULL, ImGuiWindowFlags_NoDecoration);

		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(20 * scaling, 8 * scaling));		// from 8, 4
		ImGui::AlignTextToFramePadding();
		ImGui::Text("Current Directory");

		ImGui::SameLine(ImGui::GetContentRegionAvailWidth() - ImGui::CalcTextSize("Settings").x - ImGui::GetStyle().FramePadding.x * 2.0f /*+ ImGui::GetStyle().ItemSpacing.x*/);
		if (ImGui::Button("Settings"))//, ImVec2(0, 30 * scaling)))
			; // gui_state = Settings;
		ImGui::PopStyleVar();

		//fetch_game_list();

		ImGui::BeginChild(ImGui::GetID("library"), ImVec2(0, 0), true);
		{
			ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(8 * scaling, 20 * scaling));		// from 8, 4


			for (auto game : entries) {
			//	if (filter.IsActive() &&
			//		filter.PassFilter(game.c_str()))
				{
				//	ImGui::PushID(game.c_str());
					if (ImGui::Selectable(game.c_str()))
					{
						//gui_state = ClosedNoResume;
						//gui_start_game(game.path);
						printf(" SELECTED \"%s\" \n", game.c_str());

						if (string(".") == game) {	// how does break cause a crash here ??
							refresh();
							//break;
						}
						else if (string("..") == game) {
							cdUp();
							//break;
						}

						else if (!cdTo(game))
							if (tryToRun(currPath + "/" + game))
								;//Hide();
					}
				//	ImGui::PopID();
				}
			}
			ImGui::PopStyleVar();
		}


		ImGui::EndChild();
		ImGui::End();
		ImGui::PopStyleVar();

	}

	string homePath() {
		return string("/data");
	}

	string defPath() {
		return homePath();
	}

	void cdUp()
	{
		if (currPath.empty()) {
			currPath = string("/");
			return;
		}

		size_t csz = currPath.size();
		size_t lsi = currPath.rfind('/');

		if ((csz - 1) == lsi)		// ends w. sep, b00000
			lsi = currPath.rfind('/', csz - 2);

		currPath.resize(lsi + ((0==lsi)?1:0));
		refresh();
	}

	bool cdTo(string subDir)
	{
		if (currPath.empty()) {
			printf("Error, currPath is empty on cdTo(\"%s\") \n", subDir.c_str());
			return false;
		}
		bool endsOnSep = ('/' == subDir[subDir.size()-1]);
		string tpath = currPath;
		if (!endsOnSep) tpath += "/";
		tpath+=subDir;

		if (refresh(tpath)) {
			currPath = tpath;
			return true;
		}
		refresh(currPath);	// Make sure we get current data back if we can't open tpath !
		return false;
	}


	bool refresh(string dir = string())
	{
		entries.clear();

		if (dir.empty())
			dir = currPath;

		// getdents() , fill list 

		printf("%s(\"%s\") \n", __FUNCTION__, dir.c_str());

		int dfd = _open(dir.c_str(), O_RDONLY | O_DIRECTORY); //sceKernelOpen(dir.c_str(), SCE_KERNEL_O_RDONLY, SCE_KERNEL_O_DIRECTORY);	// SCE_KERNEL_O_DIRECTORY gets cut off wtf
		if (dfd > 0) {

			dentBuff.resize(0x4000);

			int dsize = 0;
			while (0 < (dsize = sceKernelGetdents(dfd, &dentBuff[0], dentBuff.size())))
			{
				int offs = 0;
				SceKernelDirent *de = (SceKernelDirent *)&dentBuff[offs];

				while (offs < dsize && de) {
					de = (SceKernelDirent *)&dentBuff[offs];

					printf("entry fileNo: %X , name: \"%s\" \n", de->d_fileno, de->d_name);

					entries.push_back(de->d_name);
					offs += de->d_reclen;
				}
			}

			_close(dfd);
			return true;
		}
		return false;
	}





};













