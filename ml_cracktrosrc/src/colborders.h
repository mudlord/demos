#include "sys/msys.h"
#include "intro.h"

#define PI 3.1415926535897932384626433832795
#define ANG2RAD PI/180.0f

void drawColBorders ( void ) {
	int cnt=0;
	static float colcnt=0.0,colcur=colcnt;
	float blockh=(3.5f/YRES)*2;
	static float colcnt1=0.0,colcur1=colcnt1;
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
	//	starsprite[i].rcol = 90.0*.01;
	//	starsprite[i].gcol = 40.0*.01;
	//	starsprite[i].bcol = 150.0*.01;
		glColor4f (.2+ sin(colcur)*0.5, 0.0,.2+ sin(colcur)*0.5 ,1.0);
		glBegin(GL_TRIANGLE_STRIP);
		glVertex3f (-1.0f + (cnt*0.0333f), -0.85f, -1.0f);
		glVertex3f (-1.0f + (cnt+1)*0.0333f, -0.85f, -1.0f);
		glVertex3f (-1.0f + (cnt*0.0333f), -0.85f + blockh, -1.0f);
		glVertex3f (-1.0f + (cnt+1)*0.0333f, -0.85f + blockh, -1.0f);
		glEnd();
		colcur -= ANG2RAD * 12.;

	}
	//top
	    for (cnt=0; cnt < 60; cnt++) {
		glColor4f (.2+ sin(colcur1)*0.5, 0.0,.2+ sin(colcur1)*0.5 ,1.0);
		glBegin(GL_TRIANGLE_STRIP);
		glVertex3f (-1.0f + ((59-cnt)*0.0333f), 0.85f, -1.0f);
		glVertex3f (-1.0f + ((59-cnt+1)*0.0333f), 0.85f, -1.0f);
		glVertex3f (-1.0f + ((59-cnt)*0.0333f), 0.85f - blockh, -1.0f);
		glVertex3f (-1.0f + ((59-cnt+1)*0.0333f),0.85f - blockh, -1.0f);
		glEnd();
		colcur1 -= ANG2RAD * 12.;
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
		colcur -= ANG2RAD * 8.;
		colcur1 -= ANG2RAD * 8.;

		timeMoved = timeGetTime();
	}
}