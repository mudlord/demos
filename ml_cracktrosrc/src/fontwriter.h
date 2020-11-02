#include "sys/msys.h"
#include "intro.h"

#include "sprite.h"
#include "font.h"

#include "sync/sync.h"
#include "sync_page.h"
GLuint fonttexture;
sprite2 fontletter;
const struct sync_track *text_write;
const struct sync_track *page_write;
struct sync_device *rocket;


GLvoid fontwriter_init(GLvoid)								// Build Our Font Display List
{
	float	cx;											// Holds Our X Character Coord
	float	cy;											// Holds Our Y Character Coord
	GLuint fonttexture = loadTGATextureMemory(font,font_len,false);
	fontletter.setTextureID( fonttexture);
	rocket = sync_create_device("sync");
	#ifndef SYNC_PLAYER
	if (sync_connect(rocket, "localhost", SYNC_DEFAULT_PORT))
    {
			return;
	}
	#endif
#ifndef DEBUG
	text_write= sync_get_track_mem(rocket, "textwrite",sync_textwrite,sync_textwrite_len);
	page_write= sync_get_track_mem(rocket, "pagewrite",sync_pagewrite,sync_pagewrite_len);
#else
	text_write= sync_get_track(rocket, "textwrite");
	page_write= sync_get_track(rocket, "pagewrite");
#endif
	
	//initShader( &text_aa, bg_vert, (const char*)textaa_frag );
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
	if (c >= 'A' && c <= 'Z')pos = c - 'A';
	//.
	if (c == 0x2E)pos = 26;
	if (c>='0' && c <= '9')
	{ 
		pos = c - '0';
		pos +=27;
	}
	//space
	if (c == 0x20)pos = 37;
	return pos;
}




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


	fontletter.setPositionY(-objY );
	fontletter.setAlpha( 1.0f );
	fontletter.setWidth( 0.25f );
	fontletter.setHeight( 0.25f );
	fontletter.setFrameDelay( 1.0f );
	fontletter.setTextureAnimeInfo( 608,16, 16, 16,39,1,39);

	int l = strlen((char*)buffer);
	for(int a = 0; a < l; a++)
	{
		unsigned char c = buffer[a];
		int pos = gettablepos(c);
		fontletter.setPositionX(objX);
		fontletter.render(pos,255/255.0,255/255.0,255/255.0,time);
		objX -= -0.25;
	}
}

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
	{5, 10,100, 340,   -110, "TEAM OMB PRESENTS THIS CRACKTRO FOR",1.0},
	{5, 10,100, 320,   -110, "RADBUILDER 2.2.0.355 BY LONGTION",1.0},
	{5, 10,100, 340,   -110, "PROTECTION...     MODDED MD5       ",2.0},
	{5, 10,100, 320,   -110, "RELEASE TYPE...   KEYGEN           ",2.0},
	{5, 10,100, 300,   -110, "DATE...           28.9.2013        ",2.0},
	{5, 10,100, 340,   -110, "GREETZ GO TO..",3.0},
	{5, 10,100, 320,   -110, "RNDD FFF LNDL SND RED...",3.0},
	{5, 10,100, 300,   -110, "AND OTHERS WHO DESERVE RESPECT....",3.0},
	{5, 10,100, 340,   -110, "THNX TO MUDLORD FOR CODING THIS CRACKTRO..",4.0},
	{5, 10,100, 320,   -110, "GREETS TO 8BITBUBSY FOR THE NICE TUNE......",4.0},
	{5, 10,100, 300,   -110, "MUDLORD SENDS GREETS TO NERVE RAIZOR TITAN",4.0},
	{5, 10,100, 280,   -110, "SCENESAT DSS AND OTHERS HE MISSED....",4.0}
};
const int numtext = sizeof(demotext)/sizeof(text);

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


void fontwrite(float delta)
{
	static float texttime = 0;
	texttime += (float)delta/1000.f;

	if (texttime > 50.0)texttime = 0.0f;

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