/*
 * OpenBOR - http://www.LavaLit.com
 * -----------------------------------------------------------------------
 * Licensed under the BSD license, see LICENSE in OpenBOR root for details.
 *
 * Copyright (c) 2004 - 2011 OpenBOR Team
 */

#include <cmath>
#include <unistd.h>
#include <kernel.h>
#include <pad.h>


#include "ps4.h"

#include "ps4port.h"
#include "control.h"
//#include "control/control.h"

// *FIXME* why TF does this give me -Wignored-attributes warnings when it doesn't in the other source file ??
#include "imgui/imgui.h"





#define	PAD_START			1
#define	MAX_PADS			4
#define	PAD_END				(18*MAX_PADS)

static int usejoy;
static int lastkey[MAX_PADS];

static const char *padnames[PAD_END + 1 + 1] = {
	"...",
#define CONTROLNAMES(x) \
	x" Up",             \
	x" Right",          \
	x" Down",           \
	x" Left",           \
	x" X",              \
	x" O",              \
	x" []",             \
	x" /\\",            \
	x" L-Trigger",      \
	x" R-Trigger",      \
	x" Start",          \
	x" Select",         \
	x" Note",           \
	x" Home",           \
	x" Hold",           \
	x" Screen",         \
	x" Volume Up",      \
	x" Volume Down",
	CONTROLNAMES("PSP 1")
	CONTROLNAMES("PSP 2")
	CONTROLNAMES("PSP 3")
	CONTROLNAMES("PSP 4")
	"undefined"
};




SceUserServiceUserId userId;

int32_t pad1_H = 0;
ScePadControllerInformation pad1_info;

ImVec2 mousePos(50, 50), lastPos(50, 50);

///*FIXME* 


void control_init(int joy_enable)
{
	usejoy = joy_enable;

//	PerPortReset();
//	pad1 = PerPadAdd(&PORTDATA1);

	if (pad1_H > 0) {
		printf("pad1_H: %X\n", pad1_H);
		return;
	}

	if (SCE_OK != scePadInit())
		printf("--- Error, scePadInit() failed!\n"); //verify

	pad1_H = scePadOpen(userId, SCE_PAD_PORT_TYPE_STANDARD, 0, NULL);

	if (pad1_H > 0) {
		printf("pad1_H: %X\n", pad1_H);
		return;
	}

	SceUserServiceLoginUserIdList userIdList;
	if (SCE_OK != sceUserServiceGetLoginUserIdList(&userIdList))
		printf("--- failed to get sceUserServiceGetLoginUserIdList()\n");	// *FIXME* die

	for (int i = 0; i < SCE_USER_SERVICE_MAX_LOGIN_USERS; i++) {
		if (userIdList.userId[i] != SCE_USER_SERVICE_USER_ID_INVALID) {
			pad1_H = scePadOpen(userIdList.userId[i], SCE_PAD_PORT_TYPE_STANDARD, 0, NULL);
			if (pad1_H > 0) {
				printf("pad1_H: %X\n", pad1_H);
				return;
			}
		}
	}

}

#define PS4_DPAD_UP         0x00000001
#define PS4_DPAD_RIGHT      0x00000002
#define PS4_DPAD_DOWN       0x00000004
#define PS4_DPAD_LEFT       0x00000008
#define PS4_CROSS           0x00000010
#define PS4_CIRCLE          0x00000020
#define PS4_SQUARE          0x00000040
#define PS4_TRIANGLE        0x00000080
#define PS4_LEFT_TRIGGER    0x00000100
#define PS4_RIGHT_TRIGGER   0x00000200
#define PS4_START           0x00000400
#define PS4_SELECT          0x00000800
#define PS4_NOTE			0x00001000
#define PS4_HOME			0x00002000
#define PS4_HOLD			0x00004000
#define PS4_SCREEN			0x00008000
#define PS4_VOLUP			0x00010000
#define PS4_VOLDOWN			0x00020000


unsigned long getPad(int port)
{
	if (port != 0) return lastkey[port] = 0;

	unsigned long btns = 0;

	ScePadData pad;

	int ret = scePadReadState(pad1_H, &pad);
	if (ret != SCE_OK || !pad.connected) {
		printf("Warning, Controller is not connected or failed to read data! (or pad1 badptr) (Error: %08X, pad1_H: %X) \n", (u32)ret, pad1_H);
		return 0;
	}


#define mn(a,b) (((a)<(b))?(a):(b))
#define mx(a,b) (((a)>(b))?(a):(b))

	s8 lsX = pad.leftStick.x - 128; lsX = (lsX < 0) ? mn(lsX, (s8)-8) : mx(lsX, (s8)7);
	s8 lsY = pad.leftStick.y - 128; lsY = (lsY < 0) ? mn(lsY, (s8)-8) : mx(lsY, (s8)7);

	if (control_getjoyenabled())
	{
		if (lsY >=  48) btns |= PS4_DPAD_DOWN;
		if (lsY <= -48) btns |= PS4_DPAD_UP;
		if (lsX <= -48) btns |= PS4_DPAD_LEFT;
		if (lsX >=  48) btns |= PS4_DPAD_RIGHT;
	}

	if (pad.buttons & SCE_PAD_BUTTON_OPTIONS)  btns |= PS4_START;

	if (pad.buttons & SCE_PAD_BUTTON_UP)	    btns |= PS4_DPAD_UP;
	if (pad.buttons & SCE_PAD_BUTTON_DOWN)     btns |= PS4_DPAD_DOWN;
	if (pad.buttons & SCE_PAD_BUTTON_LEFT)     btns |= PS4_DPAD_LEFT;
	if (pad.buttons & SCE_PAD_BUTTON_RIGHT)    btns |= PS4_DPAD_RIGHT;

	if (pad.buttons & SCE_PAD_BUTTON_L1) btns |= PS4_LEFT_TRIGGER;
	if (pad.buttons & SCE_PAD_BUTTON_R1) btns |= PS4_RIGHT_TRIGGER;

	if (pad.buttons & SCE_PAD_BUTTON_TRIANGLE) btns |= PS4_TRIANGLE;
	if (pad.buttons & SCE_PAD_BUTTON_CIRCLE)   btns |= PS4_CIRCLE;
	if (pad.buttons & SCE_PAD_BUTTON_CROSS)	btns |= PS4_CROSS;
	if (pad.buttons & SCE_PAD_BUTTON_SQUARE)   btns |= PS4_SQUARE;


	if (pad.buttons & SCE_PAD_BUTTON_L2) btns |= PS4_SELECT;
//	if (pad.buttons & SCE_PAD_BUTTON_R2) btns |= PS4_RIGHT_TRIGGER;



	int _control_imgui(const ScePadData& pad);
	_control_imgui(pad);


	//if (btns & PS4_HOME && btns & PS4_START) borExit(-1);
	return lastkey[port] = btns;
}


int _control_imgui(const ScePadData& pad)
{

	ImGuiIO &io = ImGui::GetIO();


#define MAP_BUTTON(NAV_NO, BUTTON_ENUM)     { io.NavInputs[NAV_NO] = (pad.buttons & BUTTON_ENUM) ? 1.0f : 0.0f; }
#define MAP_ANALOG(NAV_NO, VALUE, V0, V1)   { float vn = (float)(VALUE - V0) / (float)(V1 - V0); if (vn > 1.0f) vn = 1.0f; if (vn > 0.0f && io.NavInputs[NAV_NO] < vn) io.NavInputs[NAV_NO] = vn; }


	MAP_BUTTON(ImGuiNavInput_DpadDown, SCE_PAD_BUTTON_DOWN);      // D-Pad Down
	MAP_BUTTON(ImGuiNavInput_DpadUp,   SCE_PAD_BUTTON_UP);        // D-Pad Up
	MAP_BUTTON(ImGuiNavInput_DpadLeft, SCE_PAD_BUTTON_LEFT);      // D-Pad Left
	MAP_BUTTON(ImGuiNavInput_DpadRight,SCE_PAD_BUTTON_RIGHT);     // D-Pad Right

	MAP_BUTTON(ImGuiNavInput_Activate, SCE_PAD_BUTTON_CROSS);
	MAP_BUTTON(ImGuiNavInput_Cancel,   SCE_PAD_BUTTON_CIRCLE);
	MAP_BUTTON(ImGuiNavInput_Menu,     SCE_PAD_BUTTON_TRIANGLE);
	MAP_BUTTON(ImGuiNavInput_Input,    SCE_PAD_BUTTON_SQUARE);

	MAP_BUTTON(ImGuiNavInput_FocusPrev, SCE_PAD_BUTTON_L1);
	MAP_BUTTON(ImGuiNavInput_FocusNext, SCE_PAD_BUTTON_R1);
	MAP_BUTTON(ImGuiNavInput_TweakSlow, SCE_PAD_BUTTON_L2);
	MAP_BUTTON(ImGuiNavInput_TweakFast, SCE_PAD_BUTTON_R2);

	s16 LdZ = pad1_info.stickInfo.deadZoneLeft;
	MAP_ANALOG(ImGuiNavInput_LStickLeft, pad.leftStick.x, -LdZ, -32768);
	MAP_ANALOG(ImGuiNavInput_LStickRight,pad.leftStick.x, +LdZ, +32767);
	MAP_ANALOG(ImGuiNavInput_LStickUp,   pad.leftStick.y, +LdZ, +32767);
	MAP_ANALOG(ImGuiNavInput_LStickDown, pad.leftStick.y, -LdZ, -32767);


#if 0



	// display size for some calcs
	u32 dispW = VideoOut::Get()->Width(),
		dispH = VideoOut::Get()->Height();


	/// Touch Data ///

	float tScale = 1.30f;

	s32 tX = pad.touchData.touch[0].x,
		tY = pad.touchData.touch[0].y;

	f32 pResX = pad1_info.touchPadInfo.resolution.x,
		pResY = pad1_info.touchPadInfo.resolution.y;

	f32 fsX = (-(pResX / 2) + (f32)tX) / pResX * tScale * dispW,
		fsY = (-(pResY / 2) + (f32)tY - 100) / pResY * tScale * dispH;

	if (tX) mousePos.x = (dispW / 2) + fsX;
	if (tY) mousePos.y = (dispH / 2) + fsY;

	//	if (tX || tY)
	//		printf("Touch: %d,%d , scaled: %.2f,%.2f  , Mouse: %.2f,%.2f \n", tX, tY, fsX, fsY, mousePos.x, mousePos.y);

		/// RStick Data ///

	tX = -128 + pad.rightStick.x;
	tY = -128 + pad.rightStick.y;

	float rtScale = 0.3f;
	if (tX < -10 || tX>10)	mousePos.x += (float)tX * rtScale;
	if (tY < -10 || tY>10)	mousePos.y += (float)tY * rtScale;

	/// Clamp
	mousePos.x = (mousePos.x <= 0) ? 1 :
		(mousePos.x >= dispW - 10) ? dispW - 11 : mousePos.x;


	mousePos.y = (mousePos.y <= 0) ? 1 :
		(mousePos.y >= dispH - 10) ? dispH - 11 : mousePos.y;

	// io.MousePos always seems to reset to zero, so we keep our own and set
	if (lastPos != mousePos) {
		lastPos = mousePos
		io.MousePos = mousePos;
	}
	io.MouseDown[0] = (pad.buttons&SCE_PAD_BUTTON_TOUCH_PAD) || (pad.buttons&SCE_PAD_BUTTON_R3);

#endif

	return 0;
}


void control_update(s_playercontrols ** playercontrols, int numplayers)
{
	unsigned long k;
	unsigned long i;
	int player;
	int t;
	s_playercontrols * pcontrols;
	unsigned port[MAX_PADS];

	for (i = 0; i < MAX_PADS; i++) port[i] = getPad(i);

	for (player = 0; player < numplayers; player++)
	{
		pcontrols = playercontrols[player];
		k = 0;
		for (i = 0; i < 32; i++) {
			t = pcontrols->settings[i];
			if (t >= PAD_START && t <= PAD_END)
			{
				int portnum = (t - 1) / 18;
				int shiftby = (t - 1) % 18;
				if (portnum >= 0 && portnum <= 3)
				{
					if ((port[portnum] >> shiftby) & 1) k |= (1 << i);
				}
			}
		}
		pcontrols->kb_break = 0;
		pcontrols->newkeyflags = k & (~pcontrols->keyflags);
		pcontrols->keyflags = k;
	}
}








static int flag_to_index(unsigned long flag)
{
	int index = 0;
	unsigned long bit = 1;
	while (!((bit << index)&flag) && index < 31) ++index;
	return index;
}

void control_exit()
{
	usejoy = 0;
}

int control_usejoy(int enable)
{
	usejoy = enable;
	return 0;
}

int control_getjoyenabled()
{
	return usejoy;
}

int keyboard_getlastkey(void)
{
	int i, ret = 0;
	for (i = 0; i < MAX_PADS; i++)
	{
		ret |= lastkey[i];
		lastkey[i] = 0;
	}
	return ret;
}

void control_setkey(s_playercontrols * pcontrols, unsigned int flag, int key)
{
	if (!pcontrols) return;
	pcontrols->settings[flag_to_index(flag)] = key;
	pcontrols->keyflags = pcontrols->newkeyflags = 0;
}

// Scan input for newly-pressed keys.
// Return value:
// 0  = no key was pressed
// >0 = key code for pressed key
// <0 = error
int control_scankey()
{
	static unsigned ready = 0;
	unsigned i, k = 0;

	for (i = 0; i < MAX_PADS; i++)
	{
		if (lastkey[i])
		{
			k = 1 + i * 18 + flag_to_index(lastkey[i]);
			break;
		}
	}

	if (ready && k)
	{
		ready = 0;
		return k;
	}
	ready = (!k);
	return 0;
}

char * control_getkeyname(unsigned keycode)
{
	if (keycode >= PAD_START && keycode <= PAD_END) return (char*)padnames[keycode];
	return "...";
}

void control_rumble(int port, int ratio, int msec)
{
}
