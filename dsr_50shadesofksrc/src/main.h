#include "sync/sync.h"
#include "syncs.h"
struct sync_device *rocket;
const struct sync_track *titleframelimit_rocket;
const struct sync_track *titleframeshape_rocket;
const struct sync_track *titleframesharpness_rocket;
const struct sync_track *titlestrpcnt_rocket;
const struct sync_track *titlestrpintens_rocket;
const struct sync_track *titlestoblack_rocket;
const struct sync_track *titlesnoisemix_rocket;

const struct sync_track *mainsize_rocket;
const struct sync_track *scene_number_rocket;
const struct sync_track *tv_artifacts_rocket;
const struct sync_track *camx_rocket;
const struct sync_track *camy_rocket;
const struct sync_track *camz_rocket;

int row_number;



float titleframelimit_f;
float titleframeshape_f;
float titleframesharpness_f;
float titlestrpcnt_f;
float titlestrpintens_f;
float titlestoblack_f;
float titlesnoisemix_f;
float tv_artifacts;
float mainsize_f;
float mainlogoy_f;


float scene_number;
float camx_f;
float camy_f;
float camz_f;
int rowtimes;

float sc1_mengertweak1;
float sc1_mengertweak2;
float sc1_mengertweak3;
const struct sync_track *sc1_mengertweak1_rocket;
const struct sync_track *sc1_mengertweak2_rocket;
const struct sync_track *sc1_mengertweak3_rocket;

float speed_f;
const struct sync_track *speed_rocket;
float mbamount_f;
const struct sync_track *mbamount_rocket;

float sc3_rotatex;
float sc3_rotatey;
float sc3_rotatez;
const struct sync_track *sc3_rotatex_rocket;
const struct sync_track *sc3_rotatey_rocket;
const struct sync_track *sc3_rotatez_rocket;

float sc3_tweak1;
float sc3_tweak2;
float sc3_tweak3;
float sc3_tweak4;
float sc3_tweak5;
float sc3_mengerscale;
float sc3_lightscale;
float sc3_fogscale;
const struct sync_track *sc3_tweak1_rocket;
const struct sync_track *sc3_tweak2_rocket;
const struct sync_track *sc3_tweak3_rocket;
const struct sync_track *sc3_tweak4_rocket;
const struct sync_track *sc3_tweak5_rocket;
const struct sync_track *sc3_mengerscale_rocket;
const struct sync_track *sc3_lightscale_rocket;
const struct sync_track *sc3_fogscale_rocket;

float page_val;
float left_scroll;
float right_scroll;
const struct sync_track *pageval_rocket;
const struct sync_track *left_scroll_rocket;
const struct sync_track *right_scroll_rocket;



FBOELEM main_fbo; //1280/640, 720 /360
sprite maintex;

#include "raymarch.h"


#include "v5.h"
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

long GetAudioTime() {
	MMTIME mmtime;
	mmtime.wType = TIME_SAMPLES;
	MMRESULT res = waveOutGetPosition(hWaveOut, &mmtime, sizeof mmtime);
	switch (mmtime.wType) {
	case TIME_BYTES:
		return MulDiv(mmtime.u.cb, 1000, SAMPLE_RATE*sizeof(SAMPLE_TYPE) * 2);
	case TIME_MS:
		return mmtime.u.ms;
	case TIME_SAMPLES:
		return MulDiv(mmtime.u.sample, 1000, SAMPLE_RATE);
	}
	return -1;
}


float rocket_get_time()
{
	static long delta = 0;
	static long lastTime = GetAudioTime();
	long currTime = GetAudioTime();
	//static long lastTime = timeGetTime();
	//long currTime = timeGetTime();
	
	long diff = currTime - lastTime;
	delta = diff;
	lastTime = currTime;
	static float sceneTime = 0;
	sceneTime += (float)delta / 1000.f;
	return sceneTime;
}

#ifndef SYNC_PLAYER

long music_start_offset = 0;
bool music_is_playing = false;
static const float music_samples_per_row = 60.0f * (float)SAMPLE_RATE / (float)BPM / (float)2.5;

float music_get_position(HWAVEOUT h)
{
	MMTIME mmtime;
	mmtime.wType = TIME_SAMPLES;
	MMRESULT res = waveOutGetPosition(h, &mmtime, sizeof mmtime);
	return  (float)mmtime.u.sample;
}

float rocket_get_row(HWAVEOUT h)
{
	float pos = music_get_position(h);
	return pos / music_samples_per_row;
}

static void rocket_pause(void *d, int flag)
{
	HWAVEOUT h = *((HWAVEOUT *)d);
	if (flag){
		waveOutPause(h);
		music_is_playing = false;
	}
	else {
		waveOutRestart(h);
		music_is_playing = true;
	}
}

static void rocket_set_row(void *d, int row)
{
	HWAVEOUT h = *((HWAVEOUT *)d);
	music_start_offset = row * (long)music_samples_per_row;

	WaveHDR.lpData = (LPSTR)(float*)(lpSoundBuffer + music_start_offset * 2);
	WaveHDR.dwBufferLength = (MAX_SAMPLES - (DWORD)music_start_offset) * 2;

	//send buffer to waveOut
	waveOutReset(h);
	waveOutPrepareHeader(h, &WaveHDR, sizeof(WaveHDR));
	waveOutWrite(h, &WaveHDR, sizeof(WaveHDR));

	//put on pause
	if (!music_is_playing)
		waveOutPause(h);

}

static int rocket_is_playing(void *d)
{
	return music_is_playing;
}

static struct sync_cb rocket_cb = {
	rocket_pause,
	rocket_set_row,
	rocket_is_playing
};
#endif
void init_rocket()
{
	rocket = sync_create_device("sync");

#ifdef SYNC_PLAYER
	
	titleframelimit_rocket = sync_get_track_mem(rocket,"frameLimit", sync_frameLimit_data, sizeof(sync_frameLimit_data));
	titleframeshape_rocket = sync_get_track_mem(rocket, "frameShape", sync_frameShape_data, sizeof(sync_frameShape_data));
	titleframesharpness_rocket = sync_get_track_mem(rocket, "frameSharpness",sync_frameSharpness_data,sizeof(sync_frameSharpness_data));
	titlestrpcnt_rocket = sync_get_track_mem(rocket, "strp_cnt",sync_strp_cnt_data,sizeof(sync_strp_cnt_data));
	titlestrpintens_rocket = sync_get_track_mem(rocket, "strp_trnsinst",sync_strp_trnsinst_data,sizeof(sync_strp_trnsinst_data));
	titlestoblack_rocket = sync_get_track_mem(rocket, "toblack",sync_toblack_data,sizeof(sync_toblack_data));
	titlesnoisemix_rocket = sync_get_track_mem(rocket, "noisemix",sync_noisemix_data,sizeof(sync_noisemix_data));

	mainsize_rocket = sync_get_track_mem(rocket, "mainysize", sync_mainysize_data, sizeof(sync_mainysize_data));


	scene_number_rocket = sync_get_track_mem(rocket, "scene_number",sync_scene_number_data,sizeof(sync_scene_number_data));
	tv_artifacts_rocket = sync_get_track_mem(rocket, "tv_artifacts",sync_tv_artifacts_data,sizeof(sync_tv_artifacts_data));


	camx_rocket = sync_get_track_mem(rocket, "camx",sync_camx_data,sizeof(sync_camx_data));
	camy_rocket = sync_get_track_mem(rocket, "camy", sync_camy_data, sizeof(sync_camy_data));
	camz_rocket = sync_get_track_mem(rocket, "camz", sync_camz_data, sizeof(sync_camz_data));


	sc1_mengertweak1_rocket = sync_get_track_mem(rocket, "sc1_mengertweak1",sync_sc1_mengertweak1_data,sizeof(sync_sc1_mengertweak1_data));
	sc1_mengertweak2_rocket = sync_get_track_mem(rocket, "sc1_mengertweak2",sync_sc1_mengertweak2_data,sizeof(sync_sc1_mengertweak2_data));
	sc1_mengertweak3_rocket = sync_get_track_mem(rocket, "sc1_mengertweak3",sync_sc1_mengertweak3_data,sizeof(sync_sc1_mengertweak3_data));
	speed_rocket = sync_get_track_mem(rocket, "speed",sync_speed_data,sizeof(sync_speed_data));
	mbamount_rocket = sync_get_track_mem(rocket, "mb_amount", sync_mb_amount_data, sizeof(sync_mb_amount_data));

	sc3_rotatex_rocket = sync_get_track_mem(rocket, "sc3_rotatex", sync_sc3_rotatex_data, sizeof(sync_sc3_rotatex_data));
	sc3_rotatey_rocket = sync_get_track_mem(rocket, "sc3_rotatey", sync_sc3_rotatey_data, sizeof(sync_sc3_rotatey_data));
	sc3_rotatez_rocket = sync_get_track_mem(rocket, "sc3_rotatez", sync_sc3_rotatez_data, sizeof(sync_sc3_rotatez_data));

	sc3_tweak1_rocket = sync_get_track_mem(rocket, "sc3_tweak1", sync_sc3_tweak1_data, sizeof(sync_sc3_tweak1_data));
	sc3_tweak2_rocket = sync_get_track_mem(rocket, "sc3_tweak2", sync_sc3_tweak2_data, sizeof(sync_sc3_tweak2_data));
	sc3_tweak3_rocket = sync_get_track_mem(rocket, "sc3_tweak3", sync_sc3_tweak3_data, sizeof(sync_sc3_tweak3_data));
	sc3_tweak4_rocket = sync_get_track_mem(rocket, "sc3_tweak4", sync_sc3_tweak4_data, sizeof(sync_sc3_tweak4_data));
	sc3_tweak5_rocket = sync_get_track_mem(rocket, "sc3_tweak4", sync_sc3_tweak5_data, sizeof(sync_sc3_tweak5_data));
	sc3_mengerscale_rocket = sync_get_track_mem(rocket, "sc3_mengerscale", sync_sc3_mengerscale_data, sizeof(sync_sc3_mengerscale_data));
	sc3_fogscale_rocket = sync_get_track_mem(rocket, "sc3_fogscale", sync_sc3_fogscale_data, sizeof(sync_sc3_fogscale_data));
	pageval_rocket = sync_get_track_mem(rocket, "page_val",sync_page_val_data,sizeof(sync_page_val_data));
	left_scroll_rocket = sync_get_track_mem(rocket, "left_scroll",sync_left_scroll_data,sizeof(sync_left_scroll_data));
	right_scroll_rocket = sync_get_track_mem(rocket, "right_scroll",sync_right_scroll_data,sizeof(sync_right_scroll_data));

	
	/*titleframelimit_rocket = sync_get_track(rocket, "frameLimit");
	titleframeshape_rocket = sync_get_track(rocket, "frameShape");
	titleframesharpness_rocket = sync_get_track(rocket, "frameSharpness");
	titlestrpcnt_rocket = sync_get_track(rocket, "strp_cnt");
	titlestrpintens_rocket = sync_get_track(rocket, "strp_trnsinst");
	titlestoblack_rocket = sync_get_track(rocket, "toblack");
	titlesnoisemix_rocket = sync_get_track(rocket, "noisemix");
	mainsize_rocket = sync_get_track(rocket, "mainysize");
	scene_number_rocket = sync_get_track(rocket, "scene_number");
	tv_artifacts_rocket = sync_get_track(rocket, "tv_artifacts");

	camx_rocket = sync_get_track(rocket, "camx");
	camy_rocket = sync_get_track(rocket, "camy");
	camz_rocket = sync_get_track(rocket, "camz");


	sc1_mengertweak1_rocket = sync_get_track(rocket, "sc1_mengertweak1");
	sc1_mengertweak2_rocket = sync_get_track(rocket, "sc1_mengertweak2");
	sc1_mengertweak3_rocket = sync_get_track(rocket, "sc1_mengertweak3");
	speed_rocket = sync_get_track(rocket, "speed");
	mbamount_rocket = sync_get_track(rocket, "mb_amount");

	sc3_rotatex_rocket = sync_get_track(rocket, "sc3_rotatex");
	sc3_rotatey_rocket = sync_get_track(rocket, "sc3_rotatey");
	sc3_rotatez_rocket = sync_get_track(rocket, "sc3_rotatez");

	sc3_tweak1_rocket = sync_get_track(rocket, "sc3_tweak1");
	sc3_tweak2_rocket = sync_get_track(rocket, "sc3_tweak2");
	sc3_tweak3_rocket = sync_get_track(rocket, "sc3_tweak3");
	sc3_tweak4_rocket = sync_get_track(rocket, "sc3_tweak4");
	sc3_tweak5_rocket = sync_get_track(rocket, "sc3_tweak5");
	sc3_mengerscale_rocket = sync_get_track(rocket, "sc3_mengerscale");
	sc3_fogscale_rocket = sync_get_track(rocket, "sc3_fogscale");

	pageval_rocket = sync_get_track(rocket, "page_val");
	left_scroll_rocket = sync_get_track(rocket, "left_scroll");
	right_scroll_rocket = sync_get_track(rocket, "right_scroll");*/

#else
	sync_connect(rocket, "localhost", SYNC_DEFAULT_PORT);
	
	titleframelimit_rocket = sync_get_track(rocket, "frameLimit");
	titleframeshape_rocket = sync_get_track(rocket, "frameShape");
	titleframesharpness_rocket = sync_get_track(rocket, "frameSharpness");
	titlestrpcnt_rocket = sync_get_track(rocket, "strp_cnt");
	titlestrpintens_rocket = sync_get_track(rocket, "strp_trnsinst");
	titlestoblack_rocket = sync_get_track(rocket, "toblack");
	titlesnoisemix_rocket = sync_get_track(rocket, "noisemix");
	mainsize_rocket = sync_get_track(rocket, "mainysize");
	scene_number_rocket = sync_get_track(rocket, "scene_number");
	tv_artifacts_rocket = sync_get_track(rocket, "tv_artifacts");

	camx_rocket = sync_get_track(rocket, "camx");
	camy_rocket = sync_get_track(rocket, "camy");
	camz_rocket = sync_get_track(rocket, "camz");


	sc1_mengertweak1_rocket = sync_get_track(rocket, "sc1_mengertweak1");
	sc1_mengertweak2_rocket = sync_get_track(rocket, "sc1_mengertweak2");
	sc1_mengertweak3_rocket = sync_get_track(rocket, "sc1_mengertweak3");
	speed_rocket = sync_get_track(rocket, "speed");
	mbamount_rocket = sync_get_track(rocket, "mb_amount");

	sc3_rotatex_rocket = sync_get_track(rocket, "sc3_rotatex");
	sc3_rotatey_rocket = sync_get_track(rocket, "sc3_rotatey");
	sc3_rotatez_rocket = sync_get_track(rocket, "sc3_rotatez");

	sc3_tweak1_rocket = sync_get_track(rocket, "sc3_tweak1");
	sc3_tweak2_rocket = sync_get_track(rocket, "sc3_tweak2");
	sc3_tweak3_rocket = sync_get_track(rocket, "sc3_tweak3");
	sc3_tweak4_rocket = sync_get_track(rocket, "sc3_tweak4");
	sc3_tweak5_rocket = sync_get_track(rocket, "sc3_tweak5");
	sc3_mengerscale_rocket = sync_get_track(rocket, "sc3_mengerscale");
	sc3_fogscale_rocket = sync_get_track(rocket, "sc3_fogscale");

   pageval_rocket = sync_get_track(rocket, "page_val");
   left_scroll_rocket = sync_get_track(rocket, "left_scroll");
   right_scroll_rocket = sync_get_track(rocket, "right_scroll");
#endif
}

int run_rocket(float synctime)
{
	double row = synctime * 5.0;
	row_number = (int)floor(row);
#ifndef SYNC_PLAYER
	if (sync_update(rocket, row_number, NULL, NULL))
	//if (sync_update(rocket, (int)floor(row), &rocket_cb, (void *)&hWaveOut))
		sync_connect(rocket, "localhost", SYNC_DEFAULT_PORT);
#endif

	titleframelimit_f = float(sync_get_val(titleframelimit_rocket, row));
	titleframeshape_f = float(sync_get_val(titleframeshape_rocket, row));
	titleframesharpness_f = float(sync_get_val(titleframesharpness_rocket, row));
	titlesnoisemix_f = float(sync_get_val(titlesnoisemix_rocket, row));
	titlestoblack_f = float(sync_get_val(titlestoblack_rocket, row));
	titlestrpcnt_f = float(sync_get_val(titlestrpcnt_rocket, row));
	titlestrpintens_f = float(sync_get_val(titlestrpintens_rocket, row));

	mainsize_f = float(sync_get_val(mainsize_rocket, row));
	scene_number = float(sync_get_val(scene_number_rocket, row));
	tv_artifacts = float(sync_get_val(tv_artifacts_rocket, row));
	camx_f = float(sync_get_val(camx_rocket, row));
	camy_f = float(sync_get_val(camy_rocket, row));
	camz_f = float(sync_get_val(camz_rocket, row));



	sc1_mengertweak1 = float(sync_get_val(sc1_mengertweak1_rocket, row));
	sc1_mengertweak2 = float(sync_get_val(sc1_mengertweak2_rocket, row));
	sc1_mengertweak3 = float(sync_get_val(sc1_mengertweak3_rocket, row));

	speed_f = float(sync_get_val(speed_rocket, row));
	mbamount_f = float(sync_get_val(mbamount_rocket, row));

	sc3_rotatex = float(sync_get_val(sc3_rotatex_rocket, row));
	sc3_rotatey = float(sync_get_val(sc3_rotatey_rocket, row));
	sc3_rotatez = float(sync_get_val(sc3_rotatez_rocket, row));


	sc3_tweak1 = float(sync_get_val(sc3_tweak1_rocket, row));
	sc3_tweak2 = float(sync_get_val(sc3_tweak2_rocket, row));
	sc3_tweak3 = float(sync_get_val(sc3_tweak3_rocket, row));
	sc3_tweak4 = float(sync_get_val(sc3_tweak4_rocket, row));
	sc3_tweak5 = float(sync_get_val(sc3_tweak5_rocket, row));
	sc3_mengerscale = float(sync_get_val(sc3_mengerscale_rocket, row));
	sc3_fogscale = float(sync_get_val(sc3_fogscale_rocket, row));

	page_val = float(sync_get_val(pageval_rocket, row));
	right_scroll = float(sync_get_val(right_scroll_rocket, row));
	left_scroll = float(sync_get_val(left_scroll_rocket, row));
	right_scroll = float(sync_get_val(right_scroll_rocket, row));

	return 0;



}


sprite logo;

void init_main(){
	init_raymarch();

	GLuint background_texture;
	int width = 32;
	int height = 32;
	GLubyte image[4 * 32 * 32] = { 0 };
	for (int j = 0; j < height; ++j) {
		for (int i = 0; i < width; ++i) {
			size_t index = j*width + i;
			image[4 * index + 0] = 0xFF * (j / 10 % 2)*(i / 10 % 2); // R
			image[4 * index + 1] = 0xFF * (j / 13 % 2)*(i / 13 % 2); // G
			image[4 * index + 2] = 0xFF * (j / 17 % 2)*(i / 17 % 2); // B
			image[4 * index + 3] = 0xFF; // A
		}
	}

	glGenTextures(1, &background_texture);
	glBindTexture(GL_TEXTURE_2D, background_texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	// set texture content
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
	glBindTexture(GL_TEXTURE_2D, 0);

	maintex.xsize = XRES;
	maintex.ysize = YRES;
	maintex.x = XRES / 2;
	maintex.y = YRES / 2;
	maintex.texture = background_texture;
	maintex.acol = 255;
	maintex.rcol = 255;
	maintex.gcol = 255;
	maintex.bcol = 255;
	init_sprite(&maintex);
	main_fbo = init_fbo(XRES, YRES);

}