#include "sys/msys.h"
#include "intro.h"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp" 

#include <mmsystem.h>
#include "sprite.h"
#include "main.h"
#include "postproc.h"

#include "panda.h"
#include "fontwriter.h"
#include "flashback.h"
sprite pandalogo;
sprite flashback_logo;



int intro_init( int xr, int yr, int nomusic, IntroProgressDelegate *pd )
{
	srand(NULL);
    // progress report, (from 0 to 200)
    pd->func( pd->obj, 0 );

	int panda_w, panda_h, comp;
	tex_topgrad = loadTexMemory(dsr_panda, dsr_panda_len, &panda_w, &panda_h, 0);
	//gradient textures
	memset(&pandalogo, 0, sizeof(sprite));

	pandalogo.xsize = panda_w*2;
	pandalogo.ysize = panda_h*2;
	pandalogo.x = XRES /2;
	pandalogo.y = YRES /2;
	pandalogo.texture = tex_topgrad;
	pandalogo.acol = 255;
	pandalogo.rcol = 255;
	pandalogo.gcol = 255;
	pandalogo.bcol = 255;
	init_sprite(&pandalogo);
	int tex_flashback = loadTexMemory(flashback, flashback_len, &panda_w, &panda_h, 0);
	flashback_logo.xsize = panda_w;
	flashback_logo.ysize = panda_h;
	flashback_logo.x = XRES / 2;
	flashback_logo.y = YRES / 2;
	flashback_logo.texture = tex_flashback;
	flashback_logo.acol = 255;
	flashback_logo.rcol = 255;
	flashback_logo.gcol = 255;
	flashback_logo.bcol = 255;
	init_sprite(&flashback_logo);
	init_postproc();
	init_main();
	init_fonts();
	pd->func(pd->obj, 100);
	init_rocket();
	init_sc2blur();
	pd->func(pd->obj, 150);
	InitSound();
    pd->func( pd->obj, 200 );

    return 1;
}

void intro_end()
{

}




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
};

text demotext[] =
{
	/*
	BEG  DUR  X    Y   XSP  YSP
	*/

	// End scroller
	{ 5, 10, 100, 100, -110, "DESIRE PRESENTED THIS INTRO AT", 1.0, 1.0 },
	{ 10, 10, 100, 170, -110, "~", 1.0, 1.0 },

	{ 5, 10, 100, 240, -110, "CODE AND GRAPHICS BY MUDLORD        ", 1.0, 1.0 },
	{ 5, 10, 100, 270, -110, "MUSIC BY TRIACE                     ", 1.0, 1.0 },
	{ 5, 10, 100, 300, -110, "DESIRE LOGO BY ALIEN               ", 1.0, 1.0 },
	{ 5, 10, 100, 330, -110, "FLASHBACK LOGO BY RELOAD OF DEFAME", 1.0, 1.0 },
	{ 5, 10, 100, 100, -110, "GREETS GO TO...............        ", 2.0, 1.0 },
	{ 5, 10, 100, 150, -110, "DEFAME                             ", 2.0, 1.0 },
	{ 5, 10, 100, 190, -110, "DISASTER AREA                      ", 2.0, 1.0 },
	{ 5, 10, 100, 230, -110, "ONSLAUGHT                          ", 2.0, 1.0 },
	{ 5, 10, 100, 270, -110, "DIGITAL ACCESS                     ", 2.0, 1.0 },
	{ 5, 10, 100, 310, -110, "FUNKENSTORT                         ", 2.0, 1.0 },
	{ 5, 10, 100, 350, -110, "DUCK AND CHICKEN                          ", 2.0, 1.0 },
	{ 5, 10, 100, 390, -110, "ADAY                                  ", 2.0, 1.0 },
	{ 5, 10, 100, 440, -110, "JIMAGE                                  ", 2.0, 1.0 },
	{ 5, 10, 100, 480, -110, "AND ALL OTHERS AT THIS FINE PARTY.", 2.0, 1.0 },
};
const int numtext = sizeof(demotext) / sizeof(text);


void textwriter_run()
{
	for (int i = 0; i < numtext; i++)
	{
		if (page_val == demotext[i].page)
		{
			if (demotext[i].start == 10)
			{
				flashback_logo.x = left_scroll+190;
				flashback_logo.y = demotext[i].y;
				draw_sprite(&flashback_logo, XRES, YRES);
			}
			 
			if (demotext[i].dir == 1.0 && demotext[i].start != 10)
			{
				draw_font(left_scroll, demotext[i].y, (char*)demotext[i].msg);
			}
			if (demotext[i].dir == 2.0)
			{
				draw_font(right_scroll, demotext[i].y, (char*)demotext[i].msg);
			}

		}
	}

}


int intro_do(void)
{
	float sceneTime = rocket_get_time();

	run_rocket(sceneTime);
	sc3_lightscale = 50.0;

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
	}
	//bass drum of 1st-2nd scenes
	if (spectrum[1] > 9.5 && spectrum[1] < 20.0 && (row_number > 0x180 && row_number < 0x262))
	{
		titlestrpcnt_f = 32.0;
		titlestrpintens_f = 0.15;
		tv_artifacts = 2.0;
    }

	if (spectrum[1] > 8.0 && row_number > 0x262)
	{
		sc3_lightscale -= spectrum[1] * 1.4;
		
	}
	
	//MMRESULT res = waveOutGetPosition(hWaveOut, &mmtime, sizeof mmtime);
	//snare
	//float aha = (&_4klang_envelope_buffer)[((mmtime.u.sample >> 8) << 5) + 2 * 1+ 0];
	//bassdrum
	//float aha = (&_4klang_envelope_buffer)[((mmtime.u.sample >> 8) << 5) + 2 * 1 + 0];
		

		

	//hihats
	//if (spectrum[3] > 16.0)
	//{
	//	titlesnoisemix_f = 0.25;
	//}

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	//glClearColor(0.0, 0.0,0.0, 1.0f);


	float camx = 0.0;
	float camz = 0.0;
	float camy = 0.0;
	switch ((int)scene_number)
	{
	case 0:
		glBindFramebuffer(GL_FRAMEBUFFER, postproc_fbo.fbo);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glViewport(0, 0, XRES, YRES);
		draw_sprite(&pandalogo, XRES, YRES);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glViewport(0, 0, XRES, YRES);

		tv.texture = postproc_fbo.texture;
		draw_postproc(XRES, YRES, sceneTime, 1.0);
		
		break;
	case 1:
		camx = -0.25 + 3.2*cos(0.2*sceneTime + 4.0);
		camz = 0.5 + 1 * cos(0.2*sceneTime + 2.0);
		camy = camy_f;
	case 2:
	case 3:
	case 4:
	case 5:
		glBindFramebuffer(GL_FRAMEBUFFER, main_fbo.fbo);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		
		glViewport(0, 0, XRES, YRES);
		draw_raymarch(sceneTime, camx, camy, camz, (int)scene_number - 1);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glViewport(0, 0, XRES, YRES);

		glBindFramebuffer(GL_FRAMEBUFFER, postproc_fbo.fbo);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glClearColor(0.0, 0.0, 0.0, 1.0f);
		glViewport(0, 0, XRES, YRES);
		maintex.texture = main_fbo.texture;
		maintex.ysize = (int)mainsize_f;
		draw_sprite(&maintex, XRES, YRES);

		textwriter_run();

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glViewport(0, 0, XRES, YRES);


		PingPong = (PingPong + 1) % 2;
		glBindFramebuffer(GL_FRAMEBUFFER, blur_fbo[PingPong].fbo);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glClearColor(0.0, 0.0, 0.0, 1.0f);
		glViewport(0, 0, XRES, YRES);
		tv.texture = postproc_fbo.texture;
		draw_postproc(XRES, YRES, sceneTime, 0.0);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glViewport(0, 0, XRES, YRES);

		//if (speed_f > 0.5)
		//{
			draw_sc2blur();
		//}
		//else
		//{
		//	tv.texture = postproc_fbo.texture;
		//	draw_postproc(XRES, YRES, sceneTime, 0.0);
	//	}
		break;
	case 6:
		return 1;
		break;
	}

	
	return 0;

}