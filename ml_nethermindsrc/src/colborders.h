#include "sys/msys.h"
#include "intro.h"

struct COLOR {
	float r,g,b,a;
};
struct COLOR *scrollbackcols = NULL;

void initcolborders()
{
	float factor=1.2f;
	struct COLOR rgb;
	 int cnt, colcnt=0;
	rgb.r = 0.0f;
	rgb.b = 0.0f;
	rgb.g = 0.0f;
	scrollbackcols = (struct COLOR*)malloc (sizeof(struct COLOR)*120);
	rgb.r = 1.0f;
	rgb.g = 0.0f;
	rgb.b = 0.0f;
	rgb.a = 1.0f;

	for (cnt=0; cnt < 20; cnt++) {
		scrollbackcols[colcnt].r = rgb.r;
		scrollbackcols[colcnt].g = rgb.g += 0.05f;
		scrollbackcols[colcnt].b = rgb.b;
		scrollbackcols[colcnt].a = rgb.a;
		colcnt++;
	}

	for (cnt=0; cnt < 20; cnt++) {
		scrollbackcols[colcnt].r = rgb.r -= 0.05f;
		scrollbackcols[colcnt].g = rgb.g;
		scrollbackcols[colcnt].b = rgb.b;
		scrollbackcols[colcnt].a = rgb.a;
		colcnt++;
	}

	for (cnt=0; cnt < 20; cnt++) {
		scrollbackcols[colcnt].r = rgb.r;
		scrollbackcols[colcnt].g = rgb.g;
		scrollbackcols[colcnt].b = rgb.b += 0.05f;
		scrollbackcols[colcnt].a = rgb.a;
		colcnt++;
	}

	for (cnt=0; cnt < 20; cnt++) {
		scrollbackcols[colcnt].r = rgb.r;
		scrollbackcols[colcnt].g = rgb.g -= 0.05f;
		scrollbackcols[colcnt].b = rgb.b;
		scrollbackcols[colcnt].a = rgb.a;
		colcnt++;
	}

	for (cnt=0; cnt < 20; cnt++) {
		scrollbackcols[colcnt].r = rgb.r += 0.05f;
		scrollbackcols[colcnt].g = rgb.g;
		scrollbackcols[colcnt].b = rgb.b;
		scrollbackcols[colcnt].a = rgb.a;
		colcnt++;
	}

	for (cnt=0; cnt < 20; cnt++) {
		scrollbackcols[colcnt].r = rgb.r;
		scrollbackcols[colcnt].g = rgb.g;
		scrollbackcols[colcnt].b = rgb.b -= 0.05f;
		scrollbackcols[colcnt].a = rgb.a;
		colcnt++;
	}
}

void drawColBorders ( void ) {
	int cnt=0;
	static int colcnt=0;
	float blockh=(3.5f/YRES)*2;
	// draws the colored lines top & bottom (lazy mode :P)
	glDisable(GL_TEXTURE_2D);
	glMatrixMode (GL_MODELVIEW);
	glPushMatrix ();
	glLoadIdentity ();
	glMatrixMode (GL_PROJECTION);
	glPushMatrix ();
	glLoadIdentity ();
	//bottom
	for (cnt=0; cnt < 60; cnt++) {
		glColor4f (scrollbackcols[ ((colcnt+cnt) % 120) ].r, scrollbackcols[ ((colcnt+cnt) % 120) ].g, scrollbackcols[ ((colcnt+cnt) % 120) ].b, 1.0f);
		glBegin(GL_TRIANGLE_STRIP);
		glVertex3f (-1.0f + (cnt*0.0333f), -0.85f, -1.0f);
		glVertex3f (-1.0f + (cnt+1)*0.0333f, -0.85f, -1.0f);
		glVertex3f (-1.0f + (cnt*0.0333f), -0.85f + blockh, -1.0f);
		glVertex3f (-1.0f + (cnt+1)*0.0333f, -0.85f + blockh, -1.0f);
		glEnd();
	}
	//top
	for (cnt=0; cnt < 60; cnt++) {
		glColor4f (scrollbackcols[ 119 - ((colcnt+cnt) % 120) ].r, scrollbackcols[ 119 - ((colcnt+cnt) % 120)  ].g, scrollbackcols[ 119 - ((colcnt+cnt) % 120) ].b, 1.0f);
		glBegin(GL_TRIANGLE_STRIP);
		glVertex3f (-1.0f + ((59-cnt)*0.0333f), 0.85f, -1.0f);
		glVertex3f (-1.0f + ((59-cnt+1)*0.0333f), 0.85f, -1.0f);
		glVertex3f (-1.0f + ((59-cnt)*0.0333f), 0.85f - blockh, -1.0f);
		glVertex3f (-1.0f + ((59-cnt+1)*0.0333f),0.85f - blockh, -1.0f);
		glEnd();
	}
	glPopMatrix ();
	glMatrixMode (GL_MODELVIEW);
	glPopMatrix ();


	float fps = 1000 / 60;
	static DWORD timeMoved = timeGetTime();
	//keep on getting the current time
	DWORD time = timeGetTime();
	//calculate how much time passed since timeMoved was updated
	DWORD timePassesSinceLastMove = time - timeMoved;
	if( timePassesSinceLastMove > fps)
	{ 
		colcnt++;
		timeMoved = timeGetTime();
	}
}