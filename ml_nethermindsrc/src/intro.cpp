#include "sys/msys.h"
#include "intro.h"
#include "fontwriter.h"
#include "shaderrepo.h"
#include "colborders.h"
#include "logodraw.h"
#include "shaderbackground.h"




#include "ufmod.h"
#include "song.h"


#define in_time(a, x) (sceneTime >= (a) && sceneTime < (a)+(x))
// A cool curve which is excellent for fading text
#define io_curve(x) ((x)*(1-(x))*4)
// Fades in & out during the given time frame
#define fade_io(a, x) (io_curve((sceneTime-(a))/(x)))

void loadspr();

int intro_init( int xr, int yr, int nomusic, IntroProgressDelegate *pd )
{
	//Resize(XRES,YRES);

	srand(GetTickCount());
    // progress report, (from 0 to 200)
    pd->func( pd->obj, 0 );
	initcolborders();
	fontwriter_init();
    shadertex_init();
	loadspr();
    pd->func( pd->obj, 1.0 );
	HWAVEOUT *res = uFMOD_PlaySong((void*)xm,(void*)sizeof(xm), XM_MEMORY);
	if (res == NULL) return 0;
    return 1;
}

void intro_end()
{
	
}

//---------------------------------------------------------------------

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
	{12, 20,10, 20,   -260, "HELLO. MUDLORD ON THE KEYS HERE FOR THIS"},
	{18.9, 20,10, 20,   -260, "SMALL INTRO FOR SYNTAX 2012. THIS IS DONE"},
	{26.0, 20,10, 20,   -260, "FOR THIS HERE SHINDIG ABOUT A MONTH BEFORE SAID SHENANIGANS."},
	{36.5, 20,10, 20,   -260, "GREETS GO TO....DSS ZORKE GARGAJ RAIZOR KNOEKI SUNSPIRE FELL"},
	{46.7, 20,10, 20,   -260, "AND ALL PEOPLE WHO WERE AT FLASHBACK AND NOW AT SYNTAX."},
	{56.0, 20,10, 20,   -260, "YAY FOR THE AUSSIE SCENE I SUPPOSE...."},
	{62.5, 20,10, 20,   -260, "GREETS TO ALIEN OF PDX FOR THE SWEET LOGO. THANKS DUDE."},
	{72.7, 20,10, 20,   -260, "HERETIC WROTE THE TUNE TO THIS SMALL PROD. THANKS BRO.."},
	{85.3, 20,10, 20,   -260, "FUCKINGS TO PEOPLE WHO WHINE ABOUT SHIT. MAKE A DEMO ABOUT IT...."},
	{101.5, 20,10, 20,   -260,"OR SHUT THE FUCK UP. THINK ABOUT IT FOR A MOMENT................."},
	{116.5, 20,10, 20,   -260,"SCROLLER WILL LOOP SOON. HOPE YOU LIKED MY LITTLE INTRO.........."},
	
};

const int numtext = sizeof(demotext)/sizeof(text);

int intro_do( void )
{
	static long lastTime = timeGetTime();
	long currTime = timeGetTime();
	long deltaTime = currTime - lastTime;
	lastTime = currTime;
	static float sceneTime = 0;
	sceneTime += (float)deltaTime/1000.f;
	static float texttime = 1000;
	texttime += (float)deltaTime/1000.f;

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	draw_tunneltex(deltaTime);
	drawColBorders ();
	drawglenz(deltaTime,sceneTime);

	if (texttime>135)texttime= 7.0;
	for(int i = 0; i < numtext; i++)
	{
		fontwrite_write(demotext[i].x + (texttime-demotext[i].start)*demotext[i].xspd,25,(char*)demotext[i].msg, deltaTime);
	}

	glScissor(1	,int(0.075f*YRES),XRES-2,int(0.85f*YRES));
    glEnable(GL_SCISSOR_TEST); 
	shadertex_do(sceneTime);
	glDisable(GL_SCISSOR_TEST);
	
	
	
		
	return 0;

}