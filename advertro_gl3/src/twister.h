#include "sys/msys.h"
#include "intro.h"
#include <math.h>
#include "ra_logo.h"

#define max(x, y) (((x) > (y)) ? (x) : (y))
#define min(x, y) (((x) < (y)) ? (x) : (y))

int twistheight = 86;
int twistwidth = 350;
GLuint twisttex;
unsigned char twistdata[350*86*4];
sprite twistspr;

unsigned char *logo_data;
int logo_h, logo_w;

static inline int ftoi(const float f)
{
	return f >= 0.0f ? (int)(f + 0.5f) : (int)(f - 0.5f);
}


void init_twister()
{
	int comp;
	logo_data = stbi_load_from_memory(ra_logo,ra_logo_len,&logo_w,&logo_h,&comp,3);


	memset(twistdata,0xff,twistwidth*twistheight*4);
	glGenTextures( 1, &twisttex );
	glBindTexture( GL_TEXTURE_2D, twisttex );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
	glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, twistwidth, twistheight, 0,
		GL_RGBA, GL_UNSIGNED_BYTE, twistdata );

	twistspr.xsize = XRES;
	twistspr.ysize = 86;
	twistspr.x = 0;
	twistspr.y = 0;
	twistspr.texture =twisttex;
	twistspr.acol = 255;
	twistspr.rcol = 255;
	twistspr.gcol = 255;
	twistspr.bcol = 255;
	init_sprite(&twistspr);

}

static void drawSideTex(uint8_t *dest,
	const uint8_t *texture,
	const int x,
	const uint32_t y1, const uint32_t y2,
	const float z1, const float z2)
{
	if (!dest || !texture)
		return;

	const uint32_t h = y2 - y1,
		tw = twistwidth * 3U;

	if (h < 1U || h > twistheight - 1U)
		return;
	const float dT = ((float)twistheight / (h + 2U));
	float tex = 0.0f;
	const int s1 = ftoi(255.0f * min(z1, z2)),
		s2 = ftoi(255.0f * max(z1, z2));
	const int dS = ((s2 - s1) << 16) / (signed)h;
	int shade = s1 << 16;
	const uint8_t *column = texture + (x * 3);
	uint8_t *destination = dest + ((y1 * twistwidth+ x) << 2);
	const uint32_t stride = (twistwidth<< 2) - 3;
	uint32_t y = y1;
	while (y <= y2) {
		const uint8_t *source = column + ((uint32_t)(tex + 0.5f) * tw);

		*destination++ = (source[0] * (shade >> 16)) >> 8;
		*destination++ = (source[1] * (shade >> 16)) >> 8;
		*destination++ = (source[2] * (shade >> 16)) >> 8;

		destination += stride;
		tex += dT;
		shade += dS;
		++y;
	}
}

static float twisty(const float angle, const uint32_t x)
{
	return ((x + 64) * sin(((angle + x) / 5.0f) * 0.017453292f) / 1.5f) * 0.017453292f;
}

static void barTwister(uint8_t *output, const uint8_t *texture, const float angle)
{
	static const float spi4 = 0.7071067811865f,
		scale = 0.95f;

	static const float coords[4][2] =
	{
		{ -spi4 * scale,  spi4 * scale },
		{ -spi4 * scale, -spi4 * scale },
		{  spi4 * scale, -spi4 * scale },
		{  spi4 * scale,  spi4 * scale }
	};
	uint32_t y[4];
	float z[4];

	if (!output)
		return;



	if (!texture)
		return;

	const float height = twistheight - 1;
	for (uint32_t x = 0U; x < twistwidth; x++) {
		const float r = twisty(angle, x),
			c = cos(r), s = sin(r);

		for (uint32_t i = 0U; i < 4U; i++) {
			z[i] =            ((c * coords[i][1] - s * coords[i][0]) + 1.0f) / 2.0f;
			y[i] = (uint32_t)(((s * coords[i][1] + c * coords[i][0]) / 2.0f + 0.5f) * height + 0.5f);
			y[i] = min(twistheight - 1, y[i]);
		}

		for (uint32_t now = 0U; now < 4U; now++) {
			const uint32_t next = (now + 1U) & 3U;

			if (y[now] > y[next])
				drawSideTex(output, texture, x, y[next], y[now], z[next], z[now]);
		}
	}
}


void draw_twister(float delta)
{

	static float angle = 0.0;
	//	memset(twistdata, 0xff, twistwidth* twistheight * 4);
	for (int i=0;i<twistwidth*twistheight*4;i+=4)
	{
		twistdata[i]=0x44;
		twistdata[i+1]=0x44;
		twistdata[i+2]=0x44;
		twistdata[i+3]=0xff;
	}
	barTwister(twistdata, logo_data, angle);
	for (int i=0;i<twistwidth*twistheight*4;i+=4)
	{
		int r = twistdata[i];
		int g = twistdata[i+1];
		int b = twistdata[i+2];
		if (r == 0x44 && g == 0x44 && b == 0x44)
		{
			twistdata[i+3]=0x00;
		}

	}
	angle += delta / 5.0f;
	glBindTexture(GL_TEXTURE_2D,twisttex);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, twistwidth, twistheight, 
		GL_RGBA, GL_UNSIGNED_BYTE, twistdata);
	glBindTexture(GL_TEXTURE_2D,0);
	draw_sprite(&twistspr,XRES,YRES);

}