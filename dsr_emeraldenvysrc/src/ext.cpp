//--------------------------------------------------------------------------//
// iq / rgba  .  tiny codes  .  2008                                        //
//--------------------------------------------------------------------------//

#ifdef WINDOWS
#define WIN32_LEAN_AND_MEAN
#define WIN32_EXTRA_LEAN
#include <windows.h>
#endif
#include <GL/gl.h>
#include "glext.h"
#ifdef LINUX
#include <GL/glx.h>
#endif

//--- d a t a ---------------------------------------------------------------

#include "ext.h"

static char *funciones = {
	// multitexture
	"glActiveTextureARB\x0"
	"glClientActiveTextureARB\x0"
	"glMultiTexCoord2fARB\x0"
	// programs
	"glDeleteProgramsARB\x0"
	"glBindProgramARB\x0"
	"glProgramStringARB\x0"
	"glProgramLocalParameter4fvARB\x0"
	"glProgramEnvParameter4fvARB\x0"
	// textures 3d
	"glTexImage3D\x0"
	// vbo-ibo
	"glBindBufferARB\x0"
	"glBufferDataARB\x0"
	"glBufferSubDataARB\x0"
	"glDeleteBuffersARB\x0"

	// shader
	"glCreateProgram\x0"
	"glCreateShader\x0"
	"glShaderSource\x0"
	"glCompileShader\x0"
	"glAttachShader\x0"
	"glLinkProgram\x0"
	"glUseProgram\x0"
	"glUniform4fv\x0"
	"glUniform1i\x0"
	"glGetUniformLocationARB\x0"
	"glGetObjectParameterivARB\x0"
	"glGetInfoLogARB\x0"

	"glLoadTransposeMatrixf\x0"

	"glBindRenderbufferEXT\x0"
	"glDeleteRenderbuffersEXT\x0"
	"glRenderbufferStorageEXT\x0"
	"glBindFramebufferEXT\x0"
	"glDeleteFramebuffersEXT\x0"
	"glCheckFramebufferStatusEXT\x0"
	"glFramebufferTexture1DEXT\x0"
	"glFramebufferTexture2DEXT\x0"
	"glFramebufferTexture3DEXT\x0"
	"glFramebufferRenderbufferEXT\x0"
	"glGenerateMipmapEXT\x0"
	"glCompressedTexImage2DARB\x0"
	"wglSwapIntervalEXT\x0"
	"glGenFramebuffersEXT\x0"
	"glGenRenderbuffersEXT\x0"
	"glUniform1fv\x0"
	"glUniform2fv\x0"
	"glBlendFuncSeparateEXT\x0"
};

void *msys_oglfunc[NUMFUNCIONES];

//--- c o d e ---------------------------------------------------------------

int EXT_Init(void)
{
	char *str = funciones;
	for (int i = 0; i < NUMFUNCIONES; i++)
	{
		msys_oglfunc[i] = wglGetProcAddress( str );
		str += 1+strlen( str );

		if (!msys_oglfunc[i])
			return(0);
}
	return(1);
}

world_coords GetWorldCoords(int x, int y)
{
	world_coords coords;
	double objX, objY, objZ;//holder for world coordinates
	GLint view[4];//viewport dimensions+pos
	GLdouble  p[16];//projection matrix
	GLdouble  m[16];//modelview matrix
	GLdouble z = 10.0;//Z-Buffer Value?
	glGetDoublev(GL_MODELVIEW_MATRIX, m);
	glGetDoublev(GL_PROJECTION_MATRIX, p);
	glGetIntegerv(GL_VIEWPORT, view);
	//view[3]-cursorY = conversion from upper left (0,0) to lower left (0,0)
	//Unproject 2D Screen coordinates into wonderful world coordinates
	gluUnProject(x, view[3] - y, 1, m, p, view, &objX, &objY, &objZ);
	objX /= z;
	objY /= z;

	coords.x = objX;
	coords.y = objY;
	return coords;
}

screenspace_coords GetScreenCoords(double fx, double fy, double fz)
{
	screenspace_coords coords;
	GLdouble x, y;
	GLint view[4];//viewport dimensions+pos
	GLdouble  p[16];//projection matrix
	GLdouble  m[16];//modelview matrix
	GLdouble z = 10.0;//Z-Buffer Value?
	glGetDoublev(GL_MODELVIEW_MATRIX, m);
	glGetDoublev(GL_PROJECTION_MATRIX, p);
	glGetIntegerv(GL_VIEWPORT, view);

	gluProject(fx, fy, fz,
		m, p, view,
		&x, &y, &z);
	x /= z;
	y /= z;
	coords.x = (int)x;
	coords.y = (int)y;

	return coords;
}

void BeginOrtho2D(float width, float height, bool flip_y)
{
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	if (!flip_y)
	{
		gluOrtho2D(0, width, 0, height);
	}
	else
	{
		glOrtho(0, width, height, 0, -1, 1);
	}


	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
}

// Sets an orthographic projection. x: x1..x2, y: y1..y2 
void BeginOrtho2D(float x1, float y1, float x2, float y2)
{
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	gluOrtho2D(x1, x2, y1, y2);
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
}

void BeginPerspective(float fovy, float xratio)
{
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	gluPerspective(fovy, xratio, 0.1f, 100.0f);
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
}

// Restore the previous projection 
void EndProjection()
{
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
}

void Resize(int x, int y)
{
	// Prevent div by zero 
	y = y ? y : 1;

	// Resize viewport 
	glViewport(0, 0, x, y);

	// Recalc perspective 
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(60.0f, static_cast<float>(x) / static_cast<float>(y), 0.1f, 100.0f);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}


void start_fbo(GLuint fbo, int width, int height, bool twod)
{
	oglBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fbo);
	glPushAttrib(GL_VIEWPORT_BIT);
	glViewport(0, 0, width, height);
}

void end_fbo()
{
	glPopAttrib();
	oglBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
}

void DrawFullScreenQuad(int iWidth, int iHeight)
{
	glBegin(GL_QUADS);
	// Display the top left point of the 2D image
	glTexCoord2f(0.0f, 1.0f);	glVertex2f(0, 0);
	// Display the bottom left point of the 2D image
	//glTexCoord2f(0.0f, 0.0f);	glVertex2f(0, SCREEN_HEIGHT);
	glTexCoord2f(0.0f, 0.0f);	glVertex2f(0, iHeight);
	// Display the bottom right point of the 2D image
	//glTexCoord2f(1.0f, 0.0f);	glVertex2f(SCREEN_WIDTH, SCREEN_HEIGHT);
	glTexCoord2f(1.0f, 0.0f);	glVertex2f(iWidth, iHeight);
	// Display the top right point of the 2D image
	//glTexCoord2f(1.0f, 1.0f);	glVertex2f(SCREEN_WIDTH, 0);
	glTexCoord2f(1.0f, 1.0f);	glVertex2f(iWidth, 0);
	// Stop drawing 
	glEnd();
}


void draw_fbotexture(GLuint texture, int width, int height)
{
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);		// Clear Screen And Depth Buffer
	glEnable(GL_TEXTURE_2D);
	glDisable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	BeginOrtho2D(width, height, true);
	glBindTexture(GL_TEXTURE_2D, texture);
	DrawFullScreenQuad(width, height);
	glBindTexture(GL_TEXTURE_2D, 0);
	glDisable(GL_BLEND);
	glEnable(GL_DEPTH_TEST);
	EndProjection();
}

FBOELEM init_fbo(int width, int height)
{
	FBOELEM elem = { 0 };
	int current, enderr = 1;
	GLuint error = 0;
	oglGenFramebuffersEXT(1, &elem.fbo);
	oglBindFramebufferEXT(GL_FRAMEBUFFER_EXT, elem.fbo);
	oglGenRenderbuffersEXT(1, &elem.depthbuffer);
	oglBindRenderbufferEXT(GL_RENDERBUFFER_EXT, elem.depthbuffer);
	oglRenderbufferStorageEXT(GL_RENDERBUFFER_EXT, GL_DEPTH_COMPONENT, width, height);
	oglFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GL_RENDERBUFFER_EXT, elem.depthbuffer);
	glGenTextures(1, &elem.texture);
	glBindTexture(GL_TEXTURE_2D, elem.texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	oglFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, elem.texture, 0);

	// check if everything was ok with our requests above.
	error = oglCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);
	if (error != GL_FRAMEBUFFER_COMPLETE_EXT) {
		FBOELEM err = { 0 };
		elem.status = 0;
		enderr = 0;
		return err;
	}
	elem.status = 1;
	// set Rendering Device to screen again.
	glBindTexture(GL_TEXTURE_2D, 0);
	oglBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
	oglBindRenderbufferEXT(GL_RENDERBUFFER_EXT, 0);
	return elem;
}

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