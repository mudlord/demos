#include "sys/msys.h"
#include "intro.h"

typedef struct _tagRGB {
	BYTE r, g, b;
}RGB;

#ifndef PI
#define PI 3.141592653589794
#endif

//plasma implementation
int plasma_w = 256;
int plasma_h = 256;
BYTE *plasma_data;
RGB  plasma_palette[256];
BYTE *m_plasma1;
BYTE *m_plasma2;
int plasma_texsize;

GLuint plasma_texture;
sprite plasma_spr;
sprite plasma_spr2;

void plasma_palleteupdate()
{
	double factor=(double)GetTickCount();
	for (int i=0; i<256; ++i)
	{
		plasma_palette[i].r=(BYTE)(128+127*sin(i*PI/64+factor/133));
		plasma_palette[i].g=(BYTE)(128-127*cos(i*PI/128+factor/61));
		plasma_palette[i].b=(BYTE)(128+127*sin(i*PI/64+factor/72));
	}
}

BOOL plasma_init()
{
	plasma_texsize=plasma_w*plasma_h*3;	// RGB occupy 3 bytes
	plasma_data=new BYTE[plasma_texsize];
	ZeroMemory(plasma_data, plasma_texsize); 
	m_plasma1=new BYTE[plasma_w*plasma_h*4];	// plasma buffer will 4 times bigger than texture.
	if (NULL==m_plasma1)return FALSE;
	m_plasma2=new BYTE[plasma_w*plasma_h*4]; // plasma buffer will 4 times bigger than texture.
	if (NULL==m_plasma2)return FALSE;
	int width=plasma_w*2;
	int height=plasma_h*2;
	int offs=0;
	for (int y=0; y<height; ++y)
	{
		for (int x=0; x<width; ++x)
		{
			m_plasma1[offs]=(BYTE)(128+127*sin((double)hypot(x, plasma_h-y)/48));
			m_plasma2[offs]=(BYTE)(128+127*sin((float)x/(37+15*cos((float)y/74)))
			*cos((float)y/(31-21*sin((float)x/57))));
			++offs;
		}
	}

	glGenTextures(1, &plasma_texture);
	glBindTexture(GL_TEXTURE_2D, plasma_texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, 3, plasma_w, plasma_w, 0, GL_RGB, GL_UNSIGNED_BYTE,NULL);
	glBindTexture(GL_TEXTURE_2D,0);

	plasma_spr2.xsize =47;
	plasma_spr2.ysize = 47;
	plasma_spr2.texture = plasma_texture;
	plasma_spr2.x = XRES-96;
	plasma_spr2.y = 220;

	plasma_spr.xsize = 190;
	plasma_spr.ysize = 190;
	plasma_spr.x = XRES-96;
	plasma_spr.y = 90;
	plasma_spr.texture = plasma_texture;

	return TRUE;
}

void plasma_run()
{
	// half size of texture's width and height.
	static int half_width =plasma_w/2;
	static int half_height=plasma_h/2;

	// the following lines to determine a movement on every plasma.
	// method from Alex Champandard's tutorial "The art of demomaking"
	// URL: http://www.flipcode.com/demomaking/
	// "09/13/99 - Issue 04 - Per Pixel Control"

	int x1, y1;	// top-left point in m_plasma1 buffer.
	int x2, y2; // top-left point in m_plasma2 buffer.
	int offs1, offs2, dst;
	BYTE col;
	double factor=(double)(GetTickCount()>>4);

	x1=half_width  + (int)((half_width-1)  * sin(factor/128)  );
	x2=half_width  + (int)((half_width-1)  * cos(factor/-74)  );
	y1=half_height + (int)((half_height-1) * cos(factor/-113) );
	y2=half_height + (int)((half_height-1) * sin(factor/33)   );

	offs1=y1*plasma_w*2+x1;
	offs2=y2*plasma_w*2+x2;

	plasma_palleteupdate();

	// do it.
	dst=0;
	for (int y=0; y<plasma_h; ++y)
	{
		for (int x=0; x<plasma_w; ++x)
		{
			col=m_plasma1[offs1]+m_plasma1[offs2]+
				m_plasma2[offs1]+m_plasma2[offs2];

			// because we working under RGB mode, so we just select color
			// from our custom palette, and set corresponding RGB channel
			// to texture buffer.
			plasma_data[dst  ]=plasma_palette[col].r;
			plasma_data[dst+1]=plasma_palette[col].g;
			plasma_data[dst+2]=plasma_palette[col].b;

			++offs1; ++offs2; dst+=3;
		}
		offs1+=plasma_w;	offs2+=plasma_w;
	}

	glBindTexture(GL_TEXTURE_2D,plasma_texture);
	glTexImage2D(GL_TEXTURE_2D, 0, 3,plasma_w, plasma_h, 0, GL_RGB, GL_UNSIGNED_BYTE,
	plasma_data);
	glBindTexture(GL_TEXTURE_2D,0);
	draw_sprite(plasma_spr, XRES, YRES, false);
	draw_sprite(plasma_spr2, XRES, YRES, false);

}

//smoke
sprite smoke_spr;
GLuint smoke_tex;

void smoke_init()
{

}

void smoke_run()
{

}


void fx_init()
{
	plasma_init();


}

void fx_do()
{
	plasma_run();
}