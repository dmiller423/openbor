/*
**	base.h
*/
#pragma once

#include <cassert>

#include <Gnm.h>
#include <pad.h>
#include <kernel.h>
#include <video_out.h>
#include <audioout.h>
#include <libsysmodule.h>


#define USE_GPU_ALLOC

// base types

typedef uint8_t  u8,  U8;
typedef uint16_t u16, U16;
typedef uint32_t u32, U32;
typedef uint64_t u64, U64;

typedef int8_t  s8,  S8;
typedef int16_t s16, S16;
typedef int32_t s32, S32;
typedef int64_t s64, S64;

typedef float  f32, F32;
typedef double f64, F64;


typedef U64 unat, unat_t;	// 'Natural' sized base types, PS4 is x64 no check req.
typedef S64 snat, snat_t;

/*
#ifdef size_t	// getting errors about size_t being int, W T F
#undef size_t
typedef unat  size_t;
#endif
*/
typedef unat  addr_t;
typedef off_t offs_t;
// size_t is std //



typedef int32_t sce_error;


// constexpr helpers

template<typename T> constexpr T KB(const T n) { return 1024 * n;  }
template<typename T> constexpr T MB(const T n) { return 1024 * KB(n); }
template<typename T> constexpr T GB(const T n) { return 1024 * MB(n); }
template<typename T> constexpr T TB(const T n) { return 1024 * GB(n); }



#define INLINE inline // __forceinline__


// Generic Alignment for non pow2, up is abuseable since it's an integer :)

template<typename T> INLINE T Align(const T addr, const T align, unat up=0)
{
	return (addr / align + up) * align;
}

// Alignment for unsigned integers of any type, align must be pow2!

#define POW2_MASK (align - static_cast<T>(1))

template<typename T> INLINE T AlignUp(const T addr, const T align)
{
	return (addr + POW2_MASK) & ~POW2_MASK;
}

template<typename T> INLINE T AlignDown(const T addr, const T align)
{
	return addr & ~POW2_MASK;
}

//#define alignTo		Align
#define alignUp		AlignUp
#define alignDown	AlignDown

