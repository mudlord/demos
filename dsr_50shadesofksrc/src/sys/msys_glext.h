//--------------------------------------------------------------------------//
// iq . 2003/2008 . code for 64 kb intros by RGBA                           //
//--------------------------------------------------------------------------//

#ifndef _MSYS_GLEXT_H_
#define _MSYS_GLEXT_H_

#ifdef WINDOWS
#define WIN32_LEAN_AND_MEAN
#define WIN32_EXTRA_LEAN
#include <windows.h>
#endif

#include "gl3w.h"

#include <stdio.h>


// init
int msys_glextInit( void );
void initShader( int *pid,const char *vs,const char *fs );
unsigned char *readShaderFile( const char *fileName );
void Resize(int x, int y);
unsigned char *LoadImageMemory(unsigned char* data, int size, int * width, int * height);
GLuint loadTexMemory(unsigned char* data, int size, int * width, int * height, int blur);
GLuint loadTexGenTexMemory(unsigned char *data,int size, int width, int height);
GLuint loadTGATextureMemory(unsigned char* data, int size, bool blur);

struct FBOELEM {
	GLuint fbo;
	GLuint depthbuffer;
	GLuint texture;
	GLuint depthtexture;
	GLint status;
};

GLuint init_rendertexture(int resx, int resy);
FBOELEM init_fbo(int width, int height, bool fp = false);


#endif
