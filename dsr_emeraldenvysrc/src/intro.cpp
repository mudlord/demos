//--------------------------------------------------------------------------//
// iq / rgba  .  tiny codes  .  2008                                        //
//--------------------------------------------------------------------------//

#define WIN32_LEAN_AND_MEAN
#define WIN32_EXTRA_LEAN
#include <windows.h>
#include <mmsystem.h>
#include <GL/gl.h>
#include <math.h>
#include "config.h"
#include "ext.h"
#include "shaders/fsh_rayt.inl"
#include "shaders/vsh_2d.inl"
#include "fp.h"
#include "sync/sync.h"
#include "sync.h"
#include "emerald.h"
struct sync_device *rocket;
const struct sync_track *fades_rocket;
const struct sync_track *sync_rocket;
const struct sync_track *shader_rocket;
const struct sync_track *snarehit1_rocket;
const struct sync_track *snarehit2_rocket;
const struct sync_track *colordistort_rocket;

const struct sync_track * stripcnt_rocket;
const struct sync_track * stripintense_rocket;
const struct sync_track * striptransintense_rocket;
const struct sync_track * striprgbdistort_rocket;

//=================================================================================================================


static void initShader( int *pid, const char *vs, const char *fs )
{
    pid[0] = oglCreateProgram();                           
	const int vsId = oglCreateShader( GL_VERTEX_SHADER ); 
	const int fsId = oglCreateShader( GL_FRAGMENT_SHADER );
	oglShaderSource( vsId, 1, &vs, 0 );
	oglShaderSource( fsId, 1, &fs, 0 );
    oglCompileShader( vsId );
    oglCompileShader( fsId );
	oglAttachShader( pid[0], fsId );
	oglAttachShader( pid[0], vsId );
	oglLinkProgram( pid[0] );

    #ifdef DEBUG
        int		result;
        char    info[1536];
        oglGetObjectParameteriv( vsId,   GL_OBJECT_COMPILE_STATUS_ARB, &result ); oglGetInfoLog( vsId,   1024, NULL, (char *)info ); if( !result ) DebugBreak();
        oglGetObjectParameteriv( fsId,   GL_OBJECT_COMPILE_STATUS_ARB, &result ); oglGetInfoLog( fsId,   1024, NULL, (char *)info ); if( !result ) DebugBreak();
        oglGetObjectParameteriv( pid[0], GL_OBJECT_LINK_STATUS_ARB,    &result ); oglGetInfoLog( pid[0], 1024, NULL, (char*)info ); if( !result ) DebugBreak();
    #endif
}


#ifndef	WAVE_FORMAT_PCM 
#	define	WAVE_FORMAT_PCM                0x0001 
#endif 
#ifndef	WAVE_FORMAT_IEEE_FLOAT 
#	define	WAVE_FORMAT_IEEE_FLOAT         0x0003 
#endif 
#ifndef	WAVE_FORMAT_EXTENSIBLE 
#	define	WAVE_FORMAT_EXTENSIBLE         0xfffe 
#endif 

SAMPLE_TYPE	lpSoundBuffer[MAX_SAMPLES * 2];
HWAVEOUT	hWaveOut;

#pragma data_seg(".wavefmt")
WAVEFORMATEX WaveFMT =
{
#ifdef FLOAT_32BIT	
	WAVE_FORMAT_IEEE_FLOAT,
#else
	WAVE_FORMAT_PCM,
#endif		
	2, // channels
	SAMPLE_RATE, // samples per sec
	SAMPLE_RATE*sizeof(SAMPLE_TYPE) * 2, // bytes per sec
	sizeof(SAMPLE_TYPE) * 2, // block alignment;
	sizeof(SAMPLE_TYPE) * 8, // bits per sample
	0 // extension not needed
};

#pragma data_seg(".wavehdr")
WAVEHDR WaveHDR =
{
	(LPSTR)lpSoundBuffer,
	MAX_SAMPLES*sizeof(SAMPLE_TYPE) * 2,			// MAX_SAMPLES*sizeof(float)*2(stereo)
	0,
	0,
	0,
	0,
	0,
	0
};

MMTIME MMTime =
{
	TIME_SAMPLES,
	0
};


#pragma code_seg(".initsnd")
void  InitSound()
{
	// thx to xTr1m/blu-flame for providing a smarter and smaller way to create the thread :)
	CreateThread(0, 0, (LPTHREAD_START_ROUTINE)_4klang_render, lpSoundBuffer, 0, 0);
	//_4klang_render(lpSoundBuffer);
	waveOutOpen(&hWaveOut, WAVE_MAPPER, &WaveFMT, NULL, 0, CALLBACK_NULL);
	waveOutPrepareHeader(hWaveOut, &WaveHDR, sizeof(WaveHDR));
	waveOutWrite(hWaveOut, &WaveHDR, sizeof(WaveHDR));
}


//=================================================================================================================
//shader 1 - creditsintro_frag
//shader 2 - pillars_frag
//shader 3 - circular/maze2_frag
//shader 4 - ballmaze1_frag
//shader 5 - ballmaze2_frag
static int   pid[6];
static int   vhs_shader;
static int   distort_shader;
GLuint      vhs_texture;

//=================================================================================================================

int intro_init( void )
{
    if( !EXT_Init() )
        return( 0 );
    initShader( &pid[0], raymarch_vert, creditsintro_frag );
	initShader(&pid[1], raymarch_vert, pillars_frag);
	initShader(&pid[2], raymarch_vert, circular_frag);
	initShader(&pid[3], raymarch_vert, ballmaze1_frag);
	initShader(&pid[4], raymarch_vert, ballmaze2_frag);
	initShader(&pid[5], raymarch_vert, ballmaze3_frag);
	initShader(&vhs_shader, vhs_vert, vhs_frag);
	initShader(&distort_shader,vhs_vert, image_distort);
	Resize(XRES, YRES);
	vhs_texture = init_rendertexture(XRES, YRES);


	rocket = sync_create_device("sync");

#ifndef DEBUG
	rocket = sync_create_device("sync");
	fades_rocket = sync_get_track_mem(rocket, "fade", sync_fade_data, sizeof(sync_fade_data));
	shader_rocket = sync_get_track_mem(rocket, "shadernum", sync_shadernum_data, sizeof(sync_shadernum_data));
	sync_rocket = sync_get_track_mem(rocket, "sync", sync_sync_data, sizeof(sync_sync_data));
	snarehit1_rocket = sync_get_track_mem(rocket, "snarehit1", sync_snarehit1_data, sizeof(sync_snarehit1_data));
	snarehit2_rocket =sync_get_track_mem(rocket, "snarehit2", sync_snarehit2_data, sizeof(sync_snarehit2_data));
	colordistort_rocket=sync_get_track_mem(rocket, "colordistort", sync_colordistort_data, sizeof(sync_colordistort_data));


	stripcnt_rocket = sync_get_track_mem(rocket, "strp_cnt", sync_strp_cnt_data, sizeof(sync_strp_cnt_data));
	stripintense_rocket = sync_get_track_mem(rocket, "strp_intens", sync_strp_intens_data, sizeof(sync_strp_intens_data));
	striptransintense_rocket = sync_get_track_mem(rocket, "strp_trnsinst", sync_strp_trnsinst_data, sizeof(sync_strp_trnsinst_data));
	striprgbdistort_rocket = sync_get_track_mem(rocket, "rgb_distort", sync_rgb_distort_data, sizeof(sync_rgb_distort_data));

#else
	if (sync_connect(rocket, "localhost", SYNC_DEFAULT_PORT))
	{
		return 0;
	}
	fades_rocket = sync_get_track(rocket, "fade");
	sync_rocket = sync_get_track(rocket, "sync");
	shader_rocket = sync_get_track(rocket, "shadernum");
	snarehit1_rocket = sync_get_track(rocket, "snarehit1");
	snarehit2_rocket = sync_get_track(rocket, "snarehit2");
	colordistort_rocket = sync_get_track(rocket, "colordistort");

	stripcnt_rocket = sync_get_track(rocket, "strp_cnt");
	stripintense_rocket = sync_get_track(rocket, "strp_intens");
	striptransintense_rocket = sync_get_track(rocket, "strp_trnsinst");
	striprgbdistort_rocket = sync_get_track(rocket, "rgb_distort");
#endif
	InitSound();


    return 1;
}

//=================================================================================================================
int intro_do( long time )
{
    //--- update parameters -----------------------------------------
	static long lastTime = timeGetTime();
	long currTime = timeGetTime();
	static double delta = 0.0;
	long diff = currTime - lastTime;
	delta = diff;
	lastTime = currTime;
	static float sceneTime = 0;
	sceneTime += (float)delta / 1000.f;


	waveOutGetPosition(hWaveOut, &MMTime, sizeof(MMTIME));




	static float synctime = 0.0;
	double row = synctime * 5.0;
	synctime += (float)delta / 1000.f;

	int rowtimes = (int)floor(row);


	if (rowtimes > 1049) return 1;
#ifndef SYNC_PLAYER
	if (sync_update(rocket, (int)floor(row), NULL, NULL))
		sync_connect(rocket, "localhost", SYNC_DEFAULT_PORT);
#endif
	float fade_f = sync_get_val(fades_rocket, row);
	float sync_f = sync_get_val(sync_rocket, row);
	float shader_f = sync_get_val(shader_rocket, row);

	float stripcnt_f = sync_get_val(stripcnt_rocket, row);
	float stripintense_f = sync_get_val(stripintense_rocket, row);
	float striptransintense_f = sync_get_val(striptransintense_rocket, row);
	float striprgbdistort_f = sync_get_val(striprgbdistort_rocket, row);

	float snarehit1_f = 0;
	float snarehit2_f = 0;
	float colordistort_f =1.0;



	//1 - snare 2 - bassline 3 - flanger bass 4 - lead voice 5 - hi - hat 6 - bass drum

	float aha = (&_4klang_envelope_buffer)[((MMTime.u.sample >> 8) << 5) + 2 * 0 + 0];
	int aha1 = (&_4klang_note_buffer)[((MMTime.u.sample >> 8) << 5) + 2 *0 + 0];
	if (aha1 && aha > 0.55)
	{
		snarehit1_f = 1.0f;
		colordistort_f = 1.0;
	}
	if (!aha1 || aha < 0.55)
	{
		snarehit1_f = 0.0f;
		colordistort_f = 0.0;
	}


	const float fade = fade_f;
	const float sync = sync_f;
    const float t  = sceneTime;
	int fragnum = shader_f;
    //--- render -----------------------------------------
	float res[2] = { (float)XRES, (float)YRES };
	glClear(GL_COLOR_BUFFER_BIT);
    oglUseProgram( pid[fragnum] );
	oglUniform2fv(oglGetUniformLocation(pid[fragnum], "resolution"), 1, res);
	oglUniform1fv(oglGetUniformLocation(pid[fragnum], "time"), 1, &t);
	oglUniform1fv(oglGetUniformLocation(pid[fragnum], "fade"), 1, &fade);
	oglUniform1fv(oglGetUniformLocation(pid[fragnum], "sync"), 1, &sync);
    glRects( -1, -1, 1, 1 );
	oglUseProgram(0);
	//copy tex to image for postproc
	glBindTexture(GL_TEXTURE_2D, vhs_texture);
	glCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 0, 0, XRES, YRES);
	glBindTexture(GL_TEXTURE_2D, 0);

	GLuint shadertexture;

	oglUseProgram(distort_shader);
	oglActiveTextureARB(GL_TEXTURE0_ARB);
	shadertexture = oglGetUniformLocation(distort_shader, "tex");
	oglUniform2fv(oglGetUniformLocation(distort_shader, "resolution"), 1, res);
	oglUniform1fv(oglGetUniformLocation(distort_shader, "time"), 1, &t);
	oglUniform1fv(oglGetUniformLocation(distort_shader, "strp_cnt"), 1, &stripcnt_f);
	oglUniform1fv(oglGetUniformLocation(distort_shader, "strp_intens"), 1, &stripintense_f);
	oglUniform1fv(oglGetUniformLocation(distort_shader, "strp_trnsinst"), 1, &striptransintense_f);
	oglUniform1fv(oglGetUniformLocation(distort_shader, "rgb_distort"), 1, &striprgbdistort_f);
	oglUniform1i(shadertexture, 0);
	draw_fbotexture(vhs_texture, XRES, YRES);
	oglUseProgram(0);


	glBindTexture(GL_TEXTURE_2D, vhs_texture);
	glCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 0, 0, XRES, YRES);
	glBindTexture(GL_TEXTURE_2D, 0);

	//do vhs shader
	oglUseProgram(vhs_shader);
	oglActiveTextureARB(GL_TEXTURE0_ARB);
	shadertexture = oglGetUniformLocation(vhs_shader, "tex");
	oglUniform2fv(oglGetUniformLocation(vhs_shader, "resolution"), 1, res);
	oglUniform1fv(oglGetUniformLocation(vhs_shader, "time"), 1, &t);
	oglUniform1fv(oglGetUniformLocation(vhs_shader, "snarehit"), 1, &snarehit1_f);
	oglUniform1fv(oglGetUniformLocation(vhs_shader, "snarehit2"), 1, &snarehit2_f);
	oglUniform1fv(oglGetUniformLocation(vhs_shader, "colordistort"), 1, &colordistort_f);
	oglUniform1i(shadertexture, 0);
	draw_fbotexture(vhs_texture, XRES, YRES);
	oglUseProgram(0);

	

	return 0;
}