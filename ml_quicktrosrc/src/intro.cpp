#include "sys/msys.h"
#include "intro.h"

#include "floor.h"
#include "fontwriter.h"
#include "ufmod.h"
#include "music.h"



#define in_time(a, x) (sceneTime >= (a) && sceneTime < (a)+(x))
// A cool curve which is excellent for fading text
#define io_curve(x) ((x)*(1-(x))*4)
// Fades in & out during the given time frame
#define fade_io(a, x) (io_curve((sceneTime-(a))/(x)))

struct text
{
	float start;
	float duration;
	float x, y;
	float xspd;
	const char *msg;
};

text demotext[] =
{
	/*
	 BEG  DUR  X    Y   XSP  YSP
	*/	

	// End scroller
	{6, 20,10, 20,   -260, "HELLO. MUDLORD AGAIN IN THIS SUPER QUICK"},
	{13.1, 20,10, 20,   -260, " ENTRY. THIS WAS DONE A WEEK BEFORE"},
	{19.5, 20,10, 20,   -260, "SYNTAX AND ON A MAJOR DEADLINE...."},
	{26.1, 20,10, 20,   -260, "THIS ALSO FEATURES A LOT OF REGURGITATED CODE.."},
	{35.1, 20,10, 20,   -260, "WHICH MAKES IT EASIER TO CODE. BUT AS YOU CAN "},
	{43.3, 20,10, 20,   -260, "SEE I RAN OUT OF IDEAS. GREETS TO RAVEN FOR THE EPIC LOGO..."},
	{54.1, 20,10, 20,   -260, "ZALZA DONE MUSIC AND THE REST IS ME. HOPED YOU LIKE IT."},

	
};

const int numtext = sizeof(demotext)/sizeof(text);

int intro_init( int xr, int yr, int nomusic, IntroProgressDelegate *pd )
{
	Resize(XRES,YRES);

	srand(NULL);
    // progress report, (from 0 to 200)
    pd->func( pd->obj, 0 );
	floor_init();
	fontwriter_init();
    pd->func( pd->obj, 1.0 );
	HWAVEOUT *res = uFMOD_PlaySong((void*)xm,(void*)sizeof(xm), XM_MEMORY);
    if (res == NULL) return 0;
    return 1;
}

void intro_end()
{
	
}




int intro_do( void )
{
	static long lastTime = timeGetTime();
	long currTime = timeGetTime();
	long deltaTime = currTime - lastTime;
	lastTime = currTime;
	static float sceneTime = 0;
	sceneTime += (float)deltaTime/1000.f;
	static float texttime = 0;
	texttime += (float)deltaTime/1000.f;

	float blockh=(3.5f/YRES)*2;

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	
	floor_render(sceneTime);

	if (texttime>64)texttime= 0;
	for(int i = 0; i < numtext; i++)
	{
		fontwrite_write(demotext[i].x + (texttime-demotext[i].start)*demotext[i].xspd,430,(char*)demotext[i].msg, deltaTime);
	}

	//fontwrite_write(XRES/2,420,"HELLO WORLD",0.0);
	
		
	return 0;

}