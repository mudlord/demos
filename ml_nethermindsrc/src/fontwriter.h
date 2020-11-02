#include "sys/msys.h"
#include "intro.h"

#include "sprite.h"
#include "font_tga.h"
//#include "font_tga2.h"
//#include "font_normal.h"

GLuint fonttexture;
sprite2 fontletter;
GLuint fontrendert;
static int text_aa;

GLvoid fontwriter_init(GLvoid)								// Build Our Font Display List
{
	float	cx;											// Holds Our X Character Coord
	float	cy;											// Holds Our Y Character Coord
	GLuint fonttexture = loadTGATextureMemory(font_tga,font_tga_len,false);
	fontletter.setTextureID( fonttexture);
	//initShader( &text_aa, bg_vert, (const char*)textaa_frag );
}

int gettablepos(char c)
{
	int pos;
	if (c >= 'A' && c <= 'Z')pos = c - 'A';
	//space
	if (c == 0x20)pos = 26;
	//.
	if (c == 0x2E)pos = 27;
	if (c>='0' && c <= '9')
	{ 
		pos = c - '0';
		pos +=28;
	}
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
	fontletter.setWidth( 0.5f );
	fontletter.setHeight( 0.5f );
	fontletter.setFrameDelay( 1.0f );
	fontletter.setTextureAnimeInfo( 608,16, 16, 16,39,1,39);
	//fontletter.setTextureAnimeInfo( 112,80, 16, 16,7,5,35);

	int l = strlen((char*)buffer);
	for(int a = 0; a < l; a++)
	{
		unsigned char c = buffer[a];
		int pos = gettablepos(c);
		fontletter.setPositionX(objX);
		fontletter.render(pos,255/255.0,255/255.0,255/255.0,1.0);
		objX -= -0.50;
	}
}


void fontwrite_write(int x, int y,char* text, float time)
{
	BeginPerspective(45.0, (GLdouble)XRES / (GLdouble)YRES) ;
	gluLookAt( 0.0, 0.0, 10,  // Camera Position
		0.0, 0.0, 0.0, // Look At Point
		0.0, 1.0, 0.0);// Up Vector


	static float sceneTime = 0;
	sceneTime += (float)time/10000.f;
	float resolution[2] = {608,16};
	//oglUseProgram(text_aa);
	//oglActiveTextureARB(GL_TEXTURE0_ARB);
	//GLuint resolution_uniform = oglGetUniformLocation(text_aa, "resolution" );
	//GLuint shadertexture =  oglGetUniformLocation( text_aa, "tex0" );
	//oglUniform1i(shadertexture,0);
	//oglUniform2fv(resolution_uniform,1,resolution);
	

	Print(x,y, text);

	//oglUseProgram(0 );


	EndProjection();

	
}