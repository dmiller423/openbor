/*
**	VideoOut.h,	Initial VideoOut impl. - Add GpuAlloc 
*/
#pragma once

#include "ps4-base.h"

#ifdef  USE_GPU_ALLOC
#include "GpuAlloc.h"
#endif

class VideoOut
{
	static VideoOut* videoOut; // singleton style, for ::Get() - Note it doesn't force singleton, don't be stupid

	//typedef int32_t VideoHandle;
//	static VideoOut& videoOut;

	int32_t handle;
	int32_t flipMode, flipRate;
	bool tripleBuffer, tile;

	SceKernelEqueue flipQueue;
	SceVideoOutParamMain param;
	SceVideoOutBufferAttribute attr;

#ifdef USE_GPU_ALLOC
	GpuAlloc fbAlloc;
#else
	off_t phyAddr;
#endif
	addr_t mapAddr;
	size_t memSize;			// Size total of all framebuffer, aligned total
	size_t bufSize;			// Size of one framebuffer (w.o alignment)
	size_t alignTo;

	addr_t addrList[16];	// 16=max buffer slots

	unat currBuffer;

public:

	VideoOut(u32 defWidth=1920, u32 defHeight=1080, bool defTile=false, bool defTripleBuffer=false)
		: handle(0)
		, param({0})
		, flipRate(0) // 0=60fps 1=30fps 2=20? auto
#ifdef USE_VSYNC
		, flipMode(SCE_VIDEO_OUT_FLIP_MODE_VSYNC)	// waitFlip waits on vsync
#else
		, flipMode(SCE_VIDEO_OUT_FLIP_MODE_HSYNC)
#endif
		, tile(defTile)
		, tripleBuffer(defTripleBuffer)
		, alignTo(MB<unat>(2))
		, mapAddr(0)
		, currBuffer(0)
#ifdef USE_GPU_ALLOC
		, fbAlloc(Garlic)
#else
		, phyAddr(0)
#endif
	{
		memset(addrList, 0, sizeof(addr_t) * 16);

		memset(&param, 0, sizeof(param));
		param.paramVersion = SCE_VIDEO_OUT_PARAM_MAIN_VERSION;
		param.setServiceThreadPriority = 0;
		param.setServiceThreadAffinityMask = 0;

		memset(&attr, 0, sizeof(attr));
		attr.width  = defWidth;
		attr.height = defHeight;
		attr.aspectRatio = SCE_VIDEO_OUT_ASPECT_RATIO_16_9;
		attr.pixelFormat = SCE_VIDEO_OUT_PIXEL_FORMAT_A8B8G8R8_SRGB;
		attr.tilingMode  = SCE_VIDEO_OUT_TILING_MODE_LINEAR; // (!tile) ? : 0;		// change to tiled later, GpuMode must be NEO for pro
		attr.pitchInPixel = defWidth;
	}

	bool Init() {
		if (!Open()) {
			printf("Error, Failed to obtain handle!\n");
			return false;
		}
		if (SCE_OK != sceKernelCreateEqueue(&flipQueue, "flipQueue") ||
			SCE_OK != sceVideoOutAddFlipEvent(flipQueue, handle, NULL)) {
			printf("Error, Failed to create/add flip queue\n");
			goto fail;
		}
		if (!AllocFramebuffers()) {
			printf("Error, Failed to AllocFramebuffers!\n");
			goto fail;
		}
		sceVideoOutSetFlipRate(handle, flipRate);

		videoOut = this;
		return true;

	fail:
		Close();
		return false;
	}

	void Term() {
		videoOut = nullptr;
		FreeFramebuffers();
		Close();
	}

	static inline VideoOut *Get() { return videoOut; }


	INLINE int32_t Handle() { return handle; }

	INLINE u32 Width() { return attr.width; }
	INLINE u32 Height() { return attr.height; }

	INLINE size_t MemSize() { return memSize;  }
	INLINE size_t BufferSize() { return bufSize; }
	INLINE size_t BufferCount() { return (tripleBuffer ? 3 : 2); }

	INLINE addr_t GetBuffer(unat n) { return addrList[n & 15]; }
	INLINE addr_t CurrentBuffer() { return GetBuffer(currBuffer); }
	INLINE addr_t NextBuffer() { return GetBuffer(((currBuffer + 1) % BufferCount())); }

	INLINE bool IsFlipPending() {
		return sceVideoOutIsFlipPending(handle) != 0;
	}

	INLINE void WaitOnFlip() {
		int out = 0;
		SceKernelEvent ev;
		while (IsFlipPending()) {
			assert(SCE_OK == sceKernelWaitEqueue(flipQueue, &ev, 1, &out, 0));
		}
	}

	INLINE bool GetFlipStatus(SceVideoOutFlipStatus *status) {
		return SCE_OK == sceVideoOutGetFlipStatus(handle, status);
	}

	// sceVideoOutGetResolutionStatus

	INLINE void WaitOnFlip(u64 arg) {
		int out = 0;
		SceKernelEvent ev;
		SceVideoOutFlipStatus status;

		while (1) {
			GetFlipStatus(&status);
			if (status.flipArg >= arg)
				return;
			assert(SCE_OK == sceKernelWaitEqueue(flipQueue, &ev, 1, &out, 0));
		}
	}

	INLINE void SubmitFlip(s64 buffer = -1, u64 arg = 0)
	{
		sce::Gnm::flushGarlic();
		
		currBuffer = (buffer == -1) ? currBuffer : buffer;
		sceVideoOutSubmitFlip(handle, currBuffer, flipMode, arg);
		//printf("SubmitFlip() Buffer[%d] %p \n", currBuffer, (u8*)CurrentBuffer());

		const static unat bufferCount = BufferCount();
		currBuffer = ((currBuffer + 1) % bufferCount);
	}


	
private:

	bool Open() {
		handle = sceVideoOutOpen(SCE_USER_SERVICE_USER_ID_SYSTEM, SCE_VIDEO_OUT_BUS_TYPE_MAIN, 0, NULL); // &param);
		return (handle > 0);
	}

	void Close() {
		sceVideoOutClose(handle);
		handle = 0;
	}

	

	// 8294400 = 1x 1080p buffer @32bpp w.o padding/align

	bool AllocFramebuffers(u32 width=0, u32 height=0)
	{
		if (width  > 0) attr.width  = width;
		if (height > 0) attr.height = height;
		
		size_t pixelCount = attr.width * attr.height, Bpp = 4; // 32bpp
		size_t alignedSize = AlignUp(pixelCount * Bpp, alignTo);

		bufSize = pixelCount * Bpp;
		memSize = alignedSize * BufferCount();

		int32_t res = SCE_OK;

#ifndef USE_GPU_ALLOC
		if (SCE_OK != (res = sceKernelAllocateDirectMemory(0, SCE_KERNEL_MAIN_DMEM_SIZE, memSize, alignTo, SCE_KERNEL_WC_GARLIC, &phyAddr))) {
			printf("Error, sceKernelAllocateDirectMemory() Failed with 0x%08X\n",(u32)res);
			return false;
		}
		if (SCE_OK != (res = sceKernelMapDirectMemory((void**)&mapAddr, memSize, SCE_KERNEL_PROT_CPU_RW | SCE_KERNEL_PROT_GPU_RW, 0, phyAddr, alignTo))) {
			printf("Error, sceKernelMapDirectMemory() Failed with 0x%08X\n", (u32)res);
			return false;
		}
#else
		if(!fbAlloc.init(memSize, alignTo)) {
			printf("Error, fbAlloc() Failed !\n");
			return false;
		}
		mapAddr = fbAlloc.getBaseAddr();/*
		for (unat i = 0; i < BufferCount(); i++) {
			addrList[i] = fbAlloc(memSize, alignTo);
			printf("Buffer[%lX] %p \n", i, (void*)addrList[i]);
		}*/
#endif
		for (unat i = 0; i < BufferCount(); i++) {
			addrList[i] = mapAddr + (i * alignedSize);	// fbAlloc(memSize, alignTo)
			printf("Buffer[%lX] %p \n", i, (void*)addrList[i]);
		}

		if (SCE_OK != (res = sceVideoOutRegisterBuffers(handle, 0, (void* const*)addrList, BufferCount(), &attr))) {
			printf("Error, sceVideoOutRegisterBuffers() Failed with 0x%08X\n", (u32)res);
			FreeFramebuffers();
			return false;
		}

		return true;

	}

	void FreeFramebuffers()
	{
		for (unat i=0; i<BufferCount(); i++)
			sceVideoOutUnregisterBuffers(handle, i);

#ifndef USE_GPU_ALLOC
		sceKernelMunmap(mapAddr, memSize);
		sceKernelReleaseDirectMemory(phyAddr, memSize);
#else
		fbAlloc.term();
#endif
	}







};


