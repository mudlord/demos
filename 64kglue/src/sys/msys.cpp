//--------------------------------------------------------------------------//
// iq . 2003/2008 . code for 64 kb intros by RGBA                           //
//--------------------------------------------------------------------------//

#include "musys_gl.h"
#include <stdio.h>
#include <stdlib.h>
#include "musys.h"

int rand_range(int min_n, int max_n);
float rand_rangef(float a, float b);

double fsel(double a, double b, double c) {
	return a >= 0 ? b : c;
}
double clamp(double a, double min, double max)
{
	a = fsel(a - min, a, min);
	return fsel(a - max, max, a);
}

float Lerp(float a, float b, float t)
{
	return a + t*(b - a);
}



float SmoothStep(float a, float b, float t)
{
	return a + (pow(t, 2)*(3 - 2 * t))*(b - a);
}

MEM *mopen(const int8_t *src, uint32_t length)
{
	MEM *b;

	if ((src == NULL) || (length <= 0)) return (NULL);

	b = (MEM *)(malloc(sizeof(MEM)));
	if (b == NULL) return (NULL);

	b->_base = (int8_t *)(src);
	b->_ptr = (int8_t *)(src);
	b->_cnt = length;
	b->_bufsiz = length;
	b->_eof = 0;

	return (b);
}

void mclose(MEM *buf)
{
	if (buf != NULL)
	{
		free(buf);
		buf = NULL;
	}
}

int32_t mtell(MEM *buf)
{
	return (buf->_bufsiz - buf->_cnt);
}

size_t mread(void *buffer, size_t size, size_t count, MEM *buf)
{
	size_t wrcnt;
	int32_t pcnt;

	if (buf == NULL) return (0);
	if (buf->_ptr == NULL) return (0);

	wrcnt = size * count;
	if ((size == 0) || buf->_eof) return (0);

	pcnt = ((uint32_t)(buf->_cnt) > wrcnt) ? wrcnt : buf->_cnt;
	memcpy(buffer, buf->_ptr, pcnt);

	buf->_cnt -= pcnt;
	buf->_ptr += pcnt;

	if (buf->_cnt <= 0)
	{
		buf->_ptr = buf->_base + buf->_bufsiz;
		buf->_cnt = 0;
		buf->_eof = 1;
	}

	return (pcnt / size);
}

size_t mwrite(const void *buffer, size_t size, size_t count, MEM *buf)
{
	size_t wrcnt;
	int32_t pcnt;

	if (buf == NULL) return (0);
	if (buf->_ptr == NULL) return (0);

	wrcnt = size * count;
	if ((size == 0) || buf->_eof) return (0);

	pcnt = ((uint32_t)(buf->_cnt) > wrcnt) ? wrcnt : buf->_cnt;
	memcpy(buf->_ptr, buffer, pcnt);

	buf->_cnt -= pcnt;
	buf->_ptr += pcnt;

	if (buf->_cnt <= 0)
	{
		buf->_ptr = buf->_base + buf->_bufsiz;
		buf->_cnt = 0;
		buf->_eof = 1;
	}

	return (pcnt / size);
}

int32_t meof(MEM *buf)
{
	if (buf == NULL) return (1); // XXX: Should return a different value?

	return (buf->_eof);
}

void mseek(MEM *buf, int32_t offset, int32_t whence)
{
	if (buf == NULL) return;

	if (buf->_base)
	{
		switch (whence)
		{
		case SEEK_SET: buf->_ptr = buf->_base + offset;                break;
		case SEEK_CUR: buf->_ptr += offset;                             break;
		case SEEK_END: buf->_ptr = buf->_base + buf->_bufsiz + offset; break;
		default: break;
		}

		buf->_eof = 0;
		if (buf->_ptr >= (buf->_base + buf->_bufsiz))
		{
			buf->_ptr = buf->_base + buf->_bufsiz;
			buf->_eof = 1;
		}

		buf->_cnt = (buf->_base + buf->_bufsiz) - buf->_ptr;
	}
}

int rand_range(int min_n, int max_n)
{
	return rand() % (max_n - min_n + 1) + min_n;
}

float rand_rangef(float a, float b)
{
	return ((b-a)*((float)rand()/RAND_MAX))+a;
}

int msys_init( uint64_t h )
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