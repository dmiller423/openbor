/*
**	reicast: ps4.h
*/
#pragma once

#include <sys/dirent.h>
#include <unistd.h>

#include "ps4-base.h"
#include "GpuAlloc.h"
#include "VideoOut.h"

#if 0
extern "C" {

#include "../yabause.h"
#include "../peripheral.h"
#include "../yui.h"
#include "../vdp2.h"

}


#define PS4_IDBASE	(420)
#define PERCORE_PS4 (PS4_IDBASE + 1)
#define SNDCORE_PS4 (PS4_IDBASE + 2)

extern PerInterface_struct		PERPS4;
extern SoundInterface_struct	SNDPS4;

#endif

extern bool running;	// emulation running



#if 0
#define PS4_PKG
#ifdef PS4_PKG			// These are RO ofc, use unjailed /data for W
# define PS4_DIR_CFG		"/data/reicast"	
# define PS4_DIR_DATA		"/data/reicast"
# define MODULE_PATH_PREFIX	"/app0/sce_module"
#else
# define PS4_DIR_CFG		"/mnt/usb0/reicast/"
# define PS4_DIR_DATA		"/mnt/usb0/reicast/data"
# define MODULE_PATH_PREFIX "/data/self/system/common/lib"
#endif




#define RENDER_WIDTH 1920
#define RENDER_HEIGHT 1080

#define PIGLET_MODULE_NAME "libScePigletv2VSH.sprx"
#define SHCOMP_MODULE_NAME "libSceShaccVSH.sprx"
#endif


#ifdef __cplusplus
extern "C" {
#endif

#if 0
u64 readmsr(u32 msr);

u64 readcr0();
void writecr0(u64 val);

u64 readcr3();
void writecr3(u64 val);

u64 readgs();
void writegs(u64 val);
#endif

int syscall(int num, ...);




#define SC_ATTR __attribute__((__always_inline__, __nodebug__))
#define SSC_ATTR static SC_ATTR

int SSC_ATTR _open(const char *path, int flags, int mode=0)
{
	return syscall(5, path, flags, mode);
}

int SSC_ATTR _close(int fd)
{
	return syscall(6, fd);
}


//11     execvc >> PS4 kexec()

int SSC_ATTR kexec(void* kfn, void *uaddr_in)
{
	return syscall(11, kfn, uaddr_in);
}

//33      AUE_ACCESS      STD{ int access(char *path, int flags); }
int SSC_ATTR access(const char *path, int flags)
{
	return syscall(33, path, flags);
}

//58	AUE_READLINK	STD{ ssize_t readlink(char *path, char *buf, size_t count); }
size_t SSC_ATTR readlink(char *path, char *buf, size_t count)
{
	return syscall(58, path, buf, count);
}

// 60      AUE_UMASK       STD     { int umask(int newmask); } 
int SC_ATTR umask(int newmask);
//{
//	return syscall(60, newmask);
//}

//156	AUE_GETDIRENTRIES	COMPAT{ int getdirentries(int fd, char *buf, u_int count, long *basep); }
///// sysent[196] 0xffffffff` 9b3e8f00 "getdirentries"
int SSC_ATTR getdirentries(int fd, char *buf, u_int count, long *basep)
{
	return syscall(196, fd, buf, count);
}

//272	AUE_O_GETDENTS	STD{ int getdents(int fd, char *buf, size_t count); }
int SSC_ATTR getdents(int fd, char *buf, size_t count)
{
	return syscall(272, fd, buf, count);
}




//416	AUE_SIGACTION	STD	{ int sigaction(int sig, const struct sigaction *act, struct sigaction *oact); }
int SSC_ATTR sigaction(int sig, const struct sigaction *act, struct sigaction *oact)
{
	return syscall(416, sig, act, oact);
}


//417	AUE_SIGRETURN	STD	{ int sigreturn(const struct __ucontext *sigcntxp); }
int SSC_ATTR sigreturn(const struct __ucontext *sigcntxp)
{
	return syscall(417, sigcntxp);
}


//429	AUE_SIGWAIT	STD	{ int sigwait(const sigset_t *set, int *sig); }
int SSC_ATTR sigwait(const sigset_t *set, int *sig)
{
	return syscall(417, set, sig);
}










#ifdef __cplusplus
}
#endif
