#include "sys/msys.h"
#include "intro.h"

#include "sprite.h"
#include "font.h"

GLuint fonttexture;
sprite2 fontletter;
GLuint fontrendert;
static int text_aa;

GLvoid fontwriter_init(GLvoid)								// Build Our Font Display List
{
	float	cx;											// Holds Our X Character Coord
	float	cy;											// Holds Our Y Character Coord
	GLuint fonttexture = loadTGATextureMemory(font,font_len,false);
	fontletter.setTextureID( fonttexture);
	//initShader( &text_aa, bg_vert, (const char*)textaa_frag );
}

int gettablepos(char c)
{
	int pos;
	if (c >= 'A' && c <= 'Z')pos = c - 'A';
	if (c>='1' && c <= '9')
	{ 
		pos = c - '1';
		pos +=26;
	}

	//space
	if (c == 0x20)pos = 38;
	//.
	if (c == 0x2E)pos = 37;
	//!
	if (c == 0x21)pos = 35;
	if (c == ',')pos = 36;


	return pos;
}




void Print(float x1, float y1, const char *fmt, ...)
{
	va_list ap;
	unsigned char buffer[256];
	va_start(ap, fmt);
	vsprintf((char*)buffer, fmt, ap);
	va_end(ap);
	
	double objX, objY;
	world_coords coords = GetWorldCoords((int)x1,(int)y1);
	objX = coords.x;
	objY = coords.y;


	fontletter.setPositionY(-objY );
	fontletter.setAlpha( 1.0f );
	fontletter.setWidth( 0.7f );
	fontletter.setHeight( 0.7f );
	fontletter.setFrameDelay( 1.0f );
//	fontletter.setTextureAnimeInfo( 936,24, 24, 39,1);
	fontletter.setTextureAnimeInfo(936,24,24,24,39,1,39);
	//fontletter.setTextureAnimeInfo( 112,80, 16, 16,7,5,35);

	int l = strlen((char*)buffer);
	for(int a = 0; a < l; a++)
	{
		unsigned char c = buffer[a];
		int pos = gettablepos(c);
		fontletter.setPositionX(objX);
		fontletter.render(pos,255/255.0,255/255.0,255/255.0,1.0);
		objX -= -0.42;
	}
}


void fontwrite_write(int x, int y,char* text, float time)
{
	BeginPerspective(45.0, (GLdouble)XRES / (GLdouble)YRES) ;
	gluLookAt( 0.0, 0.0, 10,  // Camera Position
		0.0, 0.0, 0.0, // Look At Point
		0.0, 1.0, 0.0);// Up Vector
	Print(x,y, text);
	EndProjection();
}