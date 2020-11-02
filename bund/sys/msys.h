//--------------------------------------------------------------------------//
// iq . 2003/2008 . code for 64 kb intros by RGBA                           //
//--------------------------------------------------------------------------//

#ifndef _MSYS_H_
#define _MSYS_H_

#define XRES  1024
#define YRES  768

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <mmsystem.h>

#include "msys_types.h"
#include "msys_glext.h"
#include "msys_debug.h"

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



typedef struct
{
	unsigned long length;
	unsigned char *t_buf;
	unsigned char *buf;
} BUF;

static BUF *bufopen(unsigned char *bufToCopy, unsigned int bufferSize)
{
	BUF *b = (BUF *)malloc(sizeof (BUF));

	b->t_buf = bufToCopy;
	b->buf = &bufToCopy[0];

	b->length = bufferSize;

	return b;
}
static void bufclose(BUF *_SrcBuf)
{
	free(_SrcBuf->t_buf);
	free(_SrcBuf);
}
static void bufread(void *_DstBuf, size_t _ElementSize, size_t _Count, BUF *_SrcBuf)
{
	memcpy(_DstBuf, _SrcBuf->buf, _ElementSize * _Count);
	_SrcBuf->buf += (_ElementSize * _Count);
}
static void bufseek(BUF *_SrcBuf, long _Offset, int _Origin)
{
	if (_SrcBuf->buf != NULL)
	{
		switch (_Origin)
		{
		case SEEK_SET:
			_SrcBuf->buf = _SrcBuf->t_buf + _Offset;
			break;
		case SEEK_CUR:
			_SrcBuf->buf += _Offset;
			break;
		case SEEK_END:
			case SEEK_END: _SrcBuf->buf = _SrcBuf->t_buf + _SrcBuf->length + _Offset; break;
		default:
			break;
		}
	}
}

#endif

