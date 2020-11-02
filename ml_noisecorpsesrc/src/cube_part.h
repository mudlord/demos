#include "sys/msys.h"
#include "intro.h"
#include <sys/stat.h>
//data
#include "shader.h"
#include "vertexshader.h"
#include "fsh_rayt.inl"
#include "vsh_2d.inl"
#include "cubebg.h"
#include "cubetexture.h"


GLuint cubepart_bg = 0;
GLuint cubescreen_tex = 0;
GLuint raytexture_tex = 0;
GLuint cubetexture = 0;
GLuint fbo;					// Our handle to the FBO
GLuint depthBuffer;			// Our handle to the depth render buffer

int tv_shaderobj;
GLuint      tv_shadertexture;
GLuint      resolution_uniform;
GLuint      timer_uniform;

int ray_shaderobj;
GLuint      ray_shadertexture;
GLuint      rayresolution_uniform;
GLuint      raytimer_uniform;


float resolution[4] = {XRES,YRES,0,0};
float raymarch_param[4] = {0,XRES,YRES,0};
GLfloat color[4] =  {1.0f,1.0f,1.0f,1.0f};
void Cube(float size)
{
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D,cubescreen_tex);
	glBegin(GL_QUADS);
	// Front Face
	glNormal3f(-0.577f,-0.577f, 0.577f);glTexCoord2f(0.0f, 0.0f); glVertex3f(-1.0f, -1.0f,  1.0f);
	glNormal3f( 0.577f,-0.577f, 0.577f);glTexCoord2f(1.0f, 0.0f); glVertex3f( 1.0f, -1.0f,  1.0f);
	glNormal3f( 0.577f, 0.577f, 0.577f);glTexCoord2f(1.0f, 1.0f); glVertex3f( 1.0f,  1.0f,  1.0f);
	glNormal3f(-0.577f, 0.577f, 0.577f);glTexCoord2f(0.0f, 1.0f); glVertex3f(-1.0f,  1.0f,  1.0f);
	// Back Face
	glNormal3f(-0.577f,-0.577f,-0.577f);glTexCoord2f(1.0f, 0.0f); glVertex3f(-1.0f, -1.0f, -1.0f);
	glNormal3f(-0.577f, 0.577f,-0.577f);glTexCoord2f(1.0f, 1.0f); glVertex3f(-1.0f,  1.0f, -1.0f);
	glNormal3f( 0.577f, 0.577f,-0.577f);glTexCoord2f(0.0f, 1.0f); glVertex3f( 1.0f,  1.0f, -1.0f);
	glNormal3f( 0.577f,-0.577f,-0.577f);glTexCoord2f(0.0f, 0.0f); glVertex3f( 1.0f, -1.0f, -1.0f);
	// Top Face
	glNormal3f(-0.577f, 0.577f,-0.577f);glTexCoord2f(0.0f, 1.0f); glVertex3f(-1.0f,  1.0f, -1.0f);
	glNormal3f(-0.577f, 0.577f, 0.577f);glTexCoord2f(0.0f, 0.0f); glVertex3f(-1.0f,  1.0f,  1.0f);
	glNormal3f( 0.577f, 0.577f, 0.577f);glTexCoord2f(1.0f, 0.0f); glVertex3f( 1.0f,  1.0f,  1.0f);
	glNormal3f( 0.577f, 0.577f,-0.577f);glTexCoord2f(1.0f, 1.0f); glVertex3f( 1.0f,  1.0f, -1.0f);
	// Bottom Face
	glNormal3f(-0.577f,-0.577f,-0.577f);glTexCoord2f(1.0f, 1.0f); glVertex3f(-1.0f, -1.0f, -1.0f);
	glNormal3f( 0.577f,-0.577f,-0.577f);glTexCoord2f(0.0f, 1.0f); glVertex3f( 1.0f, -1.0f, -1.0f);
	glNormal3f( 0.577f,-0.577f, 0.577f);glTexCoord2f(0.0f, 0.0f); glVertex3f( 1.0f, -1.0f,  1.0f);
	glNormal3f(-0.577f,-0.577f, 0.577f);glTexCoord2f(1.0f, 0.0f); glVertex3f(-1.0f, -1.0f,  1.0f);
	// Right face
	glNormal3f( 0.577f,-0.577f,-0.577f);glTexCoord2f(1.0f, 0.0f); glVertex3f( 1.0f, -1.0f, -1.0f);
	glNormal3f( 0.577f, 0.577f,-0.577f);glTexCoord2f(1.0f, 1.0f); glVertex3f( 1.0f,  1.0f, -1.0f);
	glNormal3f( 0.577f, 0.577f, 0.577f);glTexCoord2f(0.0f, 1.0f); glVertex3f( 1.0f,  1.0f,  1.0f);
	glNormal3f( 0.577f,-0.577f, 0.577f);glTexCoord2f(0.0f, 0.0f); glVertex3f( 1.0f, -1.0f,  1.0f);
	// Left Face
	glNormal3f(-0.577f,-0.577f,-0.577f);glTexCoord2f(0.0f, 0.0f); glVertex3f(-1.0f, -1.0f, -1.0f);
	glNormal3f(-0.577f,-0.577f, 0.577f);glTexCoord2f(1.0f, 0.0f); glVertex3f(-1.0f, -1.0f,  1.0f);
	glNormal3f(-0.577f, 0.577f, 0.577f);glTexCoord2f(1.0f, 1.0f); glVertex3f(-1.0f,  1.0f,  1.0f);
	glNormal3f(-0.577f, 0.577f,-0.577f);glTexCoord2f(0.0f, 1.0f); glVertex3f(-1.0f,  1.0f, -1.0f);
	glDisable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D,0);
	glEnd();
	
//	glColor3f(color[0],color[1],color[2]);						// Set The Color To Violet

}

static float RandomFloat(float min, float max)
{
	return min + ((max - min) * (float)rand() / (float)RAND_MAX);
}


int init_ppbo_cube()
{
	glEnable(GL_TEXTURE_2D);
	glGenTextures(1, &cubescreen_tex);
	glBindTexture(GL_TEXTURE_2D,  cubescreen_tex);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA8,0,XRES,YRES,GL_RGBA,GL_UNSIGNED_BYTE, 0);
	glCopyTexImage2D(GL_TEXTURE_2D,0,GL_RGB,0,0,XRES,YRES,0);
	oglGenFramebuffersEXT(1, &fbo);
	oglBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fbo);
	oglGenRenderbuffersEXT(1, &depthBuffer);
	oglBindRenderbufferEXT(GL_RENDERBUFFER_EXT, depthBuffer);
	oglRenderbufferStorageEXT(GL_RENDERBUFFER_EXT, GL_DEPTH_COMPONENT, XRES, YRES);
	oglFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, cubescreen_tex, 0);
	oglFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GL_RENDERBUFFER_EXT, depthBuffer);
	oglBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
	glBindTexture(GL_TEXTURE_2D,  0);
	srand(GetTickCount());


	return true;
}

void rendtotex(float time)
{
	//timekeeping effects and sync
	static float framecount = 0.0;
	float trigger = 0.0;
	DWORD pos = uFMOD_GetRowOrder();
	int row =LOWORD(pos);
	int order = HIWORD(pos);
    
	if (order > 0)
	{
		switch (row)
		{
		case 4:
		case 10:
		case 20:
		case 26:
		case 29:
		case 42:
		case 52:
		case 60:
		trigger = 1.0;
		break;
		default:
		trigger = 0.0;
		break;
		}
		
	}
	
	static bool init = false;
	glClear (GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);		// Clear Screen And Depth Buffer
	glEnable(GL_TEXTURE_2D);

	//run shader
	resolution_uniform = oglGetUniformLocation( tv_shaderobj, "resolution" );
	timer_uniform = oglGetUniformLocation( tv_shaderobj, "time" );
	tv_shadertexture =  oglGetUniformLocation( tv_shaderobj, "Image" );
	oglUniform1i(tv_shadertexture,0);
	glBindTexture( GL_TEXTURE_2D,  cubescreen_tex);

	oglUseProgram(  tv_shaderobj );
	oglUniform4fv(resolution_uniform,1,resolution);
	GLfloat time2[4] = {time,framecount,trigger,0};
	oglUniform4fv(timer_uniform,1,time2);
	BeginOrtho2D(XRES,YRES);
	DrawFullScreenQuad(XRES,YRES);
	oglUseProgram( NULL );
	framecount+=0.1;
	glBindTexture( GL_TEXTURE_2D,  0);
	EndProjection();

	
}

void cubepart_open()
{
	cubepart_bg  = loadTexGenTexMemory(cubebg_data,cubebg_data_len,512,512);
	cubetexture = loadTexGenTexMemory(cubetexture_data,cubetexture_data_len,512,512);
	init_ppbo_cube();

	initShader(  &tv_shaderobj, (const char*)tvvert, (const char*)tvfrag );
	initShader(  &ray_shaderobj, (const char*)vsh_2d, (const char*)fsh_rayt );

}


static float fparams[4];
void cubepart_draw(float time)
{
	static GLfloat	mov = 0.0f;
	static GLfloat spin1 = 0.0f;
	int numrows = 9;

	fparams[ 0] = time;
	//resolution
	fparams[ 1] = float(XRES);
	fparams[ 2] = float(YRES);
	// camera target
	fparams[3] = 0.0f;

	//render scene to framebuffer texture/FBO
	oglBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fbo);
	glPushAttrib(GL_VIEWPORT_BIT);
	glViewport(0,0,XRES,YRES);
	glClearColor(0.0, 0.0, 0.0, 0.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);	// Clear Screen And Depth Buffer
	glLoadIdentity();									// Reset The Current Modelview Matrix
	//bg
	oglUseProgram( ray_shaderobj );
	oglUniform4fv( oglGetUniformLocation( ray_shaderobj, "fpar" ),  1, fparams );
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D,cubepart_bg);
	DrawRot(time);
	glBindTexture(GL_TEXTURE_2D,0);
	glDisable(GL_TEXTURE_2D);
	oglUseProgram(0);

	//cube forest with purple fog
	glPushMatrix();
	glRotatef(spin1,spin1,1.0f,1.0f);					// Rotate The Quad On The X axis ( NEW )
	glTranslatef(-30,-50,-50);

	glShadeModel(GL_SMOOTH);
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);

	for(int x=0; x<numrows; x++)
	{
		for(int y=0; y<numrows; y++)
		{
			for(int z=0; z<numrows; z++)
			{
				glPushMatrix();
				glTranslatef(x*numrows,y*numrows,z*numrows);
				Cube(1+sin(mov*x*y*z/30)/3.0f);
				glPopMatrix();
			}
		}
	}

	glDisable(GL_LIGHT0);
	glDisable(GL_LIGHTING);
	glPopMatrix();

	glPopAttrib();
	oglBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);


	//use texture from FBO for postproc
	rendtotex(time);
	glDepthMask(GL_TRUE);
	
	spin1+= 0.3f;
	mov+=0.0113f;										// Decrease The Rotation Variable For The Quad ( NEW )
}