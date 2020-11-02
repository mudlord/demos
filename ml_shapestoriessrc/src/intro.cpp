#include "sys/msys.h"
#include "intro.h"
#include "scene1.h"
#include "sprite.h"
#include "purplebreak.h"
#include <MMSystem.h>
#include "mmreg.h"
SAMPLE_TYPE	lpSoundBuffer[MAX_SAMPLES*2];  
HWAVEOUT	hWaveOut;

WAVEFORMATEX WaveFMT =
{
#ifdef FLOAT_32BIT	
	WAVE_FORMAT_IEEE_FLOAT,
#else
	WAVE_FORMAT_PCM,
#endif		
	2, // channels
	SAMPLE_RATE, // samples per sec
	SAMPLE_RATE*sizeof(SAMPLE_TYPE)*2, // bytes per sec
	sizeof(SAMPLE_TYPE)*2, // block alignment;
	sizeof(SAMPLE_TYPE)*8, // bits per sample
	0 // extension not needed
};

WAVEHDR WaveHDR = 
{
	(LPSTR)lpSoundBuffer, 
	MAX_SAMPLES*sizeof(SAMPLE_TYPE)*2,			// MAX_SAMPLES*sizeof(float)*2(stereo)
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

sprite screen1;
GLuint screentex_1;
GLuint screentex_2;
sprite screen2;


GLuint init_tex(int width, int height)
{
	GLuint cube_creditstex;
	glGenTextures(1, &cube_creditstex);
	glBindTexture(GL_TEXTURE_2D,  cube_creditstex);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D,0,GL_RGB,0,width,height,GL_RGB,GL_UNSIGNED_BYTE, 0);
	glCopyTexImage2D(GL_TEXTURE_2D,0,GL_RGB,0,0,width,height,0);
	glBindTexture(GL_TEXTURE_2D,0);
	return cube_creditstex;
}


int intro_init( int xr, int yr, int nomusic, IntroProgressDelegate *pd )
{
	srand(NULL);
    // progress report, (from 0 to 200)
    pd->func( pd->obj, 0 );
	init_bg();
	init_sshape();
	init_sshape_postproc();
	init_postproc();
	 pd->func( pd->obj, 100 );
	 _4klang_render(lpSoundBuffer);

	 screentex_1 = init_tex(XRES,YRES);
	 screen1.xsize = XRES;
	 screen1.ysize = YRES;
	 screen1.x = 0;
	 screen1.y = 0;
	 screen1.texture =screentex_1;
	 screen1.acol = 255;
	 screen1.rcol = 255;
	 screen1.gcol = 255;
	 screen1.bcol = 255;
	 init_sprite(&screen1);


#ifndef DEBUG
	rocket = sync_create_device("sync");
	sc1_a= sync_get_track_mem(rocket, "sshape_a",sync_sshape_a_data,sizeof(sync_sshape_a_data));
	sc1_b= sync_get_track_mem(rocket, "sshape_b",sync_sshape_b_data,sizeof(sync_sshape_b_data));
	sc1_m= sync_get_track_mem(rocket, "sshape_m",sync_sshape_m_data,sizeof(sync_sshape_m_data));
	sc1_n1= sync_get_track_mem(rocket, "sshape_n1",sync_sshape_n1_data,sizeof(sync_sshape_n1_data));
	sc1_n2= sync_get_track_mem(rocket, "sshape_n2",sync_sshape_n2_data,sizeof(sync_sshape_n2_data));
	sc1_n3= sync_get_track_mem(rocket, "sshape_n3",sync_sshape_n3_data,sizeof(sync_sshape_n3_data));

	sc1_red= sync_get_track_mem(rocket, "sshape_red",sync_sshape_red_data,sizeof(sync_sshape_red_data));
	sc1_green= sync_get_track_mem(rocket, "sshape_green",sync_sshape_green_data,sizeof(sync_sshape_green_data));
	sc1_blue= sync_get_track_mem(rocket, "sshape_blue",sync_sshape_blue_data,sizeof(sync_sshape_blue_data));


	transition_x = sync_get_track_mem(rocket, "transition_x",sync_transition_x_data,sizeof(sync_transition_x_data));
	transition_y = sync_get_track_mem(rocket, "transition_y",sync_transition_y_data,sizeof(sync_transition_y_data));
	transition_xscale = sync_get_track_mem(rocket, "transition_xscale",sync_transition_xscale_data,sizeof(sync_transition_xscale_data));
	transition_yscale= sync_get_track_mem(rocket, "transition_yscale",sync_transition_yscale_data,sizeof(sync_transition_yscale_data));

#else
	rocket = sync_create_device("sync");
	if (sync_connect(rocket, "localhost", SYNC_DEFAULT_PORT))
	{
		return 0;
	}

	sc1_a= sync_get_track(rocket, "sshape_a");
	sc1_b= sync_get_track(rocket, "sshape_b");
	sc1_m= sync_get_track(rocket, "sshape_m");
	sc1_n1= sync_get_track(rocket, "sshape_n1");
	sc1_n2= sync_get_track(rocket, "sshape_n2");
	sc1_n3= sync_get_track(rocket, "sshape_n3");
	sc1_red = sync_get_track(rocket, "sshape_red");
	sc1_green = sync_get_track(rocket, "sshape_green");
	sc1_blue = sync_get_track(rocket, "sshape_blue");

	transition_x = sync_get_track(rocket, "transition_x");
	transition_y = sync_get_track(rocket, "transition_y");
	transition_xscale = sync_get_track(rocket, "transition_xscale");
	transition_yscale = sync_get_track(rocket, "transition_yscale");

#endif


	
	


    pd->func( pd->obj, 200 );

	waveOutOpen			( &hWaveOut, WAVE_MAPPER, &WaveFMT, NULL, 0, CALLBACK_NULL );
	waveOutPrepareHeader( hWaveOut, &WaveHDR, sizeof(WaveHDR) );
	waveOutWrite		( hWaveOut, &WaveHDR, sizeof(WaveHDR) );	

    return 1;
}

void intro_end()
{

}








int intro_do( void )
{

	waveOutGetPosition(hWaveOut, &MMTime, sizeof(MMTIME));
	//bassdrum
	int curNote = (&_4klang_note_buffer)[((MMTime.u.sample >> 8) << 5) + 2*3+0];
	if (curNote != 0)
	{
		bassdrumhit = 1.0;
		
	}
	else
	{
		bassdrumhit = 0.0;
	}
	//snare
	curNote = (&_4klang_note_buffer)[((MMTime.u.sample >> 8) << 5) + 2*4+0];
	if (curNote != 0)
	{
		snarehit = 1.0;

	}
	else
	{
		snarehit = 0.0;
	}
	//choir
	int curNote2 = (&_4klang_note_buffer)[((MMTime.u.sample >> 8) << 5) + 2*2+0];
	if (curNote2 != 0)
	{
		choir = 1.0;

	}
	else
	{
		choir = 0.0;
	}






	static long lastTime = timeGetTime();
	long currTime = timeGetTime();
	static double delta = 0.0;
	long diff= currTime - lastTime;
	//delta = delta*0.5 + (diff/20.0)*0.5;
	delta = diff;
	lastTime = currTime;
	static float sceneTime = 0;

	sceneTime += (float)delta/1000.f;
	static float texttime = 0;
	texttime += (float)delta/1000.f;

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glClearColor(sshape_red/255, sshape_green/255,sshape_blue/255, 1.0f);



	//cannot use more than 1 fbo at once
	//so must do this first
	glBindFramebuffer(GL_FRAMEBUFFER,sshape_fbo.fbo);
	draw_sshape(sceneTime);
	glBindFramebuffer(GL_FRAMEBUFFER,0);
	//draw bg

	glBindFramebuffer(GL_FRAMEBUFFER,postproc_fbo.fbo);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glClearColor(sshape_red/255, sshape_green/255,sshape_blue/255, 0.0f);
	draw_bg(sceneTime);
	draw_sshape_postproc(sceneTime);
	
	glBindFramebuffer(GL_FRAMEBUFFER,0);

	draw_postproc(sceneTime,postproc_fbo,postproc_shader_program);

	glBindTexture( GL_TEXTURE_2D,  screentex_1);
	glCopyTexSubImage2D(GL_TEXTURE_2D,0,0,0,0,0,XRES,YRES);
	glBindTexture(GL_TEXTURE_2D,0);

	if(rowtimes < 0x10 || rowtimes > 0x6FC)
	{
		glClearColor(0.0, 0.0,0.0, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		
	}
	screen1.x = transitions_x;
	screen1.y = transitions_y;
	screen1.xsize = transitions_xscale;
	screen1.ysize = transitions_yscale;
	draw_sprite(&screen1,XRES,YRES);
	if (sceneTime > 113) return 1;
	return 0;

}