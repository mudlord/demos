#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <mmsystem.h> // timeGetTime();
#include <stdlib.h> // malloc(), calloc(), free()
#include <stdint.h>
#include <math.h>
#include "tinyptc_gdi.h"
#include "mod_data.h"
#include "screen.h"
#define STBI_NO_STDIO
#define STBI_ASSERT(x)
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "image.h"
#include "bottomscreen.h"
enum
{
    CIA_TEMPO_MODE    = 0,
    VBLANK_TEMPO_MODE = 1
};

extern "C"
{
	int8_t pt2play_Init(uint32_t outputFreq);
	void pt2play_Close(void);
	void pt2play_PauseSong(int8_t pause);
	void pt2play_PlaySong(uint8_t *moduleData, int8_t tempoMode);
	void pt2play_SetStereoSep(uint8_t percent);
}


static uint32_t *smallScreenBuffer;
static uint32_t ticker_Now;
static uint32_t ticker_Next;
static int32_t ticker_Delay; // Must be signed

void ticker_Sync(void)
{
    ticker_Now = timeGetTime();
    ticker_Delay = ticker_Next - ticker_Now;

    if (ticker_Delay > 0) Sleep(ticker_Delay);

    ticker_Next += 16;
}

char SINTAB[256];


#define RGB555(r,g,b) ((r>>3)<<10) + ((g>>3)<<5) + (b>>3)
#define RGB565(r,g,b) ((r>>3)<<11) + ((g>>2)<<5) + (b>>3)

#define BUILD_ARGB32(a,r,g,b) ( (b) | ( (g)<<8 ) | ( (r) <<16) | ( (a) <<24 ) )
#define BUILD_XRGB32(r,g,b) BUILD_ARGB32(255, r, g, b) 




typedef struct {
	int w;
	int h;
}dim;



uint32_t* rotate90(uint32_t*srcBuf, dim* src,dim *dst)
{
	uint32_t *dstBuf = (uint32_t*)malloc(src->w *src->h*sizeof(uint32_t));
	for (int y = 0, destinationColumn = src->h - 1; y < src->h; ++y, --destinationColumn)
	{
		int offset = y * src->w;
		for (int x = 0; x < src->w; x++)
		dstBuf[(x * src->h) + destinationColumn] = srcBuf[offset + x];
	}
	dst->w = src->h;
	dst->h = src->w;
	// Copy rotated pixels
	memcpy(srcBuf, (uint32_t*)dstBuf, src->w * src->h * sizeof(uint32_t));
	return dstBuf;
}





int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, char *lpCmdLine, int nShowCmd)
{
    ticker_Next = timeGetTime() + 16; // Must be offset by +delayfactor

   // if (!pt2play_Init(44100))
    //    return (0);

    if (!ptcOpen("Intro", SCREEN_W, SCREEN_H))
       return 0;
    smallScreenBuffer = (uint32_t *)(malloc(SCREEN_W * SCREEN_H * sizeof (int32_t)));


	Init_Bottomscreen();
   // pt2play_PlaySong((uint8_t*)modData, CIA_TEMPO_MODE);

	int x, y, n;
	//uint32_t* img_data = (uint32_t*)stbi_load_from_memory(dsr_image2, dsr_image2_len, &x, &y, &n, 4);

	dim src;
	src.w = x;
	src.h = y;
	dim dest;
	dest.w = 0;
	dest.h = 0;
//	uint32_t* img_rotate = rotate90(img_data,&src,&dest);


	//RGBAtoXBGR(img_data, x, y);

	


    for (;;)
    {
		//memset(smallScreenBuffer, 0, SCREEN_W * SCREEN_H * sizeof(int32_t));
		//memcpy(smallScreenBuffer, 0, SCREEN_W * SCREEN_H * sizeof(int32_t));
		Do_Bottomscreen(smallScreenBuffer);
        ptcUpdate(smallScreenBuffer);
        ticker_Sync();
    }

   // pt2play_Close();
    free(smallScreenBuffer);

    return 0;
}
