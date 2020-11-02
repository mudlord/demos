#include "sync/sync.h"


namespace Timer
{
	void Start();
	float GetTime();
}

/*

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

*/



/*
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
	
//	titleframelimit_rocket = sync_get_track_mem(rocket, "frameLimit", sync_frameLimit_data, sizeof(sync_frameLimit_data));

#else
	sync_connect(rocket, "localhost", SYNC_DEFAULT_PORT);
	
	//titleframelimit_rocket = sync_get_track(rocket, "frameLimit");
#endif
}*/