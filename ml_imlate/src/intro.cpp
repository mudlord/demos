#include "sys/msys.h"
#include "intro.h"

#define GB_MATH_IMPLEMENTATION
#include "sys/gb_math.h"
#include "sys/ft2play.h"
#include <mmsystem.h>
#include "main.h"
#include "fontwriter.h"
#include "sprite.h"
#include "msx.h"
#include "twister.h"
FBOELEM render_fbo;
FBOELEM font_fbo;
GLuint scene_vao, scene_texture;
shader_id postproc;
shader_id fontproc;
const char vertex_source_fbo[] =
"#version 430\n"
"out gl_PerVertex{vec4 gl_Position;};"
"out vec2 ftexcoord;"
"void main()"
"{"
"	float x = -1.0 + float((gl_VertexID & 1) << 2);"
"	float y = -1.0 + float((gl_VertexID & 2) << 1);"
"	ftexcoord.x = (x + 1.0)*0.5;"
"	ftexcoord.y = (y + 1.0)*0.5;"
"	gl_Position = vec4(x, -y, 0, 1);"
"}";


	const char  pixelate_source[] =
		"#version 430\n"
		"in vec2 ftexcoord;\n"
		 "layout(location=0)out vec4 out_color;"
		 "layout(location=1)uniform vec4 parameters;"
        "layout(location=2)uniform sampler2D tex;"
		"void main()"
		"{"
		"vec2 position = ( gl_FragCoord.xy / parameters.xy );"
   " float dx = parameters.w*(1./parameters.x);"
    "float dy = parameters.w*(1./parameters.y);"
     "vec2 coord = vec2(dx*floor(position.x/dx),dy*floor(position.y/dy));"
		"vec4 col = texture2D(tex,coord);"
		"out_color=col;"
		"}";

const char *postproc_frag =
"\n#version 430\n"
 "layout(location=0)out vec4 out_color;"
 "layout(location=1)uniform vec4 parameters;"
 "layout(location=2)uniform sampler2D tex;"
 "in vec2 ftexcoord;"
 "float r=parameters.r,v=parameters.g,n=parameters.b;"
 "float t(vec2 v)"
 "{"
   "float f=12.9898,n=78.233,r=43758.5,t=dot(v.rg,vec2(f,n)),l=mod(t,3.14);"
   "return fract(sin(l)*r);"
 "}"
 "float f(vec3 v)"
 "{"
   "float f=12.9898,n=78.233,r=58.5065,t=43758.5,l=dot(v.rgb,vec3(f,n,r)),s=mod(l,3.14);"
   "return fract(sin(s)*t);"
 "}"
 "float m(float v)"
 "{"
   "return fract(sin(mod(dot(v,12.9898),3.14))*43758.5);"
 "}"
 "void main()"
 "{"
   "vec2 v=gl_FragCoord.rg/parameters.rg;"
   "v.g=v.g;"
   "vec2 f=-1.+2.*v,s=v,t=s;"
   "mat3 l=mat3(.299,-.147,.615,.587,-.289,-.515,.114,.436,-.1),g=mat3(1.,1.,1.,0.,-.395,2.032,1.14,-.581,0.);"
   "float o=1.;"
   "o-=m(s.r*.1+s.g*50.+n)*.5;"
   "vec3 b=vec3(0.);"
   "float p=.3,x=-.002;"
   "for(int i=10;i>=0;i-=1)"
     "{"
       "float d=float(i)/10.;"
       "if(d<0.)"
         "d=0.;"
       "float e=d*-.05*p+x,a=d*.1*p+x;"
       "vec3 c=(vec3(1.)-pow(vec3(d),vec3(.2,1.,1.)))*.2;"
       "vec2 u=t+vec2(e,0.),y=t+vec2(a,0.);"
       "b+=l*texture(tex,u).rgb*c;"
       "b+=l*texture(tex,y).rgb*c;"
     "}"
   "b.r=b.r*.2+(l*texture(tex,t).rgb).r*.8;"
   "vec4 i=vec4(0.);"
   "i.rgb=g*b*o;"
   "float d=(1.-f.r*f.r)*(1.-f.g*f.g),c=clamp(pow(d,.35)-.01,0.,1.),a=f.g*r*r/r;"
   "vec3 e=mix(vec3(1.,.7,1.),vec3(.7,1.,.7),floor(mod(a,2.)));"
   "i.rgb*=e;"
   "i.rgb*=c;"
   "out_color=vec4(i.rgb,1.);"
 "}";

void draw(float time, shader_id program, int xres, int yres, GLuint texture, float sync=0.0) {
    glBindProgramPipeline(program.pid);
    glViewport(0, 0, xres, yres);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);
    glProgramUniform1i(program.fsid, 2, 0);
    float fparams[4] = { xres, yres, time, sync };
    glProgramUniform4fv(program.fsid, 1, 1, fparams);
    // bind the vao
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glBindVertexArray(scene_vao);
    // draw
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);
    glBindProgramPipeline(0);
    glDisable(GL_BLEND);
}

int intro_init( int xr, int yr, int nomusic, IntroProgressDelegate *pd )
{
	window_xr = xr;
	window_yr = yr;
	srand(NULL);
    // progress report, (from 0 to 200)
    pd->func( pd->obj, 0 );
	init_main();
	init_fonts();
	init_twister();
	postproc=initShader(vertex_source_fbo,postproc_frag);
	fontproc=initShader(vertex_source_fbo,pixelate_source);
	render_fbo = init_fbo(1280, 720, false);
	font_fbo = init_fbo(1280, 720, false);
	glGenVertexArrays(1, &scene_vao);
	pd->func(pd->obj, 100);
    pd->func( pd->obj, 200 );

	ft2play_Init(44100, 1, 1);
	ft2play_LoadModule(muzak, muzak_len);
	ft2play_PlaySong();
    return 1;
}

void intro_end()
{
 ft2play_Close();
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


int8_t ft2Play_GetNote(int nch)
{
	//if (Stm[nch].noteTriggered) 
	//{ Stm[nch].noteTriggered = false; 
	//return Stm[nch].EffTyp; }
	return Stm[nch].EffTyp;
}

struct text
{
	float start;
	float duration;
	float x;
	float y;
	float xspd;
	const char *msg;
	float page;
};

text demotext[] =
{
	/*
	 BEG  DUR  X    Y   XSP  YSP
	*/	

	// End scroller
	{5, 10,300, 100,   0, "THIS SMALL ENTRY IS FOR",0.0},
	{5, 10,300, 130,   0, "FLASHBACK 2019.",0.0},

	{5, 10,300, 200,   0, "SORRY FOR BEING AWOL......",0.0},

	{5, 10,300, 270,   0, "SO HERE IS AT LEAST SOMETHING.....",0.0},

	{5, 10,300, 100,   0, "ITS BEEN AGES SINCE I DID A INTRO.",1.0},
	{5, 10,300, 130,   0, "SO SORRY I MISSED SO MUCH PARTIES...",1.0},
	{5, 10,250, 160,   0, "I REALLY GOTTA MAKE UP FOR LOST TIME.",1.0},




	{5, 10,300, 100,   0, "GREETS TO...",2.0},
	{5, 10,300, 130,   0, "JAZZCAT.",2.0},
	{5, 10,300, 160,   0, "RELOAD.",2.0},
	{5, 10,300, 190,   0, "VOLTAGE.",2.0},
	{5, 10,300, 220,   0, "SQUEAKYNEB.",2.0},
	{5, 10,300, 250,   0, "JIMAGE.",2.0},
	{5, 10,300, 280,   0, "ADAY.",2.0},
	{5, 10,300, 310,   0, "TACTICALMAID.",2.0},
	{5, 10,300, 340,   0, "TRINARYLOGIC.",2.0},
	{5, 10,300, 370,   0, "RIPT.",2.0},
	{5, 10,300, 400,   0, "CHICKEN.",2.0},
	{5, 10,300, 430,   0, "KRION.",2.0},
	{5, 10,300, 460,   0, "GAIASWORD.",2.0},
	{5, 10,300, 490,   0, "CTRIX.",2.0},
	{5, 10,300, 520,   0, "ILLKE.",2.0},

	{5, 10,300, 570,   0, "ALL I FORGOT.",2.0},

	{5, 10,300, 610,   0, "SPECIAL GREETZ TO DESIRE DEMOGROUP.",2.0},
	{5, 10,300, 640,   0, "SORRY I LOST TOUCH.",2.0},

	{5, 10,300, 240,   0, "NJOY FLASHBACK 2019 EVERYONE....",3.0},

	{5, 10,300, 100,   0, "DEMOTEXT LOOPS HERE...",4.0},
};
const int numtext = sizeof(demotext)/sizeof(text);
int num_pages = 5;

void draw_font(int positionx, int positiony, char* buffer);

void font_draw(float delta, float scenetime)
{
	static float texttime = 0;
	static float page_timer=0;
	static int page = 0;
	if(page_timer>9)
	{
		page_timer=0.;
		page = (page + 1) % num_pages;
	}
	page_timer += (float)delta/1000.f;

	for(int i = 0; i < numtext; i++)
	{
		if (page == demotext[i].page)
		{
			draw_font(demotext[i].x,demotext[i].y, (char*)demotext[i].msg);
		}
    }
}

int intro_do(void)
{
	static long lastTime = timeGetTime();
	long currTime = timeGetTime();
	static double delta = 0.0;
	long diff= currTime - lastTime;
	delta = diff;
	lastTime = currTime;
	static float sceneTime = 0;
sceneTime += (float)delta/1000.f;


 glBindFramebuffer(GL_FRAMEBUFFER, font_fbo.fbo);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glClearColor(0.0, 0.0, 0.0, 0.0f);
       glViewport(0, 0, window_xr, window_yr);
	   font_draw(delta,sceneTime);
	    glBindFramebuffer(GL_FRAMEBUFFER, 0);


 glBindFramebuffer(GL_FRAMEBUFFER, render_fbo.fbo);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glClearColor(0.0, 0.0, 0.0, 0.0f);
       glViewport(0, 0, window_xr, window_yr);


	if (Stm[4].EffTyp == 7 && Stm[4].Eff == 2)
	draw_raymarch(sceneTime, 0, window_xr, window_yr,1);
	else
	draw_raymarch(sceneTime, 0, window_xr, window_yr,0);

	static float page_timer=0;
	static int page = 35;
	if(page_timer>9)
	{
		page_timer=0.;
		page = 35;
	}
	page_timer += (float)delta/1000.f;
	
	 page--;
	 if(page<1)page=1;
	  

	  draw(sceneTime, fontproc, 1280, 720, font_fbo.texture,page);
	  
	 
	  

	if (Stm[4].EffTyp == 7 && Stm[4].Eff == 1)
	draw_twister(delta*10);
	else
	draw_twister(delta);


        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        draw(sceneTime, postproc, 1280, 720, render_fbo.texture);
	


	return 0;
}