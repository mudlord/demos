#include "sys/msys.h"
#include "intro.h"
#include <stdint.h>
#include "ra_tile.h"
#include "ra_logo.h"
#include "ra_gradient.h"

GLuint dinothawr_tex,logo_tex,gradient_tex,tile_tex; 
unsigned char* dinothawr_data,*logo_data,*tile_data,*gradient_data;
int dinothawr_h, dinothawr_w, tile_h, tile_w,logo_h, logo_w,gradient_h, gradient_w;



sprite topgradient;
sprite bottomgradient;
sprite logo_spr;
sprite rotozoom_spr;

int rotoheight = 512;
int rotowidth = 512;
GLuint rototex;
unsigned char rotodata[512*512*4];




int twistheight = 86;
int twistwidth = 350;
GLuint twisttex;
unsigned char twistdata[350*86*4];
sprite twistspr;

// Rotozooms 'texture' into 'out'. Zoom center is defined in 'cX' and 'cY'. 't' is the current time.
// Scale is computed internally, but you can change that if you want. If you change the texture
// size, you MUST change the bitshift/bitmask constants in the texture mapper!

static void rotozoom(uint8_t *dest,
	const uint32_t width, const uint32_t height,
	const uint8_t *texture,
	const float cX, const float cY,
	const float t)
{
	if (!dest || !texture || width < 1U || height < 1U)
		return;

	// precalculate sine/cosine and scaling for this frame
	const float scale = (sin(t) + 1.0f) * 1.0f + 0.1f,
		s = sin(t) * scale,
		c = cos(t) * scale;

	for (uint32_t y = 0U; y < height; y++) {
		// Y rotation
		const float sy = (y - cY) * s,
			cy = (y - cY) * c;

		// We use a "row vector" to move in the source image. This way we can
		// go to the next pixel with simply adding the delta to the vector.
		// Floats are inaccurate, but they're accurate enough for this.

		// start position ("left edge")
		float pX = -cX * c - sy,
			pY = -cY * s + cy;

		// per-pixel delta (compute next pixel in advance and get delta)
		const float dX = ((1.0f - cX) * c - sy) - pX,
			dY = ((1.0f - cY) * s + cy) - pY;

		// Our texturing code does not like negative numbers, so add some
		// constant (2048 here) to them to make them always positive. If you
		// change the window size and see messed up pixels, change this
		// constant. It does not have to be pow2.
		pX += cX + 2048.0f;
		pY += cY + 2048.0f;

		for (uint32_t x = 0U; x < width; x++) {
			// get texture position
			const int32_t tX = (int32_t)pX,
				tY = (int32_t)pY;

			const uint32_t utX = (uint32_t)tX,
				utY = (uint32_t)tY;
			// Get four pixel pointers around the computed texel; use bitwise
			// ops to wrap coordinates. The first (& 127) is (width - 1) and the
			// second (& 127) is (height - 1). (<< 7) is log2(width). Texture
			// width and height must be pow2, but it can be rectangular.
			const uint8_t *pa = texture +  ((((utY       & 255) << 8) +  (utX       & 255)) * 3);
			*dest++ = (uint8_t)pa[0];
			*dest++ = (uint8_t)pa[1];
			*dest++ = (uint8_t)pa[2];

			// next pixel
			pX += dX;
			pY += dY;
			dest++;
		}
	}
}


unsigned char* flip(int width, int height,int channels, unsigned char* data)
{
	unsigned char *img = (unsigned char*)malloc( width*height*channels );
	memcpy( img, data, width*height*channels );
		int i, j;
		for( j = 0; j*2 < height; ++j )
		{
			int index1 = j * width * channels;
			int index2 = (height - 1 - j) * width * channels;
			for( i = width * channels; i > 0; --i )
			{
				unsigned char temp = img[index1];
				img[index1] = img[index2];
				img[index2] = temp;
				++index1;
				++index2;
			}
		}
		return img;
}

void scene1_free()
{
	
}

void logo_init()
{

}

void draw_sprite_dir_x(sprite spr, int xres, int yres, bool flip_y)
{
	float zPos = 0.0;
	float  tX=spr.xsize/2.0f;
	float  tY=spr.ysize/2.0f;

	static float floor_move = 0.0;
	floor_move+=0.011;
	
	glMatrixMode(GL_TEXTURE);
	glPushMatrix();
	glLoadIdentity();
	glEnable(GL_TEXTURE_2D);
	glBindTexture( GL_TEXTURE_2D,  spr.texture);
	glTranslatef(floor_move,1.0,0.0);
	if (flip_y)BeginOrtho2D(xres,yres,true);
	else
		BeginOrtho2D(xres,yres);
	glEnable(GL_TEXTURE_2D);
	glDisable(GL_DEPTH_TEST);
	glBindTexture(GL_TEXTURE_2D,spr.texture);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
	glPushMatrix();  // Save modelview matrix
	glTranslatef(spr.x,spr.y,0.0f);  // Position sprite
	//glColor4f(spr.rcol,spr.gcol,spr.bcol,spr.acol);
	glColor4ub(spr.rcol,spr.gcol,spr.bcol,spr.acol);
	glBegin(GL_QUADS);                                   // Draw sprite 
	glTexCoord2f(0.0f,0.0f); glVertex3i(-tX, tY,zPos);
	glTexCoord2f(0.0f,1.0f); glVertex3i(-tX,-tY,zPos);
	glTexCoord2f(1.0f,1.0f); glVertex3i( tX,-tY,zPos);
	glTexCoord2f(1.0f,0.0f); glVertex3i( tX, tY,zPos);
	glEnd();
	glPopMatrix();  // Restore modelview matrix
	glBindTexture(GL_TEXTURE_2D,0);
	glDisable(GL_TEXTURE_2D);
	glDisable(GL_BLEND);
	glEnable(GL_DEPTH_TEST);
	EndProjection();
	glMatrixMode(GL_TEXTURE);
	glPopMatrix();
}


void draw_sprite_dir_y(sprite spr, int xres, int yres, bool flip_y)
{
	float zPos = 0.0;
	float  tX=spr.xsize/2.0f;
	float  tY=spr.ysize/2.0f;

	static float floor_move = 0.0;

	floor_move-=0.011;
	

	glMatrixMode(GL_TEXTURE);
	glPushMatrix();
	glLoadIdentity();
	glEnable(GL_TEXTURE_2D);
	glBindTexture( GL_TEXTURE_2D,  spr.texture);
	glTranslatef(floor_move,1.0,0.0);
	if (flip_y)BeginOrtho2D(xres,yres,true);
	else
		BeginOrtho2D(xres,yres);
	glEnable(GL_TEXTURE_2D);
	glDisable(GL_DEPTH_TEST);
	glBindTexture(GL_TEXTURE_2D,spr.texture);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
	glPushMatrix();  // Save modelview matrix
	glTranslatef(spr.x,spr.y,0.0f);  // Position sprite
	//glColor4f(spr.rcol,spr.gcol,spr.bcol,spr.acol);
	glColor4ub(spr.rcol,spr.gcol,spr.bcol,spr.acol);
	glBegin(GL_QUADS);                                   // Draw sprite 
	glTexCoord2f(0.0f,0.0f); glVertex3i(-tX, tY,zPos);
	glTexCoord2f(0.0f,1.0f); glVertex3i(-tX,-tY,zPos);
	glTexCoord2f(1.0f,1.0f); glVertex3i( tX,-tY,zPos);
	glTexCoord2f(1.0f,0.0f); glVertex3i( tX, tY,zPos);
	glEnd();
	glPopMatrix();  // Restore modelview matrix
	glBindTexture(GL_TEXTURE_2D,0);
	glDisable(GL_TEXTURE_2D);
	glDisable(GL_BLEND);
	glEnable(GL_DEPTH_TEST);
	EndProjection();
	glMatrixMode(GL_TEXTURE);
	glPopMatrix();
}

void logo_draw()
{
	
	draw_sprite_dir_x(topgradient,XRES,YRES,true);
	draw_sprite_dir_y(bottomgradient,XRES,YRES,true);
}

GLuint make_texture (int width,int height, unsigned char* data)
{
	GLuint texture;
	glGenTextures( 1, &texture );
	glBindTexture( GL_TEXTURE_2D, texture );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
	glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB, width,height, 0,
		GL_RGB, GL_UNSIGNED_BYTE, data );
	return texture;
}

void scene1_init()
{
	 int comp;
	 dinothawr_data= stbi_load_from_memory(ra_tile,ra_tile_len,&dinothawr_w,&dinothawr_h,&comp,3);
	 logo_data = stbi_load_from_memory(ra_logo,ra_logo_len,&logo_w,&logo_h,&comp,3);
     unsigned char *gradient1 = stbi_load_from_memory(ra_gradient,ra_gradient_len,&gradient_w,&gradient_h,&comp,0);
	 gradient_data = flip(gradient_w,gradient_h,comp,gradient1);
	 stbi_image_free( gradient1 );
	 //gradient textures

	GLuint tex_topgrad = make_texture(gradient_w,gradient_h,gradient_data);
	topgradient.xsize = XRES;
	topgradient.ysize = 40;
	topgradient.x = XRES/2;
	topgradient.y = 20;
	topgradient.texture =tex_topgrad;
	topgradient.acol = 255;
	topgradient.rcol = 255;
	topgradient.gcol = 255;
	topgradient.bcol = 255;

	bottomgradient.xsize = XRES;
	bottomgradient.ysize = 60;
	bottomgradient.x = XRES/2;
	bottomgradient.y = YRES - 20;
	bottomgradient.texture =tex_topgrad;
	bottomgradient.acol = 255;
	bottomgradient.rcol = 255;
	bottomgradient.gcol = 255;
	bottomgradient.bcol = 255;

	memset(rotodata,0xff,rotowidth*rotoheight*4);
	glGenTextures( 1, &rototex );
	glBindTexture( GL_TEXTURE_2D, rototex );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
	glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, rotowidth, rotoheight, 0,
		GL_RGBA, GL_UNSIGNED_BYTE, rotodata );
	
	rotozoom_spr.xsize = XRES;
	rotozoom_spr.ysize = YRES;
	rotozoom_spr.x = XRES/2;
	rotozoom_spr.y = YRES/2;
	rotozoom_spr.texture =rototex;
	rotozoom_spr.acol = 255;
	rotozoom_spr.rcol = 255;
	rotozoom_spr.gcol = 255;
	rotozoom_spr.bcol = 255;
	

	memset(twistdata,0xff,twistwidth*twistheight*4);
	glGenTextures( 1, &twisttex );
	glBindTexture( GL_TEXTURE_2D, twisttex );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
	glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, twistwidth, twistheight, 0,
		GL_RGBA, GL_UNSIGNED_BYTE, twistdata );

	twistspr.xsize = XRES;
	twistspr.ysize = 86;
	twistspr.x = XRES/2;
	twistspr.y = YRES-40;
	twistspr.texture =twisttex;
	twistspr.acol = 255;
	twistspr.rcol = 255;
	twistspr.gcol = 255;
	twistspr.bcol = 255;

}

static const float kMaxCoverage = 0.75f / 2.0f;

// Lissajous figure
static void lissajous(const uint32_t width, const uint32_t height, const float t, float &x, float &y)
{
	const float AA = width * kMaxCoverage,           // max size
		BB = height * kMaxCoverage,
		a = 2.0f,                           // 2:3 figure
		b = 3.0f,
		gamma = 0.0f;

	x = width / 2.0f  + AA * sin(a * t + gamma);
	y = height / 2.0f + BB * sin(b * t);
}

void draw_rotozoomer()
{
	static float t = 0.0f;
	float cX, cY;
	lissajous(rotowidth, rotoheight, t / 5.0f, cX, cY);
	rotozoom(rotodata, rotowidth, rotoheight, dinothawr_data, cX, cY, t);
	glBindTexture(GL_TEXTURE_2D,rototex);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, rotowidth, rotoheight, 
		GL_RGBA, GL_UNSIGNED_BYTE, rotodata);
	glBindTexture(GL_TEXTURE_2D,0);
	draw_sprite(rotozoom_spr,XRES,YRES,false);

	t += 0.01f;
}

static inline int ftoi(const float f)
{
	return f >= 0.0f ? (int)(f + 0.5f) : (int)(f - 0.5f);
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

	// During profiling, I noticed that floating-point texture interpolation with
	// fixed-point shading interpolation is the fastest combination. I don't know why.

	// texture interpolation (+ 2U is a hack to prevent small seams)
	const float dT = ((float)twistheight / (h + 2U));
	float tex = 0.0f;

	// shading interpolation (fixed-point)
	// (there are many ways to calculate s1 and s2, this one looks good)
	const int s1 = ftoi(255.0f * min(z1, z2)),
		s2 = ftoi(255.0f * max(z1, z2));

	const int dS = ((s2 - s1) << 16) / (signed)h;
	int shade = s1 << 16;

	// setup pointers
	const uint8_t *column = texture + (x * 3);
	uint8_t *destination = dest + ((y1 * twistwidth+ x) << 2);
	const uint32_t stride = (twistwidth<< 2) - 3;
	uint32_t y = y1;

	// draw
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
	// (Y, Z) source coordinates for X-aligned quads (spi4=sin(PI/4); when
	// rotated, it gives values between -1 and 1). Scale must be below 1.0.
	static const float spi4 = 0.7071067811865f,
		scale = 0.95f;

	static const float coords[4][2] =
	{
		{ -spi4 * scale,  spi4 * scale },
		{ -spi4 * scale, -spi4 * scale },
		{  spi4 * scale, -spi4 * scale },
		{  spi4 * scale,  spi4 * scale }
	};

	// Y and Z for each vertex after rotation and scaling
	uint32_t y[4];
	float z[4];

	if (!output)
		return;

	

	if (!texture)
		return;

	const float height = twistheight - 1;

	// draw each slice
	for (uint32_t x = 0U; x < twistwidth; x++) {
		// get rotation angle
		const float r = twisty(angle, x),
			c = cos(r), s = sin(r);

		// rotate vertices
		for (uint32_t i = 0U; i < 4U; i++) {
			z[i] =            ((c * coords[i][1] - s * coords[i][0]) + 1.0f) / 2.0f;
			y[i] = (uint32_t)(((s * coords[i][1] + c * coords[i][0]) / 2.0f + 0.5f) * height + 0.5f);
			y[i] = min(twistheight - 1, y[i]);
		}

		// Draw sides, sort by Y to get proper ordering. We always draw from top
		// to bottom (Y1 > Y2); if (Y1 < Y2), then the side is back-facing.
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
	draw_sprite(twistspr,XRES,YRES,false);
}


void scene1_render(float time, float delta)
{
	draw_rotozoomer();
}