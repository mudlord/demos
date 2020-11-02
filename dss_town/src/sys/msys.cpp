//--------------------------------------------------------------------------//
// iq . 2003/2008 . code for 64 kb intros by RGBA                           //
//--------------------------------------------------------------------------//

#include "msys_glext.h"
#include "msys_types.h"
#include <stdio.h>
#include <stdlib.h>

int rand_range(int min_n, int max_n)
{
	return rand() % (max_n - min_n + 1) + min_n;
}

float rand_rangef(float a, float b)
{
	return ((b-a)*((float)rand()/RAND_MAX))+a;
}

int msys_init( uint64 h )
{
    if( !msys_glextInit() ) return 0;

    return 1;
}

void msys_end( void )
{
}

long GetTime()
{
	static bool init = false;
	static bool hires = true;
	static __int64 freq = 1;
	if(!init)
	{
		hires = !QueryPerformanceFrequency((LARGE_INTEGER *)&freq);
		if(!hires)
			freq = 1000;
		init = true;
	}

	__int64 now;

	if(hires)
		QueryPerformanceCounter((LARGE_INTEGER *)&now);
	else
		now = GetTickCount();

	return (long)((double)now);

}