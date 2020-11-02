#include "sys/msys.h"
#include "intro.h"

#define GB_MATH_IMPLEMENTATION
#include "sys/gb_math.h"
#include "sync/usync.h"
#include <mmsystem.h>
#include "main.h"
#include "ft2play_v0.77a.c"
#include "raymarch.h"
#include "postproc.h"
#include "unmo3.h"
#include <windows.h>
#include "tune.h"
#include "sprite.h"
#include "panda.h"

#include "unmo3_dll.h"
#include "MemoryModule.h"
sprite pandalogo;
namespace Timer
{
	LARGE_INTEGER LastPCV = { 0 };
	double currentTime = 0.0;

	double _Time()
	{
		LARGE_INTEGER count, freq;
		if (!LastPCV.QuadPart) {
			QueryPerformanceCounter(&LastPCV);
		}
		QueryPerformanceCounter(&count);
		QueryPerformanceFrequency(&freq);

		currentTime += (double)(count.QuadPart - LastPCV.QuadPart) / (double)(freq.QuadPart);

		LastPCV = count;

		return currentTime * 1000.0f;
	}

	float startTime = 0.0;
	void Start()
	{
		startTime = (float)_Time();
	}
	float GetTime()
	{
		return (float)_Time() - startTime;
	}
}

BYTE *Load_Input_File(char *FileName, DWORD *Size)
{
	BYTE *Memory;
	FILE *Input = fopen(FileName, "rb");

	if (!Input) return(NULL);
	// Get the filesize
	fseek(Input, 0, SEEK_END);
	*Size = ftell(Input);
	fseek(Input, 0, SEEK_SET);

	Memory = (BYTE *)malloc(*Size);
	if (!Memory) return(NULL);
	if (fread(Memory, 1, *Size, Input) != (size_t)*Size) return(NULL);
	if (Input) fclose(Input);
	Input = NULL;
	return(Memory);
}
typedef int (WINAPI * UNMO3_DECODE)(void **data, unsigned *len, unsigned flags); 
// free the data returned by UNMO3_Decode 
typedef void (WINAPI * UNMO3_FREE)(void *data);
HMEMORYMODULE handle;
int intro_init( int xr, int yr, int nomusic, IntroProgressDelegate *pd )
{
	window_xr = xr;
	window_yr = yr;
	srand(NULL);
    // progress report, (from 0 to 200)
    pd->func( pd->obj, 0 );
	
	handle = MemoryLoadLibrary(unmo3_dll, unmo3_dll_len);
	if (handle == NULL)
	{
		return 0;
	}
	UNMO3_DECODE UNMO3_Decode = (UNMO3_DECODE)MemoryGetProcAddress(handle, "UNMO3_Decode");
	UNMO3_FREE UNMO3_Free = (UNMO3_FREE)MemoryGetProcAddress(handle, "UNMO3_Free");
	unsigned int tune_sz = tunexm_len;
	//BYTE* tunexm1 = Load_Input_File("700_clown.mo3", (DWORD*)&tune_sz);
	//BYTE* tunexm1 = Load_Input_File("700_clown.xm", (DWORD*)&tune_sz);
	BYTE *tunexm1 = (BYTE*)malloc(tune_sz);
	memcpy(tunexm1, tunexm, tune_sz);
	void *buf0 = tunexm1;
	unsigned len0 = tune_sz;
	
	int err = UNMO3_Decode(&buf0,&len0, 0);
	if (err)return 0;
	ft2play_Init(44100, 1, 1);
	ft2play_LoadModule((uint8_t*)buf0,len0);
	//ft2play_LoadModule((uint8_t*)tunexm1, tune_sz);
	if (usync_init() < 0)
		return 0;
	UNMO3_Free(buf0);
	free(tunexm1);
#ifndef SYNC_PLAYER
	/* HACK: prefetch tracks - not needed when actually editing */
	usync_get_val(foo);
	usync_update(0.0f);
#endif
	pd->func(pd->obj, 100);
	int panda_w, panda_h, comp;
	GLint tex_topgrad = loadTexMemory(dsr_panda, dsr_panda_len, &panda_w, &panda_h, 0);
	//gradient textures
	memset(&pandalogo, 0, sizeof(sprite));

	pandalogo.xsize = panda_w * 1.5;
	pandalogo.ysize = panda_h * 1.5;
	pandalogo.x = window_xr / 2;
	pandalogo.y = window_yr / 2;
	pandalogo.texture = tex_topgrad;
	pandalogo.acol = 255;
	pandalogo.rcol = 255;
	pandalogo.gcol = 255;
	pandalogo.bcol = 255;
	init_sprite(&pandalogo);

	init_postproc();
	init_raymarch();
    pd->func( pd->obj, 200 );
	ft2play_PlaySong();
    return 1;
}

void intro_end()
{
#ifndef SYNC_PLAYER
	usync_export("sync-data.h");
#endif
	ft2play_FreeSong();
	ft2play_Close();
	MemoryFreeLibrary(handle);
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

int8_t ft2Play_GetNote(int nch)
{
	//if (Stm[nch].noteTriggered) 
	//{ Stm[nch].noteTriggered = false; 
	//return Stm[nch].EffTyp; }
	return Stm[nch].EffTyp;
}

int32_t ft2Play_GetRowOrder()
{
	return MAKELONG(Song.PattPos, Song.SongPos);
}


long GetAudioTime() {
	MMTIME mmtime;
	mmtime.wType = TIME_SAMPLES;
	MMRESULT res = waveOutGetPosition(_hWaveOut, &mmtime, sizeof mmtime);
	switch (mmtime.wType) {
	case TIME_BYTES:
		return MulDiv(mmtime.u.cb, 1000, 44100*sizeof(int16_t) * 2);
	case TIME_MS:
		return mmtime.u.ms;
	case TIME_SAMPLES:
		return MulDiv(mmtime.u.sample, 1000, 44100);
	}
	return -1;
}

int intro_do(void)
{
	float sceneTime = Timer::GetTime() / 1000.0;
	//float sceneTime = GetAudioTime() / 1000.0;
	double row2 = sceneTime * 5.0;
	int row_number = (int)floor(row2);

	int roworder = ft2Play_GetRowOrder();
	int row = LOWORD(roworder);
	int order = HIWORD(roworder);
//	int note = ft2Play_GetNote(3);
	int note = ft2Play_GetNote(16);




	
	frameLimit = usync_get_val(frameLimit);
	frameShape = usync_get_val(frameShape);
	frameSharpness = usync_get_val(frameSharpness);
	strp_cnt = usync_get_val(strp_cnt);
	strp_trnsinst = usync_get_val(strp_trnsinst);
	tv_artifacts = usync_get_val(tv_artifacts);
	toblack = usync_get_val(toblack);
	noisemix = usync_get_val(noisemix);
//	run_rocket(sceneTime);
	usync_update(row2);
	wobble = 0.;
	godrays = 0;
	//if (note == 0xE)

	if (Stm[16].EffTyp == 7 && Stm[16].Eff == 0)
	{

		strp_cnt = 128;
		strp_trnsinst = (float)(rand() % 80) / 100;
		tv_artifacts = rand() % 3;
		toblack = 0.0;
		noisemix = (float)(rand() % 20) / 100;
	}


	if (Stm[16].EffTyp == 7 && Stm[16].Eff == 2)
	{

		strp_cnt = 64;
		strp_trnsinst = (float)(rand() % 80) / 100;
		tv_artifacts = rand() % 3;
		toblack = 0.0;
		noisemix = 0.6;
	}

	if (Stm[16].EffTyp == 7 && Stm[16].Eff == 1)
	{
		wobble = 1.0;
	}
	if (order == 2)
	{
		sceneid = 1;
	}

	if (order == 6)
	{
		godrays = 1;
		sceneid = 2;
	}

	if (order == 7)
	{
		if (row > 55)
		{
			godrays = 0;
		}
		else
		{
			godrays = 1;
		}
	}

	if (order >= 10 && order < 12)
	{
	godrays = 1;
	}

	if (order == 8)
	{
		sceneid = 4;
	}

	if (order == 12)
	{
		sceneid = 3;
	}

	
	if (order == 14 && row > 28)toblack = 1.0;


	


	glBindFramebuffer(GL_FRAMEBUFFER, postproc_fbo.fbo);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glClearColor(0.0, 0.0, 0.0, 1.0f);
	glViewport(0, 0, window_xr, window_yr);
	draw_raymarch(sceneTime, 0, window_xr, window_yr);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glClearColor(0.0, 0.0, 0.0, 0.0f);
	glViewport(0, 0, window_xr, window_yr);
	//font render
	//fontspr.texture = postproc_fbo.texture;
	//draw_sprite(&fontspr, window_xr, window_yr);
	draw_postproc(window_xr, window_yr, sceneTime, postproc_fbo.texture, true);

	if (order == 14 && row > 29)
	{
		draw_sprite(&pandalogo,window_xr, window_yr);
	}
	if (order == 14 && row == 63) return 1;
	return 0;
}