// ps4/sblaster.cpp

#include <cmath>
#include <cstdint>
#include <kernel.h>
#include <audioout.h>

#include "samplerate.h"

extern "C" {

#include "sblaster.h"
#include "soundmix.h"
#include "globals.h"

};

#define PS4_SAMPLES 1024

#define BUF_SIZE 4096
static int16_t audioBuffers[2][BUF_SIZE];
u32 nbuf = 0;

int audioPort = 0;
static volatile bool playing = false;

ScePthread audioThread;
ScePthreadMutex audioMutex;


//void *(*start_routine)(void *),
static void* SB_thread(void *)
{
	scePthreadMutexInit(&audioMutex, nullptr, "SB_mutex");

	uint32_t zeroBuffer[PS4_SAMPLES] = { 0 };
	void* buffer = zeroBuffer;
	int audioPort = sceAudioOutOpen(SCE_USER_SERVICE_USER_ID_SYSTEM, SCE_AUDIO_OUT_PORT_TYPE_MAIN, 0, PS4_SAMPLES,
		48000, (SCE_AUDIO_OUT_PARAM_FORMAT_S16_STEREO << SCE_AUDIO_OUT_PARAM_FORMAT_SHIFT));

	while (1) {
		scePthreadMutexLock(&audioMutex);

		update_sample((unsigned char*)audioBuffers[nbuf], BUF_SIZE);

		bool should_play = playing;
		scePthreadMutexUnlock(&audioMutex);

		buffer = zeroBuffer;
		if (should_play) {
			buffer = audioBuffers[nbuf];
		}
		sceAudioOutOutput(audioPort, nullptr);
		sceAudioOutOutput(audioPort, buffer);
	}
	sceAudioOutClose(audioPort);
	return 0;
}





extern "C" int SB_playstart(int bits, int samplerate)
{
	static bool once = false;
	playing = true;

	if (!once) {
		once = true;
		auto res = scePthreadCreate(&audioThread, nullptr, SB_thread, nullptr, "SB_thread");
		assert(0 == res);
	}

	return 1;
}

extern "C" void SB_playstop(void)
{
	playing = false;
}

extern "C" void SB_setvolume(char dev, char volume){}

extern "C" void SB_updatevolume(int volume){}




