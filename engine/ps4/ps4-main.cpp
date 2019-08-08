/*
*	Heretic/ps4/ps4-main.cpp
*
*/
#include <cmath>
#include <unistd.h>
#include <kernel.h>
#include <stdlib.h>

#include "ps4.h"

#include "imgui/imgui.h"
#include "imgui/imgui_sw.hpp"
#include "gui/fsbrowser.h"
#include "gui/pause.h"


extern "C" {
#include "globals.h"

#include "packfile.h"
#include "video.h"
#include "control.h"
#include "utils.h"
#include "ram.h"
}

VideoOut videoOut(1920, 1080); // , true, true);

Scene * Scene::curr = nullptr;
VideoOut * VideoOut::videoOut = nullptr;


unsigned int sceLibcHeapExtendedAlloc = 1;  /* Switch to dynamic allocation */
size_t       sceLibcHeapSize = SCE_LIBC_HEAP_SIZE_EXTENDED_ALLOC_NO_LIMIT;
size_t       sceLibcHeapInitialSize = (1024 * 1024 * 256);
unsigned int sceLibcHeapDebugFlags = SCE_LIBC_HEAP_DEBUG_SHORTAGE;


bool running=false;

extern "C" void Menu(); // defined in menu.c

char packfile[MAX_FILENAME_LEN] = "/app0/bor.pak";
char paksDir[MAX_FILENAME_LEN];
char savesDir[MAX_FILENAME_LEN];
char logsDir[MAX_FILENAME_LEN];
char screenShotsDir[MAX_FILENAME_LEN];
char rootDir[MAX_FILENAME_LEN]; // note: this one ends with a slash


extern "C" void borExit(int reset)
{
	printf("--- borExit(%d) ---\n", reset);
	while (1);
	//sceKernelExitProcess(reset);
}

int main(int argc, char *argv[])
{

	sceUserServiceInitialize(NULL);

	if (SCE_OK != sceAudioOutInit())
		printf("Error, sceAudioOutInit() failed!\n");


	if (!videoOut.Init()) {
		printf("Error, VideoOut::Init(): Failed! \n");
		for (; 1;) {
			sceKernelSleep(10);
			printf("App Done, looping forever");
		}
	}

	printf("[VideoOut] Initialized, clearing framebuffers...\n");
	memset((void*)videoOut.CurrentBuffer(), 0, videoOut.MemSize());
	for (auto _i = 0; _i < 3; _i++) {
		videoOut.SubmitFlip();
		videoOut.WaitOnFlip();
	}


	packfile_mode(0);
//	sceCtrlSetSamplingMode(SCE_CTRL_MODE_ANALOG);

	strcpy(rootDir, "/data/openbor/");
	strcpy(paksDir, "/data/openbor/paks");
	strcpy(savesDir, "/data/openbor/saves");	// not writable
	strcpy(logsDir, "/data/openbor/logs");
	strcpy(screenShotsDir, "/data/openbor/screenshots");

#if 1
	dirExists(rootDir, 1);
	dirExists(paksDir, 1);
	dirExists(savesDir, 1);
	dirExists(logsDir, 1);
	dirExists(screenShotsDir, 1);
#endif

	

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	//io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;		// Enable Keyboard Controls
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;		// Enable Gamepad Controls
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableSetMousePos;	// Testing
	io.BackendFlags |= ImGuiBackendFlags_HasGamepad;
	io.BackendFlags |= ImGuiBackendFlags_HasSetMousePos;	// ImGuiBackendFlags_HasMouseCursors | 
	io.MouseDrawCursor = true;

	io.DisplaySize = ImVec2((float)videoOut.Width(), (float)videoOut.Height());
	io.DisplayFramebufferScale = ImVec2(1.f, 1.f);

	// Setup Dear ImGui style
	ImGui::StyleColorsDark();
	//ImGui::StyleColorsClassic();


	SoftRend1 softRend1;
	IRenderer *renderer = reinterpret_cast<IRenderer*>(&softRend1);

	if (!renderer->ImGui_Init())
		printf("Error, SoftRend2::ImGui_Init(): Failed! \n");


	Scene::Set(new FSBrowser(paksDir));

	void control_init(int joy_enable);
	control_init(0);

	for (;;)	// This is the UI loop only, game code takes over during play //
	{
		unsigned long getPad(int port);
		getPad(0);
		
		videoOut.WaitOnFlip();
		memset((void*)videoOut.CurrentBuffer(), 0, videoOut.BufferSize());

		renderer->ImGui_NewFrame();

		Scene::Render();
		ImGui::Render();

		videoOut.SubmitFlip();
	}

	Scene::Get()->leave();

	renderer->ImGui_Shutdown();


	videoOut.Term();


	printf("--Finished-SUCCESS\n");
	for (;;) sceKernelUsleep(100);

	return SCE_OK;


//	Menu();
	setSystemRam();
	openborMain(argc, argv);
	borExit(0);
	return 0;
}


bool tryToRun(string filePath)
{
	strcpy(packfile, filePath.c_str());
	openborMain(1, (char**)&"");
	return true;
}



extern "C" void vga_setpalette(unsigned char* pal)
{
	printf("%s() called\n", __FUNCTION__);

}

extern "C" void vga_vwait(void)
{
	// already using vsync via VideoOut
}




s_videomodes curr_mode{};


extern "C" void  video_exit(void) {}
extern "C" void  video_init(void) {}

extern "C" int video_set_mode(s_videomodes mode)
{
	printf("%s()\n", __FUNCTION__);
	curr_mode = mode;
	return 1;
}

extern "C" int   video_copy_screen(s_screen* scr)
{
	printf("%s()\n", __FUNCTION__);
	assert(screen_magic == scr->magic);


#if 1 // _DEBUG
	videoOut.WaitOnFlip();
	memset((void*)videoOut.CurrentBuffer(), 0, videoOut.BufferSize());
#endif




	u32 fbW = videoOut.Width(), fbH = videoOut.Height();
	u8* fbP = (u8*)videoOut.CurrentBuffer();

	u32 xMul = fbW / scr->width,
		yMul = fbH / scr->height;

	xMul = (xMul > 1) ? xMul : 1;
	yMul = (yMul > 1) ? yMul : 1;

	xMul = yMul;

	u32 xOffs = (fbW - xMul * scr->width)  / 2,
		yOffs = (fbH - yMul * scr->height) / 2;



	switch (scr->pixelformat)
	{

	case PIXEL_32:	//printf("--\tpxfmt 32\n");
	{
		for (u32 y = 0; y < scr->height; y++) {
#if 0
			memcpy(fbP + 4 * y*fbW, scr->data + 4 * y*scr->width, 4 * scr->width);
#else

			for (u32 yy = 0; yy < (yMul); yy++)	// want free scanlines?  just remove 1 from yMul or so, or set yy=1
			{
				u32* dstLine = &((u32*)fbP)[fbW * (yOffs + y * yMul + yy)];

				for (u32 x = 0; x < scr->width; x++) {
					for (u32 xx = 0; xx < xMul; xx++)
						dstLine[xOffs + x * xMul + xx] = ((u32*)scr->data)[scr->width * y + x];
				}
			}
#endif
		}
	}
	break;
	case PIXEL_8:	printf("--\tpxfmt 8\n");	assert(0);	break;
	case PIXEL_x8:	printf("--\tpxfmt x8\n");	assert(0);	break;
	case PIXEL_16:	printf("--\tpxfmt 16\n");	assert(0);	break;
	default:		printf("--\tpxfmt wtf\n");	assert(0);	break;
	}

	videoOut.SubmitFlip();

	return 0;
}


extern "C" void  video_clearscreen(void)
{
	printf("%s()\n", __FUNCTION__);

	memset((void*)videoOut.CurrentBuffer(), 0, videoOut.BufferSize());
}

extern "C" void  video_set_color_correction(int, int)
{
	printf("%s()\n", __FUNCTION__);
}








// timer


#define GETTIME_FREQ (1000)

static uint64_t start;
static unsigned lastinterval = 0;
static unsigned newticks = 0;

extern "C" void borTimerInit()
{
	start = sceKernelGetProcessTime();
}

extern "C" void borTimerExit() {}

extern "C" unsigned timer_gettick()
{
	return (sceKernelGetProcessTime() - start) / 1000;
}

extern "C" u64 timer_uticks()
{
	return sceKernelGetProcessTime() - start;
}

extern "C" unsigned get_last_interval()
{
	return lastinterval;
}

extern "C" void set_last_interval(unsigned value)
{
	lastinterval = value;
}

extern "C" void set_ticks(unsigned value)
{
	newticks = value;
}


extern "C" unsigned timer_getinterval(unsigned freq)
{
	unsigned tickspassed, ebx, blocksize, now;
	now = timer_gettick() - newticks;
	ebx = now - lastinterval;
	blocksize = GETTIME_FREQ / freq;
	ebx += GETTIME_FREQ % freq;
	tickspassed = ebx / blocksize;
	ebx -= ebx % blocksize;
	lastinterval += ebx;
	return tickspassed;
}







/// fs

extern "C" int dirExists(char *dname, int create) {
	int fd = sceKernelOpen(dname, (create ? SCE_KERNEL_O_CREAT : 0) | SCE_KERNEL_O_DIRECTORY, 0);
	if (fd) sceKernelClose(fd);
	return (int)(-1 != fd);
}

extern "C" int fileExists(char *fnam) {
	int fd = sceKernelOpen(fnam, SCE_KERNEL_O_RDONLY, 0);
	if (fd) sceKernelClose(fd);
	return (int)(-1 != fd);
}