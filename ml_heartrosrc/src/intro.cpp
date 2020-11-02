#include "sys/msys.h"
#include "intro.h"

#include "heart.h"
#include "heartbg.h"
#include "atari.h"
#include "music.h"
#include "ufmod.h"

typedef struct _obj {
	Point3	*vertices;
	long	*v_idx;
	Point3	*normals;
	long	*n_idx;
	Point2	*uvs;
	int		num_faces;
} Obj;
Obj heart_01;
Font *fnt;

GLfloat light_diffuse[]	=	{1.0, 0.0, 0.0, 1.0};	// red diffuse light
GLfloat light_position[] =	{1.0, 1.0, 1.0, 0.0};	// infinite light position

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
	float xspd, yspd;
	float xscl, yscl;
	const char *msg;
};

text demotext[] =
{
	/*
	 BEG  DUR  X    Y   XSP  YSP XSX YSX     TXT 
	*/	

	// End scroller
	{2, 10, XRES/2-400,   YRES/2+100,   0,  00,  0,  0, "mudlord presents this production"},
	{2, 10, XRES/2-400,   YRES/2+70,   0,  00,  0,  0, "for you fine people. "},
	{13, 10, XRES/2-400,   YRES/2+100,   0,  00,  0,  0, "the hardest part for this prod"},
	{13, 10, XRES/2-400,   YRES/2+70,   0,  00,  0,  0, "was doing the nice lil' heart model :D."},
	{13, 10, XRES/2-400,   YRES/2+40,   0,  00,  0,  0, "this was done mainly as a experiment"},
	{13, 10, XRES/2-400,   YRES/2+10,   0,  00,  0,  0, "for 3D modelling. i think it worked."},
	{25, 10, XRES/2-400,   YRES/2+100,   0,  00,  0,  0, "greets to groups like titan, razor,"},
	{25, 10, XRES/2-400,   YRES/2+70,   0,  00,  0,  0, "dss and fnuque."},
	{40, 5, XRES/2-400,   YRES/2+100,   0,  00,  0,  0, "if anyone cares, music by heretic"},
	{40, 5, XRES/2-400,   YRES/2+70,   0,  00,  0,  0, "rest of intro by me.. :("},
	{47, 5, XRES/2-400,   YRES/2+100,   0,  00,  0,  0, "text loops from here...DDDD:"}
};

const int numtext = sizeof(demotext)/sizeof(text);

FBOELEM motionblur[4];
int motionblur_shaderobj;


#define in_time(a, x) (sceneTime >= (a) && sceneTime < (a)+(x))
// A cool curve which is excellent for fading text
#define io_curve(x) ((x)*(1-(x))*4)
// Fades in & out during the given time frame
#define fade_io(a, x) (io_curve((sceneTime-(a))/(x)))

int intro_init( int xr, int yr, int nomusic, IntroProgressDelegate *pd )
{
	Resize(XRES,YRES);

	srand(NULL);
    // progress report, (from 0 to 200)
    pd->func( pd->obj, 0 );
	heart_01.vertices = heart_vertex;
	heart_01.v_idx = heart_vidx;
	heart_01.normals = heart_normal;
	heart_01.n_idx = heart_nidx;
	heart_01.uvs = heart_uv;
	heart_01.num_faces = (sizeof(heart_vidx)/sizeof(long));
	
	
	fnt = new Font("[ATARI-FONT]Ver0.99", 30, false, false,atari,atari_len);
	hearts_init();

	init_fbo(4, motionblur, XRES, YRES);
	initShader(  &motionblur_shaderobj, (const char*) motionblur_vert, (const char*) motionblur_frag);


    pd->func( pd->obj, 1.0 );
	HWAVEOUT *res = uFMOD_PlaySong((void*)music,(void*)sizeof(music), XM_MEMORY);
	if (res == NULL) return 0;
    return 1;
}

void intro_end()
{
	
}

void draw_heartexture(GLuint texture,int width, int height )
{
	glEnable(GL_TEXTURE_2D);
	glDisable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
	BeginOrtho2D(width,height,true);
	glBindTexture( GL_TEXTURE_2D,  texture);
	DrawFullScreenQuad(width,height);
	glBindTexture( GL_TEXTURE_2D,  0);
	glDisable(GL_BLEND);
	glEnable(GL_DEPTH_TEST);
	EndProjection();
}

void draw_heart()
{
	static int PingPong=0;
	PingPong= ( PingPong+ 1) % 4;
	glEnable	(GL_LIGHT0);
	glEnable	(GL_LIGHTING);
	start_fbo(motionblur[PingPong].fbo,XRES,YRES);
	glClearColor(0.0, 0.0, 0.0, 0.0);
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
	glLightfv	(GL_LIGHT0, GL_DIFFUSE,  light_diffuse);
	glLightfv	(GL_LIGHT0, GL_POSITION, light_position);
	static GLfloat angle1 = 0.0;
	angle1 += 2.0;
	glTranslatef( 0.0, 0.0, -6.0 );
	glRotatef( angle1,       0.0, 1.0, 0.0 );
	glBegin( GL_TRIANGLES );
	for(int i=0; i<heart_01.num_faces; i++)
	{
		//--- This is how to use the provided texture coordinates
		// glTexCoord2f( heart_01.uvs[heart_01.v_idx[i]].x, heart_01.uvs[heart_01.v_idx[i]].y );

		glNormal3f( heart_01.normals[heart_01.n_idx[i]].x,
			heart_01.normals[heart_01.n_idx[i]].y,
			heart_01.normals[heart_01.n_idx[i]].z );
		glVertex3f(	heart_01.vertices[heart_01.v_idx[i]].x,
			heart_01.vertices[heart_01.v_idx[i]].y,
			heart_01.vertices[heart_01.v_idx[i]].z );					
	}
	glEnd();
	end_fbo();
	glDisable(GL_LIGHT0);
	glDisable(GL_LIGHTING);
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
	draw_heartexture(motionblur[PingPong].texture,XRES, YRES);
	oglUseProgram( NULL );
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

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();
	//draw gradient
	BeginOrtho2D(1, 1); 
	glDepthMask(GL_FALSE); 
	glBegin(GL_QUADS); 
	glColor3f(150.0/255.0,38.0/255.0,1.0);
	glVertex2f(0.0f, 0.0f); 
	glVertex2f(1.0f, 0.0f); 
	glColor3f(197.0/255.0,0.0,0.5);
	glVertex2f(1.0f, 1.0f); 
	glVertex2f(0.0f, 1.0f); 
	glEnd(); 
	glDepthMask(GL_TRUE); 
	EndProjection(); 

	//draw_stars();
	hearts_render();


	 draw_heart();

	 if (sceneTime > 53) sceneTime = 0.0;
	 {
		 glEnable(GL_TEXTURE_2D);
		 glEnable(GL_BLEND);
		 glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		 // Switch to 2D mode
		 glDisable(GL_DEPTH_TEST);
		 BeginOrtho2D(XRES, YRES);
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
					 //flip Y coords when using FBOs
					 //	(YRES-demotext[i].y) + (sceneTime-demotext[i].start)*-demotext[i].yspd, 0);
					 (demotext[i].y) + (sceneTime-demotext[i].start)*demotext[i].yspd, 0);

				 glScalef(1+demotext[i].xscl, 1-demotext[i].yscl, 1);
				 //flip X when using FBO
				 //	glScalef(1.0f, -1.0f, 1.0f);
				 fnt->Print(0, 0, demotext[i].msg);
			 }
			 glPopMatrix();
		 }

		 // Return to 3D
		 EndProjection();
		 glDisable(GL_TEXTURE_2D);
		 glDisable(GL_BLEND);
	 }


	return 0;

}