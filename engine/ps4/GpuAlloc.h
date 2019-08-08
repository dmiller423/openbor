#pragma once


#include "ps4-base.h"

enum GpuAllocType: unat
{
	Onion		= SCE_KERNEL_WB_ONION,
	Garlic		= SCE_KERNEL_WC_GARLIC,
	GarlicWB	= SCE_KERNEL_WB_GARLIC,
	GpuAllocType_Max
};




class GpuAlloc
{
	offs_t phy;
	addr_t base;
	size_t size;		// *FIXME* rename these,  param name clash = major bugz
	size_t align;

	unat prot, flags;
	GpuAllocType type;

	constexpr static size_t minSize  = MB(1);
	constexpr static size_t minAlign = KB(16);
	constexpr static unat defProt = (SCE_KERNEL_PROT_GPU_ALL | SCE_KERNEL_PROT_CPU_WRITE);

	constexpr static size_t defAllocAlign = KB(1);	// This is a ridiculous alignment, useful for debugging though
	// Now sub alloc //

	size_t allocOffs;


public:
	virtual ~GpuAlloc() {}

	GpuAlloc(const GpuAllocType type=Onion)
		: phy(0), base(0), size(0), align(KB(16)), type(type), prot(defProt), flags(0)
		, allocOffs(0)
	{
		printf("Debug, GpuAlloc[%p]::ctor() DEFAULT\n", this);
	}

	GpuAlloc(size_t size, size_t align=KB(16), const GpuAllocType type=Onion)
		: phy(0), base(0), type(type), prot(defProt), flags(0)
		, size(AlignUp(size, minSize))
		, align(AlignUp(align, minAlign))
		, allocOffs(0)
	{
		printf("Debug, GpuAlloc[%p]::ctor(%ld, %ld) \n", this, size, align);

		bool res = init(size, align);
		assert(res); // assert not even running expr internally?? don't use !
	}

	bool init(size_t isize, size_t ialign, unat iprot=0)
	{
		sce_error res = SCE_OK;
		if (isize)  size  = AlignUp(isize, minSize);
		if (ialign) align = AlignUp(ialign, minAlign);
		if (iprot)  prot  = iprot;
		else if (this->type == Onion) prot |= SCE_KERNEL_PROT_CPU_READ;

		if (SCE_OK != (res = sceKernelAllocateDirectMemory(0, SCE_KERNEL_MAIN_DMEM_SIZE, size, align, type, &phy))) {
			printf("Error, sceKernelAllocateDirectMemory() Failed with 0x%08X\n", (u32)res);
			return false;
		}
		if (SCE_OK != (res = sceKernelMapDirectMemory((void**)&base, size, prot, 0, phy, align))) {
			printf("Error, sceKernelMapDirectMemory() Failed with 0x%08X\n", (u32)res);
			return false;
		}

		printf("Debug, GpuAlloc[%p]::Init(%ld, %ld) got phy: %p,  base: %p \n", this, isize, ialign, (void*)phy, (void*)base);

		return (SCE_OK == res);
	}

	bool term()
	{
		bool res = 
			SCE_OK == sceKernelMunmap((void*)base, size) &&
			SCE_OK == sceKernelReleaseDirectMemory(phy, size);

		phy = base = size = 0;
		return res;
	}

	addr_t getBaseAddr() {
		return base;
	}


	// A real shit impl. to test with :

	addr_t alloc(size_t asize, size_t aalign = defAllocAlign)
	{
		if (0 == asize) return 0;

		addr_t aa = AlignUp<addr_t>(base + allocOffs, aalign);

		allocOffs = (aa-base)+asize;

		if(allocOffs >= size) { // was base+size
			printf("Error, GpuAlloc[%p]() heap overflow @@ \n", this);
			return 0;
		}

		printf("Debug, GpuAlloc[%p](%lX,%lX) :: %p, allocOffs: %lX / size: %lX \n\n", this, asize, aalign, (void*)aa, allocOffs, size);

		return aa;
	}

	addr_t operator()(size_t asize, size_t aalign = defAllocAlign)
	{
		return alloc(asize, aalign);
	}

	void free(addr_t faddr)
	{
		// no need atm
	}
};














