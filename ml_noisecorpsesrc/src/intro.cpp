//--------------------------------------------------------------------------//
// iq . 2003/2008 . code for 64 kb intros by RGBA                           //
//--------------------------------------------------------------------------//


#include "sys/msys.h"
#include "sys/ufmod.h"
#include "intro.h"
#include "sys/msys.h"
#include "cube_part.h"
#include "intropart.h"
#include "credits.h"
#include "DemoText.h"

Font *fnt;

#include "music.h"

#define in_time(a, x) (sceneTime >= (a) && sceneTime < (a)+(x))
// A cool curve which is excellent for fading text
#define io_curve(x) ((x)*(1-(x))*4)
// Fades in & out during the given time frame
#define fade_io(a, x) (io_curve((sceneTime-(a))/(x)))

int intro_init( int xr, int yr, int nomusic, IntroProgressDelegate *pd )
{
	Resize(XRES,YRES);
    // progress report, (from 0 to 200)
    pd->func( pd->obj, 0 );
	intropart_open();
	cubepart_open();
	 pd->func( pd->obj, 0.5 );
	credits_open();
	fnt = new Font("Arial", 42, true, false);
    // init your stuff here (mzk player, intro, ...)
    // remember to call pd->func() regularly to update the loading bar
	 pd->func( pd->obj, 1.0 );
	HWAVEOUT *res = uFMOD_PlaySong((void*)music,(void*)music_len, XM_MEMORY);
	if (res == NULL) return 0;

	
	
/*	while (1)
	{
		DWORD pos = uFMOD_GetRowOrder();
		int row =LOWORD(pos);
		int order = HIWORD(pos);
		if (order == 0 && row == 14) break;
		Sleep(1);
	}*/
   
    return 1;
}

void intro_end( void )
{    
    // deallicate your stuff here
}


//---------------------------------------------------------------------


int intro_do( void )
{
	static long lastTime = GetTime();
	long currTime = GetTime();
	long deltaTime = currTime - lastTime;
	lastTime = currTime;
	static float sceneTime = 0;
	sceneTime += (float)deltaTime/1000.f;
    // render your intro here
	if (sceneTime<21)
	{
		intropart_draw(sceneTime);
	}
	else if(sceneTime < 60)
	{
		cubepart_draw(sceneTime-21);
	}
	else if(sceneTime < 100 )
	{
		credits_draw(sceneTime-60);
	}
	else
    return 1;

	if (sceneTime < 21 || sceneTime > 66)
	{
		glEnable(GL_TEXTURE_2D);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		// Switch to 2D mode
		glDisable(GL_DEPTH_TEST);
		BeginOrtho2D(640, 480);

		// Loop through all of the text
		for(int i = 0; i < numtext; i++)
		{
			// Draw the text if it's visible
			glPushMatrix();
			if(in_time(demotext[i].start, demotext[i].duration))
			{
				float c = fade_io(demotext[i].start, demotext[i].duration);

				glColor4f(1, 1, 1, c);

				glTranslatef(demotext[i].x + (sceneTime-demotext[i].start)*demotext[i].xspd,
					demotext[i].y + (sceneTime-demotext[i].start)*demotext[i].yspd, 0);

				glScalef(1+demotext[i].xscl, 1+demotext[i].yscl, 1);

				fnt->Print(0, 0, demotext[i].msg);
			}
			glPopMatrix();
		}

		// Return to 3D
		EndProjection();
		glEnable(GL_DEPTH_TEST);
		glDisable(GL_TEXTURE_2D);
		glDisable(GL_BLEND);
	}
	

	return( 0 );
}