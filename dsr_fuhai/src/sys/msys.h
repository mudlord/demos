//--------------------------------------------------------------------------//
// iq . 2003/2008 . code for 64 kb intros by RGBA                           //
//--------------------------------------------------------------------------//

#ifndef _MSYS_H_
#define _MSYS_H_


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
#include "msys_types.h"
#include "msys_glext.h"

int  msys_init( uint64 h );
void msys_end( void );

static HWND window_handle;

int rand_range(int min_n, int max_n);
float rand_rangef(float a, float b);

long GetTime();

 static double fsel( double a, double b, double c ) {
	return a >= 0 ? b : c; 
}
static inline double clamp ( double a, double min, double max ) 
{
	a = fsel( a - min , a, min );
	return fsel( a - max, max, a );
}

static float Lerp(float a, float b, float t)
{
	return a + t*(b - a);
}



static float SmoothStep(float a, float b, float t)
{
	return a + (pow(t,2)*(3-2*t))*(b - a);
}


#endif

