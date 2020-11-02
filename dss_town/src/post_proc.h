#include "sys/msys.h"
#include "intro.h"

struct FBOELEM {
    GLuint fbo;
    GLuint depthbuffer;
    GLuint texture;
	GLuint depthtexture;
    GLint status;
};



void start_fbo(GLuint fbo, int width, int height)
{
	oglBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fbo);
	glPushAttrib(GL_VIEWPORT_BIT);
	glViewport(0,0,width,height);
}

void end_fbo()
{
	glPopAttrib();
	oglBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
}

void draw_fbtexture(GLuint texture,int width, int height )
{
	glClear (GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);		// Clear Screen And Depth Buffer
	glEnable(GL_TEXTURE_2D);
	BeginOrtho2D(width,height,true);
	glBindTexture( GL_TEXTURE_2D,  texture);
	glTexParameteri (GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri (GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	DrawFullScreenQuad(width,height);
	glBindTexture( GL_TEXTURE_2D,  0);
	EndProjection();
}

int init_fbo ( int count, struct FBOELEM *buffers, int width, int height ) {
	// creates count nummers of Frame/Depthbuffers with a corresponding texture
	// buffers have to be allocated.
	int current, enderr = 1;
	GLenum error;

	for (current = 0; current < count; current++) {
		oglGenFramebuffersEXT (1, &buffers[current].fbo);
		oglBindFramebufferEXT (GL_FRAMEBUFFER_EXT, buffers[current].fbo);
		oglGenRenderbuffersEXT (1, &buffers[current].depthbuffer);
		oglBindRenderbufferEXT (GL_RENDERBUFFER_EXT, buffers[current].depthbuffer);
		oglRenderbufferStorageEXT (GL_RENDERBUFFER_EXT, GL_DEPTH_COMPONENT, width, height);
		oglFramebufferRenderbufferEXT (GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GL_RENDERBUFFER_EXT, buffers[current].depthbuffer);
		glGenTextures (1, &buffers[current].texture);
		glBindTexture (GL_TEXTURE_2D, buffers[current].texture);
		glTexParameteri (GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri (GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexImage2D (GL_TEXTURE_2D, 0, GL_RGBA8,  width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
		oglFramebufferTexture2DEXT (GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, buffers[current].texture, 0);
		// check if everything was ok with our requests above.
		error = oglCheckFramebufferStatusEXT (GL_FRAMEBUFFER_EXT);
		if (error != GL_FRAMEBUFFER_COMPLETE_EXT) {
			buffers[current].status = 0;
			enderr = 0;
		}
		else
			buffers[current].status = 1;
	}
	// set Rendering Device to screen again.
	oglBindFramebufferEXT (GL_FRAMEBUFFER_EXT, 0);
	return (enderr);
}


void init_fbo(GLuint texture,GLuint fbo, GLuint rbo, int width, int height)
{
	unsigned char* texturedata = new unsigned char[4 * width * height];
	memset(texturedata,0,4*XRES*YRES);
	//texture
	glEnable(GL_TEXTURE_2D);
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D,  texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA,GL_UNSIGNED_BYTE, texturedata);
	oglGenFramebuffersEXT(1, &fbo);
	oglBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fbo);
	oglGenRenderbuffersEXT(1, &rbo);
	oglBindRenderbufferEXT(GL_RENDERBUFFER_EXT, rbo);
	oglRenderbufferStorageEXT(GL_RENDERBUFFER_EXT, GL_DEPTH_COMPONENT, width, height);
	oglFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, texture, 0);
	oglFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GL_RENDERBUFFER_EXT, rbo);
	oglBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
	glGetError();
	glBindTexture(GL_TEXTURE_2D,  0);
	delete [] texturedata;
}