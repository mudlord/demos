//--------------------------------------------------------------------------//
// iq / rgba  .  tiny codes  .  2008/2015                                   //
//--------------------------------------------------------------------------//

#define WIN32_LEAN_AND_MEAN
#define WIN32_EXTRA_LEAN
#include <windows.h>
#include <GL/gl.h>
#include <math.h>
#include "main.h"
#include "ext.h"
#include "shader.inl"
#include "fp.h"

//=================================================================================================================
typedef struct
{
	int fsid;
	int vsid;
	unsigned int pid;
}shader_id;

GLuint vhs_texture;

shader_id shader_init(const char *vsh, const char *fsh)
{
	shader_id shad = { 0 };
	shad.vsid = oglCreateShaderProgramv(GL_VERTEX_SHADER, 1,&vsh);
	shad.fsid = oglCreateShaderProgramv(GL_FRAGMENT_SHADER, 1, &fsh);
	oglGenProgramPipelines(1, &shad.pid);
	oglBindProgramPipeline(shad.pid);
	oglUseProgramStages(shad.pid, GL_VERTEX_SHADER_BIT, shad.vsid);
	oglUseProgramStages(shad.pid, GL_FRAGMENT_SHADER_BIT, shad.fsid);
#ifdef DEBUG
	int		result;
	char    info[1536];
	oglGetProgramiv( shad.vsid, GL_LINK_STATUS, &result); oglGetProgramInfoLog( shad.vsid, 1024, NULL, (char *)info); if (!result) DebugBreak();
	oglGetProgramiv( shad.fsid, GL_LINK_STATUS, &result); oglGetProgramInfoLog( shad.fsid, 1024, NULL, (char *)info); if (!result) DebugBreak();
	oglGetProgramiv( shad.pid, GL_LINK_STATUS, &result); oglGetProgramInfoLog( shad.pid, 1024, NULL, (char *)info); if (!result) DebugBreak();
#endif
	oglBindProgramPipeline(0);
	return shad;
}
shader_id raymarch;
shader_id vhs_shader;

GLuint init_rendertexture(int resx, int resy)
{
	GLuint texture;
	//texture
	glEnable(GL_TEXTURE_2D);
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, resx, resy, 0, GL_RGBA,
		GL_UNSIGNED_BYTE, NULL);
	glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 0, 0, resx, resy, 0);
	glBindTexture(GL_TEXTURE_2D, 0);
	return texture;
}

int intro_init( void )
{
    if( !EXT_Init() )
        return 0;
	raymarch = shader_init(vsh, scene1_shader);
	vhs_texture = init_rendertexture(XRES, YRES);
	vhs_shader = shader_init(vsh_texture, texture_shader);
    return 1;
}

//=================================================================================================================

static float fparams[4*4];

void intro_do( long time )
{
    //--- update parameters -----------------------------------------

    const float t  = 0.001f*(float)time;

    // camera position
    fparams[ 0] = (float)XRES;
    fparams[ 1] = (float)YRES;
    fparams[ 2] = t;
    // camera target
    fparams[ 4] = 0.0f;
    fparams[ 5] = 0.0f;
    fparams[ 6] = 0.0f;
    // sphere
    fparams[ 8] = 0.0f;
    fparams[ 9] = 0.0f;
    fparams[10] = 0.0f;
    fparams[11] = 1.0f;

    //--- render -----------------------------------------
	oglBindProgramPipeline(raymarch.pid);
	oglProgramUniform4fv(raymarch.fsid, 0, 4, fparams);
    glRects( -1, -1, 1, 1 );
	oglBindProgramPipeline(0);

	glBindTexture(GL_TEXTURE_2D, vhs_texture);
	glCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 0, 0, XRES, YRES);
	glBindTexture(GL_TEXTURE_2D, 0);

	glClear(GL_COLOR_BUFFER_BIT);
	glEnable(GL_TEXTURE_2D);
	oglBindProgramPipeline(vhs_shader.pid);
	glBindTexture(GL_TEXTURE_2D, vhs_texture);
	oglProgramUniform4fv(vhs_shader.fsid, 0, 0, fparams);
	oglProgramUniform1i(vhs_shader.fsid, 4, 0);
	
	
	glBegin(GL_TRIANGLE_FAN);
	glTexCoord2f(0, 0);  glVertex2f(-1, 1);
	glTexCoord2f(1, 0);  glVertex2f(1, 1);
	glTexCoord2f(1, 1);  glVertex2f(1, -1);
	glTexCoord2f(0, 1);  glVertex2f(-1, -1);
	glEnd();

	oglBindProgramPipeline(0);
}