#include "sys/msys.h"
#include "intro.h"
#include <sys/stat.h>
//data
#include "trollface.h"
#include "creditstex.h"
#include "vertexbuffer.h"
#include "torusknot.h"
GLuint credits_bg;
GLuint cube_creditstex;
GLuint trollface_tex;


int init_pqknot_cube()
{
	glEnable(GL_TEXTURE_2D);
	glGenTextures(1, &cube_creditstex);
	glBindTexture(GL_TEXTURE_2D,  cube_creditstex);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA8,0,XRES,YRES,GL_RGB,GL_UNSIGNED_BYTE, 0);
	glCopyTexImage2D(GL_TEXTURE_2D,0,GL_RGB,0,0,XRES,YRES,0);
	glBindTexture(GL_TEXTURE_2D,0);
	return true;
}

void credits_open()
{
	credits_bg = loadTexGenTexMemory(creditstex,creditstex_len,512,512);
	trollface_tex = loadDDSTextureMemory(trollface,trollface_len,false);
	init_pqknot_cube();
}

void render_pqknot(float time)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);	// Clear Screen And Depth Buffer
	glLoadIdentity();		
	// Reset The Current Modelview Matrix
	//bg
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D,trollface_tex);
	DrawZooming(sin(time/2));
	glBindTexture(GL_TEXTURE_2D,0);
	glPushMatrix();
	float angle = time/10;
	VertexBuffer *pq = generate_torusknot(
		256,
		16,
		(float)(1.5f + sin(angle) / 2),
		0.2f,
		12,
		angle * 30,
		0.5f,
		4,
		64,
		4,
		5);
	glTranslatef( 0.0, 0.0, -10.0 );
	glRotatef( angle * 360, angle * 360, 1.0f, 0.2f);
	glBindTexture(GL_TEXTURE_2D,credits_bg);
	pq->render();
	pq->render_normals(0.1f);
	// render_normals messes up the vertex pointer, so we'll need to reactivate
	pq->activate(); 
	pq->render();
	glBindTexture(GL_TEXTURE_2D,0);
	glPopMatrix();
	delete pq;
	glBindTexture( GL_TEXTURE_2D,  cube_creditstex);
	glCopyTexSubImage2D(GL_TEXTURE_2D,0,0,0,0,0,XRES,YRES);
	glBindTexture(GL_TEXTURE_2D,0);
	glDisable(GL_TEXTURE_2D);
}

void render_cube()
{
	static GLfloat	xrot = 0;				// X Rotation ( NEW )
	static GLfloat	yrot= 0;				// Y Rotation ( NEW )
	static GLfloat	zrot= 0;				// Z Rotation ( NEW )
	glPushMatrix();
	glLoadIdentity();                           // Reset The Current Matrix
	glTranslatef(0.0f,0.0f,-5.0f);                      // Move Into The Screen 5 Units
	glRotatef(xrot,1.0f,0.0f,0.0f);                     // Rotate On The X Axis
	glRotatef(yrot,0.0f,1.0f,0.0f);                     // Rotate On The Y Axis
	glRotatef(zrot,0.0f,0.0f,1.0f);                     // Rotate On The Z Axis
	glEnable(GL_TEXTURE_2D);
	glBindTexture( GL_TEXTURE_2D,  cube_creditstex);

	glBegin(GL_QUADS);
	// Front Face
	glTexCoord2f(0.0f, 0.0f); glVertex3f(-1.0f, -1.0f,  1.0f);  // Bottom Left Of The Texture and Quad
	glTexCoord2f(1.0f, 0.0f); glVertex3f( 1.0f, -1.0f,  1.0f);  // Bottom Right Of The Texture and Quad
	glTexCoord2f(1.0f, 1.0f); glVertex3f( 1.0f,  1.0f,  1.0f);  // Top Right Of The Texture and Quad
	glTexCoord2f(0.0f, 1.0f); glVertex3f(-1.0f,  1.0f,  1.0f);  // Top Left Of The Texture and Quad
	// Back Face
	glTexCoord2f(1.0f, 0.0f); glVertex3f(-1.0f, -1.0f, -1.0f);  // Bottom Right Of The Texture and Quad
	glTexCoord2f(1.0f, 1.0f); glVertex3f(-1.0f,  1.0f, -1.0f);  // Top Right Of The Texture and Quad
	glTexCoord2f(0.0f, 1.0f); glVertex3f( 1.0f,  1.0f, -1.0f);  // Top Left Of The Texture and Quad
	glTexCoord2f(0.0f, 0.0f); glVertex3f( 1.0f, -1.0f, -1.0f);  // Bottom Left Of The Texture and Quad
	// Top Face
	glTexCoord2f(0.0f, 1.0f); glVertex3f(-1.0f,  1.0f, -1.0f);  // Top Left Of The Texture and Quad
	glTexCoord2f(0.0f, 0.0f); glVertex3f(-1.0f,  1.0f,  1.0f);  // Bottom Left Of The Texture and Quad
	glTexCoord2f(1.0f, 0.0f); glVertex3f( 1.0f,  1.0f,  1.0f);  // Bottom Right Of The Texture and Quad
	glTexCoord2f(1.0f, 1.0f); glVertex3f( 1.0f,  1.0f, -1.0f);  // Top Right Of The Texture and Quad
	// Bottom Face
	glTexCoord2f(1.0f, 1.0f); glVertex3f(-1.0f, -1.0f, -1.0f);  // Top Right Of The Texture and Quad
	glTexCoord2f(0.0f, 1.0f); glVertex3f( 1.0f, -1.0f, -1.0f);  // Top Left Of The Texture and Quad
	glTexCoord2f(0.0f, 0.0f); glVertex3f( 1.0f, -1.0f,  1.0f);  // Bottom Left Of The Texture and Quad
	glTexCoord2f(1.0f, 0.0f); glVertex3f(-1.0f, -1.0f,  1.0f);  // Bottom Right Of The Texture and Quad
	// Right face
	glTexCoord2f(1.0f, 0.0f); glVertex3f( 1.0f, -1.0f, -1.0f);  // Bottom Right Of The Texture and Quad
	glTexCoord2f(1.0f, 1.0f); glVertex3f( 1.0f,  1.0f, -1.0f);  // Top Right Of The Texture and Quad
	glTexCoord2f(0.0f, 1.0f); glVertex3f( 1.0f,  1.0f,  1.0f);  // Top Left Of The Texture and Quad
	glTexCoord2f(0.0f, 0.0f); glVertex3f( 1.0f, -1.0f,  1.0f);  // Bottom Left Of The Texture and Quad
	// Left Face
	glTexCoord2f(0.0f, 0.0f); glVertex3f(-1.0f, -1.0f, -1.0f);  // Bottom Left Of The Texture and Quad
	glTexCoord2f(1.0f, 0.0f); glVertex3f(-1.0f, -1.0f,  1.0f);  // Bottom Right Of The Texture and Quad
	glTexCoord2f(1.0f, 1.0f); glVertex3f(-1.0f,  1.0f,  1.0f);  // Top Right Of The Texture and Quad
	glTexCoord2f(0.0f, 1.0f); glVertex3f(-1.0f,  1.0f, -1.0f);  // Top Left Of The Texture and Quad
	glEnd();
	glPopMatrix();
	xrot+=0.3f;                             // X Axis Rotation
	yrot+=0.2f;                             // Y Axis Rotation
	zrot+=0.4f;                             // Z Axis Rotation
}

void credits_draw(float time)
{
	oglBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fbo);
	glPushAttrib(GL_VIEWPORT_BIT);
	glViewport(0,0,XRES,YRES);

	render_pqknot(time);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);	// Clear Screen And Depth Buffer
	glLoadIdentity();		
	// Reset The Current Modelview Matrix
	//bg
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D,credits_bg);
	DrawWaving(time);
	glBindTexture(GL_TEXTURE_2D,0);
	glDisable(GL_TEXTURE_2D);
	if(time < 1)
	{
		glEnable(GL_BLEND);
		Texture::Enable(false);

		glBlendFunc(GL_ZERO, GL_ONE_MINUS_SRC_ALPHA);

		glColor4f(1, 1, 1, 1-time);

		DrawStaticBG();

		glDisable(GL_BLEND);
		Texture::Enable(true);
	}

	render_cube();

	glPopAttrib();
	oglBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
	rendtotex(time);
	
}