#include "sys/msys.h"
#include "intro.h"

#include "tex2.h"
#include "tex3.h"
#include "fdl.h"

int xsize = XRES;
int ysize = YRES;
GLuint rendertexture = 0;
GLuint shadertex1;
GLuint shadertex2;
static int shader1;
GLuint fdltex;
FBOELEM framebuffer;
FBOELEM backframebuffer;
FBOELEM backframebuffer2;
static int   pid;
static int   bgshader;

void shadertex_init()
{
#ifdef _DEBUG
	//unsigned char* vertex = readShaderFile("vertex.inl");
	//unsigned char* fragment = readShaderFile("fragment.inl");
	initShader( &pid, (const char*)vert, (const char*)tunnel_fragment );
#else
	initShader( &pid, vert, (const char*)tunnel_fragment  );
#endif
	initShader( &bgshader, bg_vert, (const char*)bg_fragment );

	shadertex1 = loadTexGenTexMemory(tex0,tex0_len,512,512);
	shadertex2 = loadTexGenTexMemory(tex1,tex1_len,512,512);
	fdltex = loadDDSTextureMemory(fdl,fdl_len,true);
	framebuffer = init_fbo(xsize,ysize);
	backframebuffer = init_fbo(xsize,ysize);
	backframebuffer2 = init_fbo(xsize,ysize);
}


void draw_tunnel(float time, float scenetime)
{
	static rotate rot3 = {0.00,0.0,0.0};
	const colors col = {175/255.0,19/255,175/255,0.7};
	rot3.x += time/7;
	rot3.y += time/9;

	float sinetime = sin(scenetime)* 1.0+1.0;
	float glenzmod= clamp(sinetime,0.1,4.0);

	start_fbo(framebuffer.fbo,xsize,ysize);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D,shadertex2);
	DrawStaticBG();
	glBindTexture(GL_TEXTURE_2D,0);
	glDisable(GL_TEXTURE_2D);
	draw_glenz(0.0, 0.0, -10,rot3,col,glenzmod);
	end_fbo();
}

void draw_waves(float time)
{
	start_fbo(backframebuffer.fbo,xsize,ysize);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D,shadertex2);
	DrawStaticBG();
	glBindTexture(GL_TEXTURE_2D,0);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
	glBindTexture(GL_TEXTURE_2D,fdltex);
	DrawZooming(12.0);
	glBindTexture(GL_TEXTURE_2D,0);
	glDisable(GL_BLEND);
	glDisable(GL_TEXTURE_2D);
	end_fbo();
	//draw_fbotexture(backframebuffer.texture,xsize,ysize);

	start_fbo(backframebuffer2.fbo,xsize,ysize);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D,backframebuffer.texture);
	DrawDistort(time/2.0);
	glBindTexture(GL_TEXTURE_2D,0);
	glDisable(GL_TEXTURE_2D);
	end_fbo();

	oglUseProgram(bgshader);
	oglActiveTextureARB(GL_TEXTURE0_ARB);
	GLuint shadertexture =  oglGetUniformLocation( bgshader, "tex0" );
	oglUniform1i(shadertexture,0);
	draw_fbotexture(backframebuffer2.texture,xsize,ysize);
	oglUseProgram(0 );
}

void draw_tunneltex(float time)
{
	static float sceneTime = 0;
	sceneTime += (float)time/1000.f;

	draw_tunnel(time,sceneTime);
	draw_waves(sceneTime);
	
}


void shadertex_do(float sceneTime)
{
	//--- update parameters -----------------------------------------
	float resolution[2] = {XRES,YRES};
	GLfloat time2[2] = {sceneTime,0};
	GLuint resolution_uniform = oglGetUniformLocation(pid, "resolution" );
	GLuint timer_uniform = oglGetUniformLocation( pid, "time" );

	oglUseProgram(pid);
	oglActiveTextureARB(GL_TEXTURE0_ARB);
	GLuint shadertexture =  oglGetUniformLocation( pid, "tex0" );
	oglUniform1i(shadertexture,0);
	glBindTexture(GL_TEXTURE_2D,shadertex1);
	oglActiveTextureARB(GL_TEXTURE1_ARB);
	GLuint shadertexture2 =  oglGetUniformLocation( pid, "tex1" );
	oglUniform1i(shadertexture2,1);
	glBindTexture(GL_TEXTURE_2D,framebuffer.texture);

	oglUniform2fv(resolution_uniform,1,resolution);
	oglUniform2fv(timer_uniform,1,time2);
	BeginOrtho2D(XRES,YRES,true);
	DrawFullScreenQuad(XRES,YRES );
	EndProjection();

	oglActiveTextureARB(GL_TEXTURE1_ARB);
	glBindTexture(GL_TEXTURE_2D, 0);
	oglActiveTextureARB(GL_TEXTURE0_ARB);
	glBindTexture(GL_TEXTURE_2D, 0);
	glDisable(GL_TEXTURE_2D);


	oglUseProgram(0 );
}