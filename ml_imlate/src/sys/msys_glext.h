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
#include <GL/GL.h>
#include "glext.h"
#include <string.h>
#include <stdio.h>

#ifndef DEBUG
#define NUMFUNCS 58
extern void *msys_oglfunc[NUMFUNCS];
#else
#define NUMFUNCS 60
extern void *msys_oglfunc[NUMFUNCS];
#endif


#define glActiveTexture		 ((PFNGLACTIVETEXTUREPROC)msys_oglfunc[0])
#define glCompressedTexImage2D		 ((PFNGLCOMPRESSEDTEXIMAGE2DPROC)msys_oglfunc[1])
#define glCompressedTexSubImage2D		 ((PFNGLCOMPRESSEDTEXSUBIMAGE2DPROC)msys_oglfunc[2])
#define glGetCompressedTexImage		  ((PFNGLGETCOMPRESSEDTEXIMAGEPROC)msys_oglfunc[3]) 
#define glBlendFuncSeparate		 ((PFNGLBLENDFUNCSEPARATEPROC)msys_oglfunc[4])
#define glBindBuffer		 ((PFNGLBINDBUFFERPROC)msys_oglfunc[5])
#define glDeleteBuffers		 ((PFNGLDELETEBUFFERSPROC)msys_oglfunc[6])
#define glGenBuffers		((PFNGLGENBUFFERSPROC)msys_oglfunc[7])
#define glIsBuffer		 ((PFNGLISBUFFERPROC)msys_oglfunc[8])
#define glBufferData		 ((PFNGLBUFFERDATAPROC)msys_oglfunc[9])
#define glBufferSubData		 ((PFNGLBUFFERSUBDATAPROC)msys_oglfunc[10])
#define glGetBufferSubData		((PFNGLGETBUFFERSUBDATAPROC)msys_oglfunc[11])
#define glMapBuffer		 ((PFNGLMAPBUFFERPROC)msys_oglfunc[12])
#define glUnmapBuffer		((PFNGLUNMAPBUFFERPROC)msys_oglfunc[13])
#define glGetBufferParameteriv		 ((PFNGLGETBUFFERPARAMETERIVPROC)msys_oglfunc[14])
#define glGetBufferPointerv		 ((PFNGLGETBUFFERPOINTERVPROC)msys_oglfunc[15])
#define glBlendEquationSeparate		 ((PFNGLBLENDEQUATIONSEPARATEPROC)msys_oglfunc[16])
#define glDrawBuffers		 ((PFNGLDRAWBUFFERSPROC)msys_oglfunc[17])
#define glStencilOpSeparate		 ((PFNGLSTENCILOPSEPARATEPROC)msys_oglfunc[18])
#define glStencilFuncSeparate		 ((PFNGLSTENCILFUNCSEPARATEPROC)msys_oglfunc[19])
#define glStencilMaskSeparate		 ((PFNGLSTENCILMASKSEPARATEPROC)msys_oglfunc[20])
#define glBindAttribLocation		 ((PFNGLBINDATTRIBLOCATIONPROC)msys_oglfunc[21])
#define glDisableVertexAttribArray		 ((PFNGLDISABLEVERTEXATTRIBARRAYPROC)msys_oglfunc[22])
#define glEnableVertexAttribArray		 ((PFNGLENABLEVERTEXATTRIBARRAYPROC)msys_oglfunc[23])
#define glGetAttribLocation		((PFNGLGETATTRIBLOCATIONPROC)msys_oglfunc[24])
#define glGetUniformLocation		((PFNGLGETUNIFORMLOCATIONPROC)msys_oglfunc[25])
#define glVertexAttribPointer		((PFNGLVERTEXATTRIBPOINTERPROC)msys_oglfunc[26])
#define glClampColor		((PFNGLCLAMPCOLORPROC)msys_oglfunc[27])
#define glBindFragDataLocation		((PFNGLBINDFRAGDATALOCATIONPROC)msys_oglfunc[28])
#define glGetFragDataLocation		((PFNGLGETFRAGDATALOCATIONPROC)msys_oglfunc[29])
#define glFramebufferTexture		((PFNGLFRAMEBUFFERTEXTUREPROC)msys_oglfunc[30])
#define glIsRenderbuffer		((PFNGLISRENDERBUFFERPROC)msys_oglfunc[31])
#define glBindRenderbuffer		((PFNGLBINDRENDERBUFFERPROC)msys_oglfunc[32])
#define glDeleteRenderbuffers		((PFNGLDELETERENDERBUFFERSPROC)msys_oglfunc[33])
#define glGenRenderbuffers		((PFNGLGENRENDERBUFFERSPROC)msys_oglfunc[34])
#define glRenderbufferStorage		((PFNGLRENDERBUFFERSTORAGEPROC)msys_oglfunc[35])
#define glGetRenderbufferParameteriv		((PFNGLGETRENDERBUFFERPARAMETERIVPROC)msys_oglfunc[36])
#define glIsFramebuffer		((PFNGLISFRAMEBUFFERPROC)msys_oglfunc[37])
#define glBindFramebuffer		((PFNGLBINDFRAMEBUFFERPROC)msys_oglfunc[38])
#define glDeleteFramebuffers		((PFNGLDELETEFRAMEBUFFERSPROC)msys_oglfunc[39])
#define glGenFramebuffers		((PFNGLGENFRAMEBUFFERSPROC)msys_oglfunc[40])
#define glCheckFramebufferStatus		((PFNGLCHECKFRAMEBUFFERSTATUSPROC)msys_oglfunc[41])
#define glFramebufferTexture2D		((PFNGLFRAMEBUFFERTEXTURE2DPROC)msys_oglfunc[42])
#define glFramebufferRenderbuffer		((PFNGLFRAMEBUFFERRENDERBUFFERPROC)msys_oglfunc[43])
#define glGetFramebufferAttachmentParameteriv		((PFNGLGETFRAMEBUFFERATTACHMENTPARAMETERIVPROC)msys_oglfunc[44])
#define glGenerateMipmap		((PFNGLGENERATEMIPMAPPROC)msys_oglfunc[45])
#define glBlitFramebuffer		((PFNGLBLITFRAMEBUFFERPROC)msys_oglfunc[46])
#define glBindVertexArray		((PFNGLBINDVERTEXARRAYPROC)msys_oglfunc[47])
#define glDeleteVertexArrays		((PFNGLDELETEVERTEXARRAYSPROC)msys_oglfunc[48])
#define glGenVertexArrays		((PFNGLGENVERTEXARRAYSPROC)msys_oglfunc[49])

#define glCreateShaderProgramv         ((PFNGLCREATESHADERPROGRAMVPROC)msys_oglfunc[50])
#define glGenProgramPipelines          ((PFNGLGENPROGRAMPIPELINESPROC)msys_oglfunc[51])
#define glBindProgramPipeline          ((PFNGLBINDPROGRAMPIPELINEPROC)msys_oglfunc[52])
#define glUseProgramStages             ((PFNGLUSEPROGRAMSTAGESPROC)msys_oglfunc[53])
#define glProgramUniform4fv            ((PFNGLPROGRAMUNIFORM4FVPROC)msys_oglfunc[54])
#define glProgramUniform1i              ((PFNGLPROGRAMUNIFORM1IPROC)msys_oglfunc[55])
#define glProgramUniformMatrix4fv       ((PFNGLPROGRAMUNIFORMMATRIX4FVPROC)msys_oglfunc[56])
#define glGetProgramResourceLocation    ((PFNGLGETPROGRAMRESOURCELOCATIONPROC)msys_oglfunc[57])
#ifdef DEBUG
#define glGetProgramiv          ((PFNGLGETPROGRAMIVPROC)msys_oglfunc[58])
#define glGetProgramInfoLog     ((PFNGLGETPROGRAMINFOLOGPROC)msys_oglfunc[59])
#endif

struct shader_id{
	int fsid;
	int vsid;
	unsigned int pid;
};

struct FBOELEM{
	GLuint fbo;
	GLuint depthbuffer;
	GLuint texture;
	GLuint depthtexture;
	GLint status;
};

#ifdef __cplusplus
extern "C" {
#endif



// init
int msys_glextInit( void );
struct shader_id initShader(const char *vsh, const char *fsh);
void Resize(int x, int y);
unsigned char *LoadImageMemory(unsigned char* data, int size, int * width, int * height);
GLuint loadTexMemory(unsigned char* data, int size, int * width, int * height, int blur);
struct FBOELEM init_fbo(int width, int height, BOOL shit);
#ifdef __cplusplus
}
#endif

#endif
