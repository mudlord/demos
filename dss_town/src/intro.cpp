//--------------------------------------------------------------------------//
// iq . 2003/2008 . code for 64 kb intros by RGBA                           //
//--------------------------------------------------------------------------//


#include "sys/msys.h"
#include "sys/ufmod.h"
#include "intro.h"
#include "sys/msys.h"
#include "DemoText.h"

#include "atari.h"

//data 
extern "C" unsigned char house1[];
extern "C" DWORD   house1_len;
extern "C" unsigned char house2[];
extern "C" DWORD   house2_len;
extern "C" unsigned char house3[];
extern "C" DWORD   house3_len;
extern "C" unsigned char car1[];
extern "C" DWORD   car1_len;
extern "C" unsigned char car2[];
extern "C" DWORD   car2_len;
extern "C" unsigned char car3[];
extern "C" DWORD   car3_len;
extern "C" unsigned char car4[];
extern "C" DWORD   car4_len;
extern "C" unsigned char cloud_dat[];
extern "C" DWORD  cloud_len;
extern "C" unsigned char sun_dat[];
extern "C" DWORD  sun_len;
extern "C" unsigned char mountain_dat[];
extern "C" DWORD  mountain_len;
extern "C" unsigned char road_dat[];
extern "C" DWORD  road_len;
extern "C" unsigned char play_dat[];
extern "C" DWORD play_len;
extern "C" unsigned char stop_dat[];
extern "C" DWORD stop_len;
extern "C" unsigned char prev_dat[];
extern "C" DWORD  prev_len;
extern "C" unsigned char next_dat[];
extern "C" DWORD  next_len;
extern "C" unsigned char playover_dat[];
extern "C" DWORD playover_len;
extern "C" unsigned char stopover_dat[];
extern "C" DWORD stopover_len;
extern "C" unsigned char prevover_dat[];
extern "C" DWORD  prevover_len;
extern "C" unsigned char nextover_dat[];
extern "C" DWORD  nextover_len;




Font *fnt;
#include "post_proc.h"
#include "fx.h"
#include "sky.h"
#include "street.h"
#include "music_player.h"



#define in_time(a, x) (sceneTime >= (a) && sceneTime < (a)+(x))
// A cool curve which is excellent for fading text
#define io_curve(x) ((x)*(1-(x))*4)
// Fades in & out during the given time frame
#define fade_io(a, x) (io_curve((sceneTime-(a))/(x)))

FBOELEM motionblur[4];
int motionblur_shaderobj;

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
    "gl_FragColor = color/=4.0;"
	"}";

int intro_init( int xr, int yr, int nomusic, IntroProgressDelegate *pd )
{
	Resize(1024,768);
    // progress report, (from 0 to 200)
    pd->func( pd->obj, 0 );
	pd->func( pd->obj, 0.5 );
	fnt = new Font("[ATARI-FONT]Ver0.99", 20, false, false,atari,atari_len);
	sky_init();
	street_init();
	music_init();
	fx_init();
	init_fbo(4, motionblur, XRES, YRES);
	initShader(  &motionblur_shaderobj, (const char*) motionblur_vert, (const char*) motionblur_frag);

    pd->func( pd->obj, 1.0 );


   
    return 1;
}

void intro_end( void )
{    
	music_free();
    // deallicate your stuff here
}


//---------------------------------------------------------------------

#define SWAP(a,b,t) ((t)=(a), (a)=(b), (b)=(t))


int intro_do( void )
{
	static long lastTime = GetTime();
	long currTime = GetTime();
	long deltaTime = currTime - lastTime;
	lastTime = currTime;
	static float sceneTime = 0;
	sceneTime += (float)deltaTime/1000.f;

	//do special FX
	//do main render.
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glClearColor((float)89/255,(float)138/255,(float)255/255,1.0f);
	
	static int PingPong=0;
	PingPong= ( PingPong+ 1) % 4;

	start_fbo(motionblur[PingPong].fbo,XRES,YRES);
	sky_do(sceneTime);
	end_fbo();

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
	glBindTexture(GL_TEXTURE_2D,0);


	music_do();
	fx_do();
	street_do();
	
	

	

	
	
	

	


	
    
	

	/*if (sceneTime < 21 || sceneTime > 66)
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
	}*/
	

	return( 0 );
}