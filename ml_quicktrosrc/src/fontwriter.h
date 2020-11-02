#include "sys/msys.h"
#include "intro.h"

#include "sprite.h"
#include "font.h"

GLuint fonttexture;
sprite2 fontletter;
GLuint fontrendert;
static int font_shader;

int xsize = XRES;
int ysize = YRES;
FBOELEM framebuffer;

const char  font_vert[] = ""
	"varying vec4 vPos;"
	"void main()"
	"{"
	"gl_Position=gl_ModelViewProjectionMatrix*gl_Vertex;"
	"gl_TexCoord[0]=gl_MultiTexCoord0;"

	"vPos=gl_Position;"
	"}";

const char font_fragment[] = ""
	"uniform vec2 resolution,time;"
	"uniform sampler2D tex0;"
	"void main()"
	"{"
	"vec2 v=gl_TexCoord[0].rg;"
	"v.g+=sin(v.r*4*3.14159+time.x*1.5)/100;"
	"vec4 r=texture2D(tex0,v).rgba;"
	"gl_FragColor=vec4(r);"
	"}";

GLvoid fontwriter_init(GLvoid)								// Build Our Font Display List
{
	float	cx;											// Holds Our X Character Coord
	float	cy;											// Holds Our Y Character Coord
	GLuint fonttexture = loadTGATextureMemory(font,font_len,false);
	fontletter.setTextureID( fonttexture);
	framebuffer = init_fbo(xsize,ysize);
	initShader( &font_shader, font_vert, (const char*)font_fragment );
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
	fontletter.setTextureAnimeInfo( 448,16, 16, 16,28,1,28);
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

void draw_fonttexture(GLuint texture,int width, int height )
{
	glEnable(GL_TEXTURE_2D);
	glDisable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
	BeginOrtho2D(width,height,true);
	glBindTexture( GL_TEXTURE_2D,  texture);
	DrawFullScreenQuad(width,height);
	glBindTexture( GL_TEXTURE_2D,  0);
	glDisable(GL_BLEND);
	glEnable(GL_DEPTH_TEST);
	EndProjection();
}

void fontwrite_write(int x, int y,char* text, float time)
{
	start_fbo(framebuffer.fbo,xsize,ysize);
	glClearColor(0.0, 0.0, 0.0, 0.0);
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
	BeginPerspective(45.0, (GLdouble)XRES / (GLdouble)YRES) ;
	gluLookAt( 0.0, 0.0, 10,  // Camera Position
		0.0, 0.0, 0.0, // Look At Point
		0.0, 1.0, 0.0);// Up Vector

	Print(x,y, text);
	EndProjection();

	end_fbo();

	float time2=timeGetTime();
	time2 /= 100.00;
	float resolution[2] = {315,58};
	GLfloat time_uniform[2] = {time2,0};
	GLuint resolution_uniform = oglGetUniformLocation(font_shader, "resolution" );
	GLuint timer_uniform = oglGetUniformLocation( font_shader, "time" );
	oglUseProgram(font_shader);
	GLuint shadertexture =  oglGetUniformLocation( font_shader, "tex0" );
	oglUniform1i(shadertexture,0);
	oglUniform1i(shadertexture,0);
	oglUniform2fv(resolution_uniform,1,resolution);
	oglUniform2fv(timer_uniform,1,time_uniform);
	draw_fonttexture(framebuffer.texture,XRES,YRES);
	oglUseProgram(0 );
	
	
}