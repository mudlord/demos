#include "sys/msys.h"
#include "intro.h"

#include "cave.inl"
#include "logo.h"



GLuint logotex;

static int shader3;
static int shader1;
static int shader2;
static int shader4;

static int shader[4];
GLuint shadertex;
sprite logospr;
sprite logofader;



void logofader_init()
{
	GLuint logotex = loadTGATextureMemory(logo,logo_len,true);
	logofader.xsize = XRES;
	logofader.ysize = YRES;
	logofader.x = XRES/2;
	logofader.y = YRES/2;
	logofader.texture = logotex;
	logofader.acol = 255;
	logofader.rcol = 0;
	logofader.gcol = 0;
	logofader.bcol = 0;
}

void shadertex_init()
{
	initShader( &shader3, vsh_cave, (const char*)shader3frag  );
	initShader( &shader[0], vsh_cave, (const char*)shader1frag  );
	initShader( &shader[1], vsh_cave, (const char*)shader2frag  );
	initShader( &shader[2], vsh_cave, (const char*)shader4frag  );
	initShader( &shader[3], vsh_cave, (const char*)shader5frag  );
	
	logotex = loadTGATextureMemory(logo,logo_len,true);

	float tSize = 128;

	logospr.xsize = XRES;
	logospr.ysize = YRES;
	logospr.x = XRES/2;
	logospr.y = YRES/2;
	logospr.texture = logotex;
	logospr.acol = 1;
	logospr.rcol = 1;
	logospr.gcol = 1;
	logospr.bcol = 1;

	
}

void shadertex_do(float sceneTime, int shaderval)
{
	float resolution[2] = {XRES,YRES};
	float time[1] = {sceneTime};
	oglUseProgram(shaderval);
	oglUniform2fv(oglGetUniformLocation(shaderval, "resolution" ),1,resolution);
	oglUniform1fv( oglGetUniformLocation( shaderval, "time" ),  1, time );
	draw_sprite(logospr,XRES,YRES,true);
	oglUseProgram(0 );
}

void shadertex3_do(float sceneTime)
{
	float camera_x = sin(sceneTime*2.0)* 0.5 - 0.3;
	float camera_y = sin(sceneTime*2.0)* -0.5;
	float glenzmod= clamp(camera_x,0.1,4.0);

	float camera[2] = {camera_x,-camera_y};

	static float fparams[4];
	fparams[ 0] = sceneTime;
	fparams[ 1] = XRES;
	fparams[ 2] = YRES;
	// camera target
	fparams[ 3] = 0.0f;


	oglUseProgram(shader3);
	GLuint shadertexture =  oglGetUniformLocation( shader3, "tex0" );
	oglUniform2fv(oglGetUniformLocation(shader3, "camera" ),1,camera);
	oglUniform4fv( oglGetUniformLocation( shader3, "fpar" ),  1, fparams );
	draw_sprite(logospr,XRES,YRES,true);
	oglUseProgram(0 );
}