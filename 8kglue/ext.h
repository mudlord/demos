//--------------------------------------------------------------------------//
// iq / rgba  .  tiny codes  .  2008/2015                                   //
//--------------------------------------------------------------------------//

#ifndef _EXTENSIONES_H_
#define _EXTENSIONES_H_

#ifdef WINDOWS
#define WIN32_LEAN_AND_MEAN
#define WIN32_EXTRA_LEAN
#include <windows.h>
#endif
#include <GL/gl.h>
#include "glext.h"


#ifdef DEBUG
#define NUMFUNCIONES 10
#else
#define NUMFUNCIONES 8
#endif

extern void *myglfunc[NUMFUNCIONES];



#define oglCreateShaderProgramv         ((PFNGLCREATESHADERPROGRAMVPROC)myglfunc[0])
#define oglGenProgramPipelines          ((PFNGLGENPROGRAMPIPELINESPROC)myglfunc[1])
#define oglBindProgramPipeline          ((PFNGLBINDPROGRAMPIPELINEPROC)myglfunc[2])
#define oglUseProgramStages             ((PFNGLUSEPROGRAMSTAGESPROC)myglfunc[3])
#define oglProgramUniform4fv            ((PFNGLPROGRAMUNIFORM4FVPROC)myglfunc[4])
#define oglProgramUniform1i              ((PFNGLPROGRAMUNIFORM1IPROC)myglfunc[5])
#define oglActiveTextureARB            ((PFNGLACTIVETEXTUREARBPROC)myglfunc[6])
#define oglMultiTexCoord2fARB         ((PFNGLMULTITEXCOORD2FARBPROC)myglfunc[7])

#ifdef DEBUG
#define oglGetProgramiv          ((PFNGLGETPROGRAMIVPROC)myglfunc[8])
#define oglGetProgramInfoLog     ((PFNGLGETPROGRAMINFOLOGPROC)myglfunc[9])
#endif

// init
int EXT_Init( void );

#endif
