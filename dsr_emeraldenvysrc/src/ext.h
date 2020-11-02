//--------------------------------------------------------------------------//
// iq / rgba  .  tiny codes  .  2008                                        //
//--------------------------------------------------------------------------//

#ifndef _EXTENSIONES_H_
#define _EXTENSIONES_H_

#ifdef WINDOWS
#define WIN32_LEAN_AND_MEAN
#define WIN32_EXTRA_LEAN
#include <windows.h>
#endif
#include <GL/gl.h>
#include <gl/GLU.h>
#include "glext.h"
#include "wglext.h"


#define NUMFUNCIONES (44)

extern void *msys_oglfunc[NUMFUNCIONES];

// multitexture
#define oglActiveTextureARB            ((PFNGLACTIVETEXTUREARBPROC)msys_oglfunc[0])
#define oglClientActiveTextureARB      ((PFNGLCLIENTACTIVETEXTUREARBPROC)msys_oglfunc[1])
#define oglMultiTexCoord2fARB         ((PFNGLMULTITEXCOORD2FARBPROC)msys_oglfunc[2])
// programs
#define oglDeleteProgramsARB           ((PFNGLDELETEPROGRAMSARBPROC)msys_oglfunc[3])
#define oglBindProgramARB              ((PFNGLBINDPROGRAMARBPROC)msys_oglfunc[4])
#define oglProgramStringARB            ((PFNGLPROGRAMSTRINGARBPROC)msys_oglfunc[5])
#define oglProgramLocalParameter4fvARB ((PFNGLPROGRAMLOCALPARAMETER4FVARBPROC)msys_oglfunc[6])
#define oglProgramEnvParameter4fvARB   ((PFNGLPROGRAMENVPARAMETER4FVARBPROC)msys_oglfunc[7])
// textures 3d
#define oglTexImage3D                  ((PFNGLTEXIMAGE3DPROC)msys_oglfunc[8])
// vbo-ibo
#define oglBindBufferARB               ((PFNGLBINDBUFFERARBPROC)msys_oglfunc[9])
#define oglBufferDataARB               ((PFNGLBUFFERDATAARBPROC)msys_oglfunc[10])
#define oglBufferSubDataARB            ((PFNGLBUFFERSUBDATAARBPROC)msys_oglfunc[11])
#define oglDeleteBuffersARB            ((PFNGLDELETEBUFFERSARBPROC)msys_oglfunc[12])

// shaders
#define oglCreateProgram	           ((PFNGLCREATEPROGRAMPROC)msys_oglfunc[13])
#define oglCreateShader		           ((PFNGLCREATESHADERPROC)msys_oglfunc[14])
#define oglShaderSource                ((PFNGLSHADERSOURCEPROC)msys_oglfunc[15])
#define oglCompileShader               ((PFNGLCOMPILESHADERPROC)msys_oglfunc[16])
#define oglAttachShader                ((PFNGLATTACHSHADERPROC)msys_oglfunc[17])
#define oglLinkProgram                 ((PFNGLLINKPROGRAMPROC)msys_oglfunc[18])
#define oglUseProgram                  ((PFNGLUSEPROGRAMPROC)msys_oglfunc[19])
#define oglUniform4fv                  ((PFNGLUNIFORM4FVPROC)msys_oglfunc[20])
#define oglUniform1i                   ((PFNGLUNIFORM1IPROC)msys_oglfunc[21])
#define oglGetUniformLocation          ((PFNGLGETUNIFORMLOCATIONARBPROC)msys_oglfunc[22])
#define oglGetObjectParameteriv        ((PFNGLGETOBJECTPARAMETERIVARBPROC)msys_oglfunc[23])
#define oglGetInfoLog                  ((PFNGLGETINFOLOGARBPROC)msys_oglfunc[24])

#define oglLoadTransposeMatrixf        ((PFNGLLOADTRANSPOSEMATRIXFPROC)msys_oglfunc[25])

// fbo
#define oglBindRenderbufferEXT         ((PFNGLBINDRENDERBUFFEREXTPROC)msys_oglfunc[26])
#define oglDeleteRenderbuffersEXT      ((PFNGLDELETERENDERBUFFERSEXTPROC)msys_oglfunc[27])
#define oglRenderbufferStorageEXT      ((PFNGLRENDERBUFFERSTORAGEEXTPROC)msys_oglfunc[28])
#define oglBindFramebufferEXT          ((PFNGLBINDFRAMEBUFFEREXTPROC)msys_oglfunc[29])
#define oglDeleteFramebuffersEXT       ((PFNGLDELETEFRAMEBUFFERSEXTPROC)msys_oglfunc[30])
#define oglCheckFramebufferStatusEXT   ((PFNGLCHECKFRAMEBUFFERSTATUSEXTPROC)msys_oglfunc[31])
#define oglFramebufferTexture1DEXT     ((PFNGLFRAMEBUFFERTEXTURE1DEXTPROC)msys_oglfunc[32])
#define oglFramebufferTexture2DEXT     ((PFNGLFRAMEBUFFERTEXTURE2DEXTPROC)msys_oglfunc[33])
#define oglFramebufferTexture3DEXT     ((PFNGLFRAMEBUFFERTEXTURE3DEXTPROC)msys_oglfunc[34])
#define oglFramebufferRenderbufferEXT  ((PFNGLFRAMEBUFFERRENDERBUFFEREXTPROC)msys_oglfunc[35])
#define oglGenerateMipmapEXT           ((PFNGLGENERATEMIPMAPEXTPROC)msys_oglfunc[36])
#define glCompressedTexImage2DARB  ((PFNGLCOMPRESSEDTEXIMAGE2DARBPROC)msys_oglfunc[37])
#define owglswapinterval ((PFNWGLSWAPINTERVALEXTPROC)msys_oglfunc[38])
#define oglGenFramebuffersEXT ((PFNGLGENFRAMEBUFFERSEXTPROC)msys_oglfunc[39])
#define oglGenRenderbuffersEXT ((PFNGLGENRENDERBUFFERSEXTPROC)msys_oglfunc[40])
#define oglUniform1fv ((PFNGLUNIFORM1FVPROC)msys_oglfunc[41])
#define oglUniform2fv ((PFNGLUNIFORM2FVPROC)msys_oglfunc[42])

#define oglBlendFuncSeperate ((PFNGLBLENDFUNCSEPARATEEXTPROC)msys_oglfunc[43])

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
void BeginOrtho2D(float x1, float y1, float x2, float y2);
// Sets a perspective projection with a different field-of-view 
void BeginPerspective(float fovy, float xratio);
void DrawFullScreenQuad(int iWidth, int iHeight);
// Restore the previous projection 
void EndProjection();
void Resize(int x, int y);

struct FBOELEM {
	GLuint fbo;
	GLuint depthbuffer;
	GLuint texture;
	GLuint depthtexture;
	GLint status;
};


void start_fbo(GLuint fbo, int width, int height, bool twod = false);
void end_fbo();
GLuint init_rendertexture(int resx, int resy);
FBOELEM init_fbo(int width, int height);
void draw_fbotexture(GLuint texture, int width, int height);

// init
int EXT_Init( void );

#endif
