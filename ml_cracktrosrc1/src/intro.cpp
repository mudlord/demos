#include "sys/msys.h"
#include "intro.h"

#include "background.h"
#include "starbg.h"
#include "colborders.h"
#include "fontwriter.h"
#include "ufmod.h"
#include "ptbuddy.c"
#include "music.h"

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



int intro_init( int xr, int yr, int nomusic, IntroProgressDelegate *pd )
{
	Resize(XRES,YRES);

	srand(NULL);
    // progress report, (from 0 to 200)
    pd->func( pd->obj, 0 );
	bg_init();
	srand(NULL);
	fontwriter_init();
	stars_init();
    pd->func( pd->obj, 1.0 );
//	playptmod_Play();
	//HWAVEOUT *res = uFMOD_PlaySong((void*)music,(void*)sizeof(music), XM_MEMORY);
	//if (res == NULL) return 0;
	ptBuddyPlay(music, 0, 44100);
    return 1;
}

void intro_end()
{
	
}




int intro_do( void )
{
	static long lastTime = timeGetTime();
	long currTime = timeGetTime();
	static double delta = 0.0;
	long diff= currTime - lastTime;
	//delta = delta*0.5 + (diff/20.0)*0.5;
	delta = diff;
	lastTime = currTime;
	static float sceneTime = 0;
	sceneTime += (float)delta/1000.f;
	static float texttime = 0;
	texttime += (float)delta/1000.f;

	//shader rendering
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	stars_render(sceneTime);
	stars_draw();
	
	glScissor(1	,int(0.075f*YRES),XRES-2,int(0.85f*YRES));
	glEnable(GL_SCISSOR_TEST); 
	//shader timeline
	bg_do(sceneTime);
	
	glDisable(GL_SCISSOR_TEST);
	drawColBorders ();
	fontwrite(delta);



	return 0;

}