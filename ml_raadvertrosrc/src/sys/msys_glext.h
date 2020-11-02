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
#include <GL/gl.h>
#include <GL/GLU.h>
#include "glext.h"
#include "wglext.h"
#include <stdio.h>

typedef struct{
	BYTE red;
	BYTE green;
	BYTE blue;
	BYTE reserved;
}PALCOL;

#pragma pack(push, 1) // exact fit - no padding
struct head {
	DWORD  signature; 
	DWORD   width;
	DWORD   height;
	PALCOL pallete[256];
};
#pragma pack(pop)

#define NUMFUNCIONES (1)

extern void *msys_oglfunc[NUMFUNCIONES];
#define owglswapinterval ((PFNWGLSWAPINTERVALEXTPROC)msys_oglfunc[0])

/*

typedef void (APIENTRY * PFNGLGENVERTEXARRAYSPROC) (GLsizei n, GLuint *arrays);
typedef void (APIENTRY * PFNGLBINDVERTEXARRAYPROC) (GLuint array);
typedef void (APIENTRY * PFNGLENABLEVERTEXATTRIBARRAYPROC) (GLuint index);
typedef void (APIENTRY * PFNGLVERTEXATTRIBPOINTERPROC) (GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, 
	const GLvoid *pointer);
typedef void (APIENTRY * PFNGLDELETEVERTEXARRAYSPROC) (GLsizei n, const GLuint *arrays);
typedef void (APIENTRY * PFNGLDISABLEVERTEXATTRIBARRAYPROC) (GLuint index);
typedef GLint (APIENTRY * PFNGLGETATTRIBLOCATIONPROC) (GLuint program, const char *name);

#define oglGenVertexArrays  ((PFNGLGENVERTEXARRAYSPROC)msys_oglfunc[46])
#define oglBindVertexArray   ((PFNGLBINDVERTEXARRAYPROC)msys_oglfunc[47])
#define oglEnableVertexAttribArray   ((PFNGLENABLEVERTEXATTRIBARRAYPROC)msys_oglfunc[48])
#define oglVertexAttribPointer  (( PFNGLVERTEXATTRIBPOINTERPROC)msys_oglfunc[49])
#define oglDeleteVertexArrays  ((PFNGLDELETEVERTEXARRAYSPROC)msys_oglfunc[50])
#define oglDisableVertexAttribArray  ((PFNGLDISABLEVERTEXATTRIBARRAYPROC)msys_oglfunc[51])

#define oglGenBuffers  ((PFNGLGENBUFFERSPROC)msys_oglfunc[52])
#define oglGetAttribLocation   ((PFNGLGETATTRIBLOCATIONPROC)msys_oglfunc[53]) */



// init
int msys_glextInit( void );
void initShader( int *pid,const char *vs,const char *fs );
unsigned char *readShaderFile( const char *fileName );
void Resize(int x, int y);
//backgrounds


typedef struct  
{
	double x;
	double y;
	double z;
}world_coords;

typedef struct  
{
	int x;
	int y;
}screenspace_coords;

world_coords GetWorldCoords(int x, int y);
screenspace_coords GetScreenCoords(double fx, double fy, double fz);
// Sets an orthographic projection. x: 0..width, y: 0..height 
void BeginOrtho2D(float width, float height, bool flip_y = false);
// Sets an orthographic projection. x: x1..x2, y: y1..y2 
void BeginOrtho2D(float x1, float y1, float x2, float y2) ;
// Sets a perspective projection with a different field-of-view 
void BeginPerspective(float fovy, float xratio) ;
void DrawFullScreenQuad( int iWidth, int iHeight );
// Restore the previous projection 
void EndProjection() ;

struct FBOELEM {
	GLuint fbo;
	GLuint depthbuffer;
	GLuint texture;
	GLuint depthtexture;
	GLint status;
};


typedef struct  
{
	float x,y,z,speed, rot,rotspeed;
	int xsize,ysize;
	GLuint texture;
	float rcol,gcol,bcol,acol;
}sprite;
void draw_sprite(sprite spr, int xres, int yres, bool flip_y = false);


#endif
