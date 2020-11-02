#include "sys/msys.h"
#include "intro.h"
#include "stb_image.c"
#include "rotozoomer.h"
#include "gradients.h"
#include "twister.h"
#include "fontwriter.h"

int intro_init( int xr, int yr, int nomusic, IntroProgressDelegate *pd )
{
	srand(NULL);
    // progress report, (from 0 to 200)
    pd->func( pd->obj, 0 );
	init_rotozoomer();
	init_gradients();
	init_twister();
	init_fonts();
    pd->func( pd->obj, 200 );

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
	delta = diff;
	lastTime = currTime;
	static float sceneTime = 0;

	sceneTime += (float)delta/1000.f;
	static float texttime = 0;
	texttime += (float)delta/1000.f;

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glClearColor(0.0, 0.0,0.0, 1.0f);
	glViewport(0,0,XRES,YRES);

	glScissor(1	,int(0.075f*YRES),XRES-2,int(0.85f*YRES));
	glEnable(GL_SCISSOR_TEST); 
	draw_rotozoomer(sceneTime);
	glDisable(GL_SCISSOR_TEST);

	draw_gradients(sceneTime);
	draw_twister(delta);
	draw_fonts();
	return 0;

}