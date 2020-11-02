//--------------------------------------------------------------------------//
// iq . 2003/2008 . code for 64 kb intros by RGBA                           //
//--------------------------------------------------------------------------//

#ifndef _MSYS_H_
#define _MSYS_H_

static int fps_cap = 60;
static int window_xr = 1280;
static int window_yr = 720;
static int asset_xr = 1280;
static int asset_yr = 720;




#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <mmsystem.h>
#include <stdint.h>
#include "gb_math.h"
#include "musys_gl.h"


int  msys_init( );
void msys_end( void );

static HWND window_handle;

#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
	int8_t *_ptr;
	uint32_t _cnt;
	int8_t *_base;
	uint32_t _bufsiz;
	int32_t _eof;
} MEM;

long GetTime();
double fsel(double a, double b, double c);
double clamp(double a, double min, double max);
float SmoothStep(float a, float b, float t);
MEM *mopen(const int8_t *src, uint32_t length);
void mclose(MEM *buf);
int32_t mtell(MEM *buf);
size_t mread(void *buffer, size_t size, size_t count, MEM *buf);
size_t mwrite(const void *buffer, size_t size, size_t count, MEM *buf);
void mseek(MEM *buf, int32_t offset, int32_t whence);

#ifdef __cplusplus
}
#endif



#endif

