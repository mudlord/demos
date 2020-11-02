#include "sys/msys.h"
#include "intro.h"

#include "sprite.h"
#include "font.h"



#include "syncs.h"
#include "sync/sync.h"
const struct sync_track *text_write;
const struct sync_track *page_write;
struct sync_device *rocket;
sprite2 fontletter;
sprite2 fontletter2;

GLvoid fontwriter_init(GLvoid)								// Build Our Font Display List
{
	float	cx;											// Holds Our X Character Coord
	float	cy;											// Holds Our Y Character Coord
	int width, height, comp;
	unsigned char *data = stbi_load_from_memory(font,font_len,&width,&height,&comp,4);
	GLuint fonttexture;
	glGenTextures( 1, &fonttexture );
	glBindTexture( GL_TEXTURE_2D, fonttexture );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
	glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0,
		GL_RGBA, GL_UNSIGNED_BYTE, data );
	fontletter.setTextureID( fonttexture);
	fontletter2.setTextureID( fonttexture);


	rocket = sync_create_device("sync");
#ifndef SYNC_PLAYER
	if (sync_connect(rocket, "localhost", SYNC_DEFAULT_PORT))
	{
		return;
	}
#endif
#ifndef DEBUG
	text_write= sync_get_track_mem(rocket, "textwrite",sync_textwrite_data,sizeof(sync_textwrite_data));
	page_write= sync_get_track_mem(rocket, "pagewrite",sync_pagewrite_data,sizeof(sync_pagewrite_data));
#else
	text_write= sync_get_track(rocket, "textwrite");
	page_write= sync_get_track(rocket, "pagewrite");
#endif

}

GLvoid fontwriter_save()
{
#ifndef SYNC_PLAYER
	sync_save_tracks(rocket);
#endif
	sync_destroy_device(rocket);
}

int gettablepos(char c)
{
	int pos;
	if (c >= 'A' && c <= 'Z')
		pos = c - 'A';
	//space
	if (c == 0x20)pos = 26;
	//*
	if (c == 0x2A)pos = 27;
	//+
	if (c == 0x2B)pos = 28;
	//,
	if (c == 0x2C)pos = 29;
	// - 
	if (c == 0x2D)pos = 30;
	//.
	if (c == 0x2E)pos = 31;
	// /
	if (c == 0x2F)pos = 32;
	
	if (c>='0' && c <= '9')
	{ 
		pos = c - '0';
		pos +=33;
	}
	//space
	
	return pos;
}


#define PI 3.1415926535897932384626433832795
#define ANG2RAD PI/180.0f

void Print(float x1, float y1,float time, const char *fmt, ...)
{
	va_list ap;
	unsigned char buffer[256];
	va_start(ap, fmt);
	vsprintf((char*)buffer, fmt, ap);
	va_end(ap);
	
	double objX, objY;
	world_coords coords = GetWorldCoords((int)x1,(int)y1);
	objX = coords.x;
	objY = coords.y;

	static GLfloat colCnt = 0.0f;
	GLfloat colCur;
	colCur = colCnt;



	float val = 0.25;
	fontletter.setPositionY(-objY );
	fontletter2.setPositionY(-objY-0.03 );
	fontletter2.setAlpha( 1.0f );
	fontletter2.setWidth( val+0.03 );
	fontletter2.setHeight( val+0.03 );
	fontletter2.setFrameDelay( 1.0f );
	fontletter.setAlpha( 1.0f );
	fontletter.setWidth( val );
	fontletter.setHeight( val );
	fontletter.setFrameDelay( 1.0f );
	
	 void  setTextureAnimeInfo( int nTextureWidth, int nTextureHeight, 
                               int nFrameWidth,   int nFrameHeight,
                               int nNumFrameColumns, int nNumFrameRows, 
                               int nTotalFrames,
                               int nOffsetX = 0, int nOffsetY = 0 ) 

	fontletter.setTextureAnimeInfo(720,16,16,16,45,1,45);
	fontletter2.setTextureAnimeInfo(720,16,16,16,45,1,45);
	//fontletter.setTextureAnimeInfo(416,16, 16, 16,26,1,26);

	int l = strlen((char*)buffer);
	for(int a = 0; a < l; a++)
	{
		unsigned char c = buffer[a];
		int pos = gettablepos(c);
		fontletter.setPositionX(objX);
		fontletter2.setPositionX(objX+0.03);
		//fontletter.render(pos,sin(colCur)*0.7, sin(colCur)*0.7, sin(colCur)*0.4,time);
		//fontletter.render(pos,.7+ sin(colCur)*0.1, .7+ sin(colCur)*0.1, 0.9,time);
		
		fontletter2.render(pos,0.0,0.0,0.0,time);
		fontletter.render(pos,0.8,0.8,0.9,time);
		objX += 0.25;
		colCur -= ANG2RAD * 8.;

	}
	colCnt -= ANG2RAD * 3.;

}


void fontwrite_write(int x, int y,char* text, float time)
{
	BeginPerspective(45.0, (GLdouble)XRES / (GLdouble)YRES) ;
	gluLookAt( 0.0, 0.0, 10,  // Camera Position
		0.0, 0.0, 0.0, // Look At Point
		0.0, 1.0, 0.0);// Up Vector
	Print(x,y,time, text);
	EndProjection();
}
#define in_time(a, x) (time >= (a) && time < (a)+(x))
// A cool curve which is excellent for fading text
#define io_curve(x) ((x)*(1-(x))*4)
// Fades in & out during the given time frame
#define fade_io(a, x) (io_curve((time-(a))/(x)))


struct text
{
	float start;
	float duration;
	float x, y;
	float xspd;
	const char *msg;
	float page;
};

text demotext[] =
{
	/*
	 BEG  DUR  X    Y   XSP  YSP
	*/	

	// End scroller
	{5, 10,100, 500,   -110, "***********************************",1.0},
	{5, 10,100, 475,   -110, "* WE INVITE YOU TO VISIT **********",1.0},
	{5, 10,100, 450,   -110, "* WWW.LIBRETRO.COM FOR ALL YOUR ***",1.0},
	{5, 10,100, 425,   -110, "* EMULATION AND GAMING NEEDS. *****",1.0},
	{5, 10,100, 400,   -110, "***********************************",1.0},

	{5, 10,100, 500,   -110, "***********************************",2.0},
	{5, 10,100, 475,   -110, "* WE USE RETROARCH WHICH IS A *****",2.0},
	{5, 10,100, 450,   -110, "* TOTALLY CROSS-PLATFORM IMPL. ****",2.0},
	{5, 10,100, 425,   -110, "* OF THE LIBRETRO API. ************",2.0},
	{5, 10,100, 400,   -110, "***********************************",2.0},


	{5, 10,100, 500,   -110, "***********************************",3.0},
	{5, 10,100, 475,   -110, "* LIBRETRO IS *THE* TOTALLY *******",3.0},
	{5, 10,100, 450,   -110, "* AWESOME API THAT MAKES ALL YOUR *",3.0},
	{5, 10,100, 425,   -110, "* EMULATOR, *DEMO*, GAME AND  *****",3.0},
	{5, 10,100, 400,   -110, "* OTHER MULTIMEDIA DEVEL. AS EASY *",3.0},
	{5, 10,100, 375,   -110, "* AS PIE. *************************",3.0},
	{5, 10,100, 350,   -110, "***********************************",3.0},

	{5, 10,100, 500,   -110, "***********************************",4.0},
	{5, 10,100, 475,   -110, "* RETROARCH WORKS ON       ********",4.0},
	{5, 10,100, 450,   -110, "* PS3, PSP, XBOX1, XBOX360, GC , **",4.0},
	{5, 10,100, 425,   -110, "* WII, RASPBERRY PI, OPENPANDORA, *",4.0},
	{5, 10,100, 400,   -110, "* ANDROID, BLACKBERRY, IOS, *******",4.0},
	{5, 10,100, 375,   -110, "* WINDOWS, LINUX AND MAC **********",4.0},
	{5, 10,100, 350,   -110, "***********************************",4.0},

	{5, 10,100, 500,   -110, "***********************************",5.0},
	{5, 10,100, 475,   -110, "* CREDITS FOR THIS INTRO -  *******",5.0},
	{5, 10,100, 450,   -110, "* CODE - MUDLORD            *******",5.0},
	{5, 10,100, 425,   -110, "* GRAPHICS - RETINALECLIPSE *******",5.0},
	{5, 10,100, 400,   -110, "* MUSIC - KEITO             *******",5.0},
	{5, 10,100, 375,   -110, "***********************************",5.0},

	{5, 10,100, 500,   -110, "******************************************",6.0},
	{5, 10,100, 475,   -110, "* WE HOPE YOU VISIT OUR NICE LIL *********",6.0},
	{5, 10,100, 450,   -110, "* IRC CHANNEL *RETROARCH* ON *FREENODE* **",6.0},
	{5, 10,100, 425,   -110, "******************************************",6.0}
};
const int numtext = sizeof(demotext)/sizeof(text);


void fontwrite(float delta)
{

	//	fontwrite_write(XRES/2,YRES/2,"HELLO WORLD.", 1.0);


		static float texttime = 0;
		texttime += (float)delta/1500.f;

		if (texttime > 80.0)texttime = 0.0f;

		for(int i = 0; i < numtext; i++)
		{
			double row = texttime * 8.0;
#ifndef SYNC_PLAYER
			if (sync_update(rocket, (int)floor(row), NULL, NULL))
				sync_connect(rocket, "localhost", SYNC_DEFAULT_PORT);
#endif
			float scroll_away = sync_get_val(text_write, row);
			float page = sync_get_val(page_write,row);
			if (page == demotext[i].page)
			{
				fontwrite_write(scroll_away,demotext[i].y,(char*)demotext[i].msg, 1.0);
			}
		}

	
}