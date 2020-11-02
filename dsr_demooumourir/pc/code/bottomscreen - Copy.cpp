#include "bottomscreen.h"
#include "screen.h"
#include <stdlib.h> // malloc(), calloc(), free()
#include <stdint.h>
#include <math.h>
#include "stb_image.h"
#include "image2.h"
#ui

TLens::TLens()
{
	//Create the lens and precalculate the distorsion

	int r, d, y, x;
	float shift;
	int ix, iy, offset;

	r = LENSW / 2;
	d = LENSZ;

	for (y = 0; y < LENSW >> 1; y++)
		for (x = 0; x < LENSW >> 1; x++)
		{
			if ((x * x + y * y) < (r * r))
			{
				shift = d / sqrt(d*d - (x*x + y*y - r*r));

				ix = (int)(x * shift - x);
				iy = (int)(y * shift - y);
			}
			else
			{
				ix = 0;
				iy = 0;
			}
			offset = (iy * SCREEN_W + ix);
			Lens[LENSW / 2 - y][LENSW / 2 - x] = -offset;
			Lens[LENSW / 2 + y][LENSW / 2 + x] = offset;
			offset = (-iy * SCREEN_W + ix);
			Lens[LENSW / 2 + y][LENSW / 2 - x] = -offset;
			Lens[LENSW / 2 - y][LENSW / 2 + x] = offset;
		}
}

void TLens::Apply_Lens(uint32_t *Dest, uint32_t *Src, int Ox, int Oy)
{
	//Apply lens deformation on image and save it in Dest

	int temp, pos;
	register int x, y;

	for (y = 0; y < LENSW; y++)
	{
		temp = (y + Oy) * SCREEN_W + Ox;

		for (x = 0; x < LENSW; x++)
		{
			if (Lens[y][x] == 0)
				continue;

			pos = temp + x;

			Dest[pos] = Src[pos + Lens[y][x]];
		}
	}
}
TLens L1, L2;						//2 lens...

void Do_LensMoving(uint32_t *srcImage, uint32_t* dstImage)
{
	//Move lens on screen and apply the effects
	static int x = 16, y = 16;
	static int x2 = 110, y2 = 80;
	static int xd = 4, yd = 4;
	static int xd2 = 4, yd2 = -4;


	L1.Apply_Lens(dstImage, srcImage, x, y);
	L2.Apply_Lens(dstImage, srcImage, x2, y2);

	x += xd;
	y += yd;

	x2 += xd2;
	y2 += yd2;

	if (x > (SCREEN_W - LENSW - 3) || x < 3)
		xd = -xd;

	if (x2 > (SCREEN_W - LENSW - 3) || x2 < 3)
		xd2 = -xd2;

	if (y > (SCREEN_H - LENSW - 3) || y < 3)
		yd = -yd;

	if (y2 > (SCREEN_H - LENSW - 3) || y2 < 3)
		yd2 = -yd2;

}

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

const float damp = .975f;	//Must be < 1!!!!
const float ref = 18.0f;	//Reflections
const float depth = 300.0f;   //Water depth
float water[SCRDIM * 2];							//Precalculated Water buffer

static uint32_t rgb888(int r, int g, int b) { return (0x00u << 24) | ((r & 0xFF) << 16) | ((g & 0xFF) << 8) | (b & 0xFF); }
uint32_t * smallScreenBuffer;

char *DispX, *DispY;						//Precalculated distortion tables
unsigned int *PixTab;
///////////////////////////////////////////////////////////////////////////////

//Build the distortions tables
void Build_Tables(void);

void Init_Bottomscreen()
{
	int x, y, n;
	img_data = (uint32_t*)stbi_load_from_memory(dsr_image2, dsr_image2_len, &x, &y, &n, 4);

	dim src;
	src.w = x;
	src.h = y;
	dim dest;
	dest.w = 0;
	dest.h = 0;
	//	uint32_t* img_rotate = rotate90(img_data,&src,&dest);
	RGBAtoXBGR(img_data, x, y);
	smallScreenBuffer = (uint32_t *)(malloc(SCREEN_W * SCREEN_H * sizeof(int32_t)));
	DispX = (char*)malloc((320 * 2) * (240 * 2));
	DispY = (char*)malloc((320 * 2) * (240 * 2));
	PixTab = (unsigned int *)malloc(sizeof(unsigned int) * 240);

	for (int i = 0; i < 240; i++)
		PixTab[i] = i * 320;
	Build_Tables();
}




void Make_Drop(void)
{
	int ix = rand() % SCREEN_W;
	int iy = rand() % SCREEN_H;

	if (ix > 0 && ix < SCREEN_W - 1 && iy > 1 && iy < SCREEN_H - 1)
		water[iy * SCREEN_W + ix] -= depth;
}

#define scrw SCREEN_W // Screen width in pixels.
#define scrh SCREEN_H // Screen height in pixels.
#define scrs scrw * scrh

//Moving water
void Do_Water(uint32_t* srcImage,uint32_t* dstImage)
{
	int bi;
	// Water physics & 1st buffer copy pass
	for (int y = 1; y < scrh - 1; y++) {
		int yi = y * scrw;
		for (int x = 1; x < scrw - 1; x++) {
			bi = yi + x;
			water[bi + scrs] = ((water[bi - 1] + water[bi + 1] + water[bi - scrw] + water[bi + scrw]) * .5f - water[bi + scrs]) * damp;
		}
	}
	// Refraction & render pass
	for (int y = 1; y < scrh - 1; y++) {
		int yi = y * scrw;
		for (int x = 1; x < scrw - 1; x++) {
			bi = yi + x;
			float nx = water[bi + 1 + scrs] - water[bi - 1 + scrs];
			float ny = water[bi + scrw + scrs] - water[bi - scrw + scrs];
			int rx = x - (int)(nx * ref);
			int ry = y - (int)(ny * ref);
			if (rx < 1) rx = 1;
			if (ry < 1) ry = 1;
			if (rx > scrw - 2) rx = scrw - 2;
			if (ry > scrh - 2) ry = scrh - 2;
			// Specular highlights ;)
			int s = (int)(ny * 64.0f);
			if (s < 0) s = 0;
			if (s > 64) s = 64;
			int argb = srcImage[ry * scrw + rx];
			int r = ((argb >> 16) & 255) + s;
			int g = ((argb >> 8) & 255) + s;
			int b = (argb & 255) + s;
			if (r > 255) r = 255;
			if (g > 255) g = 255;
			if (b > 255) b = 255;
			dstImage[bi] = (r << 16) | (g << 8) | b;//buffer[bi] = bgimage[ry * scrw + rx];
		}
	}
	// 2nd buffer copy pass
	for (int y = 1; y < scrh - 1; y++) {
		int yi = y * scrw;
		for (int x = 1; x < scrw - 1; x++) {
			bi = yi + x;
			float w = water[bi + scrs];
			water[bi + scrs] = water[bi];
			water[bi] = w;
		}
	}
	// Blur pass..amazing how much of a difference this makes!!
	for (int y = 1; y < scrh - 1; y++) {
		int yi = y * scrw;
		for (int x = 1; x < scrw - 1; x++) {
			bi = yi + x;
			water[bi] = (water[bi] + water[bi - 1] + water[bi + 1] + water[bi - scrw] + water[bi + scrw]) * .2f;
		}
	}
}

///////////////////////////////////////////////////////////////////////////////

//Build the distortions tables
void Build_Tables(void)
{
	int i, j, dst = 0;
	float x, y;

	for (j = 0; j < (240 * 2); j++)
		for (i = 0; i < (320 * 2); i++)
		{
			x = (float)i;
			y = (float)j;

			DispX[dst] = (signed char)(8 * (2 * (sin(x / 20) + sin(x * y / 2000) +
				sin((x + y) / 100) + sin((y - x) / 70) +
				sin((x + 4 * y) / 70) + 2 *
				sin(hypot(256 - x, (150 - y / 8)) / 40))));

			DispY[dst] = (signed char)(8 * ((cos(x / 31) + cos(x * y / 1783) +
				2 * cos((x + y) / 137) +
				cos((y - x) / 55) + 2 * cos((x + 8 * y) / 57) +
				sin(hypot(384 - x, (274 - y / 9)) / 51))));

			dst++;
		}
}
static void Distortion(int x1, int y1, int x2, int y2, uint32_t* DBuf, uint32_t* pImg)
{
	int dst = 0;
	int src1 = y1 * (320 * 2) + x1;
	int src2 = y2 * (320 * 2) + x2;
	int dx, dy, i, j;

	for (j = 0; j < 240; j++)
	{
		for (i = 0; i < 320; i++)
		{
			dy = j + (DispY[src1] >> 3);
			dx = i + (DispX[src2] >> 3);

			if ((dy >= 0) && (dy <= 239) && (dx >= 0) && (dx <= 319))
				DBuf[dst] = pImg[PixTab[dy] + dx];
			else
				DBuf[dst] = 0x0;

			dst++;
			src1++;
			src2++;
		}

		src1 += 320;
		src2 += 320;
	}
}

void Do_Distort(uint32_t *src, uint32_t *dest)
{
	int x1, y1, x2, y2;
	long long currentTime = 0;

	currentTime = GetTickCount() >> 3;

	//Change this value for some different effects :)
	x1 = (320 / 2) + (int)(159.0f * cos((double)currentTime / 205));
	x2 = (320 / 2) + (int)(159.0f * sin((double)-currentTime / 197));
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