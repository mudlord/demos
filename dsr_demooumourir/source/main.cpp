
#include <3ds.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#define STBI_NO_STDIO
#define  STBI_ONLY_PNG
#define STBI_ASSERT(x)
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "syntaxman.h"
#include "bottomscreen.h"

#define TOPSCREEN_W 400
#define TOPSCREEN_H 240

#define BOTTOMSCREEN_W 320
#define BOTTOMSCREEN_H 240
#define LENSW					90	  //Lens dimension
#define LENSZ					15	  //Magnification
#define scrw BOTTOMSCREEN_W // Screen width in pixels.
#define scrh BOTTOMSCREEN_H // Screen height in pixels.
#define scrs scrw * scrh

typedef struct {
	int w;
	int h;
}dim;

typedef struct _tagRGB {
	uint8_t r, g, b;
}RGB;

void RGBtoBGR(RGB* img_data, int x, int y)
{
	RGB* image_buffer = &img_data[0];
	RGB* end = image_buffer + (x*y*sizeof(RGB)) / sizeof(RGB);
	while (image_buffer < end) {
		RGB pixel = *image_buffer;
		*image_buffer = {pixel.b,pixel.g,pixel.r};
		image_buffer++;
	}
}



inline void rotate90(RGB* srcBuf,RGB* dstBuf, dim* src,dim *dst)
{
	for (int y = 0, destinationColumn = src->h - 1; y < src->h; ++y, --destinationColumn)
	{
		int offset = y * src->w;
		for (int x = 0; x < src->w; x++)
		dstBuf[(x * src->h) + destinationColumn] = srcBuf[offset + x];
	}
	dst->w = src->h;
	dst->h = src->w;
}




//Lens class
class TLens
{
private:
	int Lens[LENSW][LENSW];	//Buffer with precalculated distorsion

public:
	TLens();							//Constructor

										//Apply lens effect on a buffer
	void Apply_Lens(RGB *Dest,RGB *Src, int Ox, int Oy);
};

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
			offset = (iy * BOTTOMSCREEN_W + ix);
			Lens[LENSW / 2 - y][LENSW / 2 - x] = -offset;
			Lens[LENSW / 2 + y][LENSW / 2 + x] = offset;
			offset = (-iy * BOTTOMSCREEN_W + ix);
			Lens[LENSW / 2 + y][LENSW / 2 - x] = -offset;
			Lens[LENSW / 2 - y][LENSW / 2 + x] = offset;
		}
}

void TLens::Apply_Lens(RGB *Dest,RGB *Src, int Ox, int Oy)
{
	//Apply lens deformation on image and save it in Dest

	int temp, pos;
	register int x, y;


	for (y = 0; y < LENSW; y++)
	{
		temp = (y + Oy) *BOTTOMSCREEN_W + Ox;

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

void Do_LensMoving(RGB *srcImage, RGB* dstImage)
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

	if (x > (BOTTOMSCREEN_W - LENSW - 3) || x < 3)
		xd = -xd;

	if (x2 > (BOTTOMSCREEN_W - LENSW - 3) || x2 < 3)
		xd2 = -xd2;

	if (y > (BOTTOMSCREEN_H - LENSW - 3) || y < 3)
		yd = -yd;

	if (y2 > (BOTTOMSCREEN_H - LENSW - 3) || y2 < 3)
		yd2 = -yd2;

}

unsigned char *DispX, *DispY;						//Precalculated distortion tables
///////////////////////////////////////////////////////////////////////////////

void Build_Tables(void)
{
	int i, j, dst = 0;
	float x, y;

	for (j = 0; j < (240 * 2); j++)
		for (i = 0; i < (400 * 2); i++)
		{
			x = (float)i;
			y = (float)j;

			DispX[dst] = (unsigned char)(8 * (2 * (sin(x / 20) + sin(x * y / 2000) +
				sin((x + y) / 100) + sin((y - x) / 70) +
				sin((x + 4 * y) / 70) + 2 *
				sin(hypot(256 - x, (150 - y / 8)) / 40))));

			DispY[dst] = (unsigned char)(8 * ((cos(x / 31) + cos(x * y / 1783) +
				2 * cos((x + y) / 137) +
				cos((y - x) / 55) + 2 * cos((x + 8 * y) / 57) +
				sin(hypot(384 - x, (274 - y / 9)) / 51))));

			dst++;
		}
}
static void Distortion(int x1, int y1, int x2, int y2, RGB* DBuf, RGB* pImg)
{
	int dst = 0;
	x2 = abs(x2);
	y2 = abs(y2);
	int src1 = y1 * (400 * 2) + x1;
	int src2 = y2 * (400 * 2) + x2;
	int dx, dy, i, j;

	for (j = 0; j < 240; j++)
	{
		for (i = 0; i < 400; i++)
		{
			
				dy = j + (DispY[src1] >> 3);
				dx = i + (DispX[src2] >> 3);


			if ((dy >= 0) && (dy <= 239) && (dx >= 0) && (dx <= 399))
			DBuf[dst] = pImg[dy * 400 + dx];
			
			else
				DBuf[dst] = RGB{0,0,0};

			dst++;
			src1++;
			src2++;
		}

		src1 += 400;
		src2 += 400;
	}
}

void Do_Distort(RGB *src,RGB *dest)
{
	int x1, y1, x2, y2;
	long long currentTime = 0;

	currentTime = osGetTime() >> 3;

	x1 = (400 / 2) + (unsigned int)(233.0f * cos((double)currentTime / 205));
	x2 = (400 / 2) + (unsigned int)(159.0f * sin((double)-currentTime / 197));
	y1 = (240 / 2) + (unsigned int)(103.0f * sin((double)currentTime / 231));
	y2 = (240 / 2) + (unsigned int)(127.0f * cos((double)-currentTime / 224));

	Distortion(x1, y1, x2, y2,dest,src);
}

#include "testmus.h"
u8* aud_buffer;
u32 aud_size;
// Audio functions (load audio)
void audio_load(){
    aud_size =mudlord_len;
    aud_buffer = (u8*)linearAlloc(aud_size);
	memcpy(aud_buffer,mudlord,aud_size);
    csndPlaySound(8, SOUND_FORMAT_16BIT | SOUND_REPEAT, 16000, 1, 0, aud_buffer, aud_buffer, aud_size);
    linearFree(aud_buffer);
}

// Audio functions (stop audio)
void audio_stop(void){
    csndExecCmds(true);
    CSND_SetPlayState(0x8, 0);
    memset(aud_buffer, 0, aud_size);
    GSPGPU_FlushDataCache(NULL, aud_buffer, aud_size);
    linearFree(aud_buffer);
}

int main()
{
	gfxInitDefault();
	csndInit();


  int x, y, n;
  RGB* img_data = (RGB*)stbi_load_from_memory(bottomscreen, bottomscreen_len, &x, &y, &n, 3);
  dim src,src2;
  dim dest,dst2;
	src.w = x;
	src.h = y;
   RGB* img_data2 = (RGB*)stbi_load_from_memory(syntaxman, syntaxman_len, &x, &y, &n, 3);
   src2.w = x;
	src2.h = y;
	DispX = (unsigned char*)malloc((400 * 2) * (240 * 2));
	DispY = (unsigned char*)malloc((400 * 2) * (240 * 2));
	RGBtoBGR(img_data,src.w,src.h);
    RGBtoBGR(img_data2,src2.w,src2.h);
	Build_Tables();

  RGB* image_rotate_top = (RGB*)malloc(src2.w*src2.h*sizeof(RGB));
  RGB* image_rotate_bottom = (RGB*)malloc(src.w*src.h*sizeof(RGB));
  RGB* screenbuffer = (RGB*)malloc(src.w*src.h*sizeof(RGB));
  RGB* screenbuffer2 = (RGB*)malloc(src2.w*src2.h*sizeof(RGB));
  audio_load();
	while (aptMainLoop())
	{
		gspWaitForVBlank();
		hidScanInput();

		// Your code goes here

		u32 kDown = hidKeysDown();
		if (kDown & KEY_START)
			break; // break in order to return to hbmenu

		// Example rendering code that displays a white pixel
		// Please note that the 3DS screens are sideways (thus 240x400 and 240x320)
		u8* fb2 = gfxGetFramebuffer(GFX_BOTTOM, GFX_LEFT, NULL, NULL);
		u8* fb = gfxGetFramebuffer(GFX_TOP, GFX_LEFT, NULL, NULL);
		dim dest;
	  dest.w = 0;
	  dest.h = 0;
	  memcpy(screenbuffer,img_data,240*320*sizeof(RGB));
	  Do_LensMoving(img_data,screenbuffer);
	  Do_Distort(img_data2,screenbuffer2);
		// Flush and swap framebuffers
	  rotate90(screenbuffer,image_rotate_bottom,&src,&dest);
	  rotate90(screenbuffer2,image_rotate_top,&src2,&dest);
	  memcpy(fb2,image_rotate_bottom,240*320*sizeof(RGB));
	  memcpy(fb,image_rotate_top,240*400*sizeof(RGB));
	  gfxFlushBuffers();
	  gfxSwapBuffers();
	}
	audio_stop();
	csndInit();
    free(img_data);
	free(img_data2);
	free(image_rotate_bottom);
	free(image_rotate_top);
	free(screenbuffer);
	gfxExit();
	return 0;
}
