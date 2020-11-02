#include "sys/musys.h"
#include "intro.h"
#include <mmsystem.h>
#include "main.h"
#include "panda.h"
sprite_t fontspr;
FBOELEM font_fbo;
int intro_init( int xr, int yr, int nomusic, IntroProgressDelegate *pd )
{
	window_xr = xr;
	window_yr = yr;
	srand(NULL);
    // progress report, (from 0 to 200)
    pd->func( pd->obj, 0 );
	init_main();
	init_fontwriter();

	fontspr.xsize = window_xr;
	fontspr.ysize = window_yr;
	fontspr.x = xr / 2;
	fontspr.y = yr / 2;
	fontspr.texture = 0;
	fontspr.acol = 255;
	fontspr.rcol = 255;
	fontspr.gcol = 255;
	fontspr.bcol = 255;
	init_sprite(&fontspr);
	font_fbo = init_fbo(asset_xr, asset_yr, false);
	pd->func(pd->obj, 100);
    pd->func( pd->obj, 200 );
    return 1;
}

void intro_end()
{

}

/*
#ifdef RELEASE
#define exp(x) pow(2.71828183f, (x))
#endif
#define PI 3.1415927f
struct complex
{
	union
	{
		struct
		{
			float real;
			float imag;
		};
		float value[2];
	};

	complex(float real = 0, float imag = 0) : real(real), imag(imag) {}

	float mag() const;
	float arg() const;
};

complex make_complex(float mag, float arg);
complex operator - (const complex& operand);
complex operator + (const complex& lhs, const complex& rhs);
complex operator - (const complex& lhs, const complex& rhs);
complex operator * (const complex& lhs, const complex& rhs);
complex operator / (const complex& lhs, const complex& rhs);

#ifdef RELEASE
extern "C" void __cdecl _CIpow()
{
	__asm
	{
		fxch	st(1)
			fyl2x
			fld		st(0)
			frndint
			fsubr	st(1), st(0)
			fxch	st(1)
			fchs
			f2xm1
			fld1
			faddp	st(1), st(0)
			fscale
			fstp	st(1)
			ret
	}
}
#endif

float complex::mag() const
{
	return sqrt(real * real + imag * imag);
}

float complex::arg() const
{
	return atan2(imag, real);
}

complex make_complex(float mag, float arg)
{
	return complex(mag * cos(arg), mag * sin(arg));
}

complex operator - (const complex& operand)
{
	return complex(-operand.real, -operand.imag);
}

complex operator + (const complex& lhs, const complex& rhs)
{
	return complex(lhs.real + rhs.real, lhs.imag + rhs.imag);
}

complex operator - (const complex& lhs, const complex& rhs)
{
	return complex(lhs.real - rhs.real, lhs.imag - rhs.imag);
}

complex operator * (const complex& lhs, const complex& rhs)
{
	return complex(
		lhs.real * rhs.real - lhs.imag * rhs.imag,
		lhs.imag * rhs.real + lhs.real * rhs.imag);
}

complex operator / (const complex& lhs, const complex& rhs)
{
	float rhs_mad_sqr = rhs.real * rhs.real + rhs.imag * rhs.imag;
	return complex(
		(lhs.real * rhs.real + lhs.imag * rhs.imag) / rhs_mad_sqr,
		(lhs.imag * rhs.real - lhs.real * rhs.imag) / rhs_mad_sqr);
}


#define STFT_WINDOW_SIZE 1024
static float spectrum[32];
float spectrum_tmp[STFT_WINDOW_SIZE];

int GetCurrentSample()
{
	MMTIME mmtime;
	mmtime.wType = TIME_SAMPLES;
	waveOutGetPosition(hWaveOut, &mmtime, sizeof(MMTIME));
	return mmtime.u.sample;
}


unsigned int bit_reverse(unsigned int x)
{
	x = (x & 0x55555555) << 1 | (x & 0xAAAAAAAA) >> 1;
	x = (x & 0x33333333) << 2 | (x & 0xCCCCCCCC) >> 2;
	x = (x & 0x0F0F0F0F) << 4 | (x & 0xF0F0F0F0) >> 4;
x = (x & 0x00FF00FF) << 8 | (x & 0xFF00FF00) >> 8;
x = (x & 0x0000FFFF) << 16 | (x & 0xFFFF0000) >> 16;
return x;
}

void FFT(complex *in_out, int n)
{
	int log2n = 0; for (int i = 1; i < n; i *= 2) ++log2n;
	for (int i = 0; i < n; ++i)
	{
		int j = bit_reverse(i) >> (32 - log2n);
		if (i < j)
		{
			complex tmp = in_out[i];
			in_out[i] = in_out[j];
			in_out[j] = tmp;
		}
	}

	for (int i = 0; i < log2n; ++i)
	{
		int mh = i << i;
		int m = mh * 2;
		for (int j = 0; j < mh; ++j)
		{
			complex e = make_complex(1, 2 * PI * j / m);
			for (int k = 0; k <= n - m; k += m)
			{
				complex u = in_out[k + j];
				complex v = in_out[k + j + mh] * e;
				in_out[k + j] = u + v;
				in_out[k + j + mh] = u - v;
			}
		}
	}
}


complex signal[STFT_WINDOW_SIZE];

void GetSpectrum(float *out)
{
	int cur = GetCurrentSample();
	for (int i = 0; i != STFT_WINDOW_SIZE; ++i)
	{
		signal[i].real = 0;
		signal[i].imag = 0;
		int j = cur - STFT_WINDOW_SIZE / 2 + i;
		if (0 <= j && j < MAX_SAMPLES)
		{
			signal[i].real = (lpSoundBuffer[2 * j] + lpSoundBuffer[2 * j + 1]) / 2;
		}
	}

	FFT(signal, STFT_WINDOW_SIZE);

	for (int i = 0; i != STFT_WINDOW_SIZE; ++i)
	{
		float mag = signal[i].mag();
		out[i] = mag > 1 ? 20 * log10(mag) : 0;
	}
}


struct text
{
	float start;
	float duration;
	float x, y;
	float xspd;
	const char *msg;
	float page;
	float dir;
};*/


int intro_do(void)
{
	float sceneTime = rocket_get_time();
/*	run_rocket(sceneTime);
	GetSpectrum(spectrum_tmp);
	int bin_size = STFT_WINDOW_SIZE / 32;
	for (int i = 0; i != 32; ++i)
	{
		float value = 0;
		for (int j = 0; j < bin_size; ++j)
		{
			int k = i * bin_size + j;
			value += spectrum_tmp[k];
		}
		value /= bin_size;
		value *= 8.0f;
		spectrum[i] = spectrum[i] * 0.75f + value * 0.25f;
	}*/
	//do shit based on FFT
	
	//MMRESULT res = waveOutGetPosition(hWaveOut, &mmtime, sizeof mmtime);
	//snare
	//float aha = (&_4klang_envelope_buffer)[((mmtime.u.sample >> 8) << 5) + 2 * 1+ 0];
	//bassdrum
	//float aha = (&_4klang_envelope_buffer)[((mmtime.u.sample >> 8) << 5) + 2 * 1 + 0];


	//we blit to fbo so that we can upscale/downscale as needed
	//helps when coding positions for text too
	//might have to do for all 2D elements.
	glBindFramebuffer(GL_FRAMEBUFFER, font_fbo.fbo);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glClearColor(0.0, 0.0, 0.0, 1.0f);
	glViewport(0, 0, asset_xr, asset_yr);
	draw_font(asset_xr / 2,asset_yr / 2,asset_xr,asset_yr, "HELLO HELLO HELLO");
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	//glBindFramebuffer(GL_READ_FRAMEBUFFER,  main_fbo.fbo);
	//glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
	//glBlitFramebuffer(0, 0, asset_xr, asset_yr, 0, 0, window_xr, window_yr, GL_DEPTH_BUFFER_BIT|GL_COLOR_BUFFER_BIT, GL_NEAREST);
	//glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
	//glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
	
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glClearColor(0.0, 0.0, 0.0, 0.0f);
	glViewport(0, 0, window_xr, window_yr);
	draw_raymarch(sceneTime, 0, window_xr, window_yr);
	//font render
	fontspr.texture = font_fbo.texture;
	draw_sprite(&fontspr, window_xr, window_yr,0,0);


	return 0;
}