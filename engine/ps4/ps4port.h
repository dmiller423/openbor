//ps4port.h
//
#ifndef PS4_H
#define PS4_H

#include "globals.h"

#ifdef __cplusplus
extern "C" {
#endif

	void openborMain(int argc, char** argv);
	void borExit(int reset);

#ifdef __cplusplus
};
#endif

extern char packfile[MAX_FILENAME_LEN];

#endif	// PS4_H
