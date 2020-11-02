#include "bottomscreen.h"
#include "screen.h"
#include <stdlib.h> // malloc(), calloc(), free()
#include <stdint.h>
#include <math.h>
#include "stb_image.h"
//#include "image2.h"
#include "syntaxman.h"

void RGBAtoXBGR(uint32_t* img_data, int x, int y)
{
	uint32_t* image_buffer = &img_data[0];
	uint32_t* end = image_buffer + (x*y*sizeof(uint32_t)) / 4;
	while (image_buffer < end) {
		unsigned int pixel = *image_buffer;
		*image_buffer = (pixel & 0xff00ff00) | ((pixel << 16) & 0x00ff0000) | ((pixel >> 16) & 0xff);
		//*image_buffer = ((pixel & 0xffffff00) >> 8) + ((pixel & 0xff) << 24);
		image_buffer++;
	}
}

uint32_t* img_data;


typedef struct {
	int w;
	int h;
}dim;

#define SCRDIM	SCREEN_W * SCREEN_H
uint32_t * smallScreenBuffer;

char *DispX, *DispY;						//Precalculated distortion tables
unsigned int *PixTab;
///////////////////////////////////////////////////////////////////////////////

//Build the distortions tables
void Build_Tables(void);

void Init_Bottomscreen()
{
	int x, y, n;
	img_data = (uint32_t*)stbi_load_from_memory(syntaxman, syntaxman_len, &x, &y, &n, 4);

	dim src;
	src.w = x;
	src.h = y;
	dim dest;
	dest.w = 0;
	dest.h = 0;
	//	uint32_t* img_rotate = rotate90(img_data,&src,&dest);
	RGBAtoXBGR(img_data, x, y);
	smallScreenBuffer = (uint32_t *)(malloc(SCREEN_W * SCREEN_H * sizeof(int32_t)));
	DispX = (char*)malloc((400 * 2) * (240 * 2));
	DispY = (char*)malloc((400 * 2) * (240 * 2));
	PixTab = (unsigned int *)malloc(sizeof(unsigned int) * 240);

	for (int i = 0; i < 240; i++)
		PixTab[i] = i * 400;
	Build_Tables();
}



///////////////////////////////////////////////////////////////////////////////

//Build the distortions tables
void Build_Tables(void)
{
	int i, j, dst = 0;
	float x, y;

	for (j = 0; j < (240 * 2); j++)
		for (i = 0; i < (400 * 2); i++)
		{
			x = (float)i;
			y = (float)j;

			DispX[dst] = (char)(8 * (2 * (sin(x / 20) + sin(x * y / 2000) +
				sin((x + y) / 100) + sin((y - x) / 70) +
				sin((x + 4 * y) / 70) + 2 *
				sin(hypot(256 - x, (150 - y / 8)) / 40))));

			DispY[dst] = (char)(8 * ((cos(x / 31) + cos(x * y / 1783) +
				2 * cos((x + y) / 137) +
				cos((y - x) / 55) + 2 * cos((x + 8 * y) / 57) +
				sin(hypot(384 - x, (274 - y / 9)) / 51))));

			dst++;
		}
}

static void Distortion(int x1, int y1, int x2, int y2, uint32_t* DBuf, uint32_t* pImg)
{
	int dst = 0;
	int x2_abs = abs(x2);
	int y2_abs = abs(y2);
	int x1_abs = abs(x1);
	int y1_abs = abs(y1);
	int src1 = y1_abs * (400 * 2) + x1_abs;
//	
	int src2 = y2_abs * (400 * 2) + x2_abs;
//	
	int dx, dy, i, j;

	for (j = 0; j < 240; j++)
	{
		for (i = 0; i < 400; i++)
		{
				dy = j + (DispY[src1] >> 3);
				dx = i + (DispX[src2] >> 3);
			

			if ((dy >= 0) && (dy <= 239) && (dx >= 0) && (dx <= 399))
				DBuf[dst] = pImg[PixTab[dy] + dx];
			else
				DBuf[dst] = 0x0;

			dst++;
			src1++;
			src2++;
		}

		src1 += 400;
		src2 += 400;
	}
}

void Do_Distort(uint32_t *src, uint32_t *dest)
{
	int x1, y1, x2, y2;
	long long currentTime = 0;

	currentTime = GetTickCount() >> 3;

	//Change this value for some different effects :)
	x1 = (400 / 2) + (int)(159.0f * cos((double)currentTime / 205));
	x2 = (400 / 2) + (int)(159.0f * sin((double)-currentTime / 197));
	y1 = (240 / 2) + (int)(99.0f * sin((double)currentTime / 231));
	y2 = (240 / 2) + (int)(127.0f * cos((double)-currentTime / 224));

	Distortion(x1, y1, x2, y2, dest, src);
}


void Do_Bottomscreen(uint32_t* dstImage)
{
	memset(smallScreenBuffer, 0, SCREEN_W * SCREEN_H * sizeof(int32_t));
	memset(dstImage,0, SCREEN_W * SCREEN_H * sizeof(int32_t));
	//memcpy(dstImage, img_data, );
	Do_Distort(img_data, dstImage);
//	Do_Water(smallScreenBuffer, smallScreenBuffer);
	//memcpy(dstImage, smallScreenBuffer, SCREEN_W * SCREEN_H * sizeof(int32_t));
	//Do_LensMoving(smallScreenBuffer, dstImage);
	
	/*
	static int cnt = 0;
	cnt++;
	if (cnt >= 10)
	{
		cnt = 0;
		Make_Drop();
	}*/

	//Do_LensMoving(img_data, dstImage);
}