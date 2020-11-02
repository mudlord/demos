#include "sys/msys.h"
#include "intro.h"

#include "cave.h"
#include "to_logo.h"

#include "ufmod.h"
#include "music.h"
#include "fontwriter.h"

sprite mudlogospr;

FBOELEM motionblur[4];
int motionblur_shaderobj;


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
	{6, 20,10, 20,   -255,   "HELLO. MUDLORD AGAIN ON THE KEYS HERE FOR THIS"},
	{13.2, 20,10, 20,   -255,"INTRO FOR FLASHBACK 2O13! THIS PRODUCTION IS"},
	{20.0, 20,10, 20,   -255, "MY THIRD PROD AND HOPEFULLY MY BEST WORK!"},
	{26.5, 20,10, 20,   -255, "GREETS GO TO MY FRIENDS AND ALL PEOPLE AT FLASHBACK."},
	{35.0, 20,10, 20,   -255, "NOW SPEAKING ABOUT THIS PARTICULAR PRODUCTION."},
	{42.5, 20,10, 20,   -255, "NO POLYGONS WERE HARMED IN THIS PROD AT ALL..."},
	{50, 20,10, 20,   -255,   "CAN NOT SAY THE SAME ABOUT YOUR LIL SHADER UNITS! "},
	{58, 20,10, 20,   -255,   "I HOPE THIS EXERCISE IN GLSL WAS WORTH IT THOUGH! ALL THOSE HOURS"},
	{68, 20,10, 20,   -255,    " SLAVING OVER A HOT COMPUTER AND COMPILER.. MAY IT NOT BE IN VAIN. "},
	{80, 20,10, 20,   -255,  "THEN AGAIN, JUST CODING THIS INTRO WAS IMMENSE FUN, ALL THE TESTING.."},
	{92, 20,10, 20,   -255,  "ALL THE RERUNNING TO TEST TIMINGS, ALL THE RECODING...AND JUNK......."},
	{104, 20,10, 20,   -255,  "FINAL SCENE IS HERE, HOPED YOU LIKED THE PROD! IT WILL LOOP SOON....."}
	
};

const int numtext = sizeof(demotext)/sizeof(text);


void toplogo_init()
{

	GLuint logotex = loadTGATextureMemory(mudlogo,mudlogo_len,true);

	float tSize = 128;

	 mudlogospr.xsize = 351;
	 mudlogospr.ysize = 56;
	 mudlogospr.x = XRES/2;
	 mudlogospr.y = 30;

	 mudlogospr.texture = logotex;
	 mudlogospr.acol = 255;
	 mudlogospr.rcol = 255;
	 mudlogospr.gcol = 255;
	 mudlogospr.bcol = 255;


	 logofader_init();
}

const char  motionblur_vert[] = ""
	"varying vec4 vPos;"
	"void main()"
	"{"
	"gl_Position=gl_ModelViewProjectionMatrix*gl_Vertex;"
	"gl_TexCoord[0]=gl_MultiTexCoord0;"
	"gl_TexCoord[1]=gl_MultiTexCoord1;"
	"vPos=gl_Position;"
	"}";


const char motionblur_frag[] = ""
	"uniform sampler2D tex1;"
	"uniform sampler2D tex2;"
	"uniform sampler2D tex3;"
	"uniform sampler2D tex4;"
	"void main()"
	"{"
	"vec2 p=gl_TexCoord[0].rg;"
	"vec4 color=texture2D(tex1,p);"
	"color+=texture2D(tex2,p);"
	"color+=texture2D(tex3,p);"
	"color+=texture2D(tex4,p);"
	"color /=4;"
	"gl_FragColor = color;"
	"}";



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


#define in_time(a, x) (sceneTime >= (a) && sceneTime < (a)+(x))
// A cool curve which is excellent for fading text
#define io_curve(x) ((x)*(1-(x))*4)
// Fades in & out during the given time frame
#define fade_io(a, x) (io_curve((sceneTime-(a))/(x)))
#define SWAP(a,b,t) ((t)=(a), (a)=(b), (b)=(t))

int intro_init( int xr, int yr, int nomusic, IntroProgressDelegate *pd )
{
	Resize(XRES,YRES);

	srand(NULL);
    // progress report, (from 0 to 200)
    pd->func( pd->obj, 0 );
	shadertex_init();
	toplogo_init();
	fontwriter_init();
	init_fbo(4, motionblur, XRES, YRES);
	
	initShader(  &motionblur_shaderobj, (const char*) motionblur_vert, (const char*) motionblur_frag);
	HWAVEOUT *res = uFMOD_PlaySong((void*)music,(void*)sizeof(music), XM_MEMORY);
	if (res == NULL) return 0;

    pd->func( pd->obj, 1.0 );
//	playptmod_Play();
    return 1;
}

void intro_end()
{
	
}




int intro_do( void )
{
	static int PingPong=0;
	PingPong= ( PingPong+ 1) % 4;

	static long lastTime = timeGetTime();
	long currTime = timeGetTime();
	long deltaTime = currTime - lastTime;
	lastTime = currTime;
	static float sceneTime = 0;
	sceneTime += (float)deltaTime/1000.f;
	static float texttime = 0;
	texttime += (float)deltaTime/1000.f;

	//shader rendering
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	start_fbo(motionblur[PingPong].fbo,XRES,YRES);
	glScissor(1	,int(0.075f*YRES),XRES-2,int(0.85f*YRES));
	glEnable(GL_SCISSOR_TEST); 
	//shader timeline
	if (sceneTime<13){
		shadertex_do(sceneTime, shader[0]);
		float timerscene = sceneTime;
		if(timerscene < 1)
		{
			float time = 1-timerscene;
			logofader.acol = time;
			draw_sprite(logofader,XRES,YRES,true);
		}
		if(timerscene > 12)
		{

			float time = timerscene-12;
			logofader.acol = time;
			draw_sprite(logofader,XRES,YRES,true);
		}
	}
	else if(sceneTime < 40){
		float timerscene = sceneTime-13;
		shadertex_do(sceneTime,shader[1]);
		if(timerscene < 1)
		{
			float time = 1-timerscene;
			logofader.acol = time;
			draw_sprite(logofader,XRES,YRES,true);
		}

		if(timerscene > 26)
		{

			float time = timerscene-26;
			logofader.acol = time;
			draw_sprite(logofader,XRES,YRES,true);
		}


	}
	else if(sceneTime < 67 )
	{
		float timerscene = sceneTime-40;
		shadertex3_do(sceneTime);
		if(timerscene < 1)
		{
			float time = 1-timerscene;
			logofader.acol = time;
			draw_sprite(logofader,XRES,YRES,true);
		}
		if(timerscene > 26)
		{

			float time = timerscene-26;
			logofader.acol = time;
			draw_sprite(logofader,XRES,YRES,true);
		}


	}
	else if (sceneTime < 94)
	{
		float timerscene = sceneTime-67;
		shadertex_do(sceneTime,shader[2]);
		if(timerscene < 1)
		{
			float time = 1-timerscene;
			logofader.acol = time;
			draw_sprite(logofader,XRES,YRES,true);
		}
		if(timerscene > 26)
		{

			float time = timerscene-26;
			logofader.acol = time;
			draw_sprite(logofader,XRES,YRES,true);
		}



	}
	else if (sceneTime < 122)
	{
		float timerscene = sceneTime-94;
		shadertex_do(sceneTime,shader[3]);
		if(timerscene < 1)
		{
			float time = 1-timerscene;
			logofader.acol = time;
			draw_sprite(logofader,XRES,YRES,true);
		}
		if(timerscene > 28)
		{

			float time = timerscene-28;
			logofader.acol = time;
			draw_sprite(logofader,XRES,YRES,true);
		}

	}
	else if(sceneTime > 122) sceneTime = 0.0;
	glDisable(GL_SCISSOR_TEST);
	end_fbo();
	//motion blur
	char temp[5]={0};
	int blurtex[4];
	for (int i=0;i<4;i++)
	{
		ZeroMemory(temp,5);
		sprintf(temp,"tex%d",i+1,4);
		blurtex[i]=oglGetUniformLocation(motionblur_shaderobj, temp);
	}
	oglUseProgram(motionblur_shaderobj );
	for (int i=0;i<4;i++)
	{
		oglUniform1i(blurtex[i], i);
		oglActiveTextureARB(GL_TEXTURE0_ARB + i);
		glBindTexture(GL_TEXTURE_2D,motionblur[((PingPong + i) % 4)].texture);
	}

	oglActiveTextureARB(GL_TEXTURE0_ARB + 0);
	draw_fbtexture(motionblur[PingPong].texture,XRES, YRES);
	oglUseProgram( NULL );



	if (texttime>122)texttime= 0.0;
	for(int i = 0; i < numtext; i++)
	{
		fontwrite_write(demotext[i].x + (texttime-demotext[i].start)*demotext[i].xspd,25,(char*)demotext[i].msg, deltaTime);
	}



	draw_sprite(mudlogospr,XRES,YRES,true);


	//fontwrite_write(XRES/2,25,"MUDLORD!",0.0);




	return 0;

}