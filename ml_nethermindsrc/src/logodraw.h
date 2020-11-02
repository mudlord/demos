#include "sys/msys.h"
#include "intro.h"

#include "logo.h"
#include "bubble.h"

#define NUM_BUBBLES 10
sprite bubbles[NUM_BUBBLES];
GLuint bubble_tex;

sprite logospr;
GLuint logo_spr;
static int logoshader;

void draw_bubbles()
{
	for (int i=0;i<NUM_BUBBLES;i++)
	{
		float zPos = 0.0f;
		float  tX=bubbles[i].xsize/2.0f;
		float  tY=bubbles[i].ysize/2.0f;
		BeginOrtho2D(XRES,YRES);
		glEnable(GL_TEXTURE_2D);
		glDisable(GL_DEPTH_TEST);
		glBindTexture(GL_TEXTURE_2D,bubbles[i].texture);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
		glColor4f(1.0,1.0,1.0,0.5);
		glPushMatrix();  // Save modelview matrix
		bubbles[i].y+=bubbles[i].speed;
		bubbles[i].rot += bubbles[i].rotspeed;
		if(bubbles[i].y >720) bubbles[i].y = 675;
		glTranslatef(bubbles[i].x,bubbles[i].y,0.0f);  // Position sprite
		glRotatef(bubbles[i].rot,0.0f,0.0f,1.0f);
		glBegin(GL_QUADS);                                   // Draw sprite 
		glTexCoord2f(0.0f,0.0f); glVertex3i(-tX, tY,zPos);
		glTexCoord2f(0.0f,1.0f); glVertex3i(-tX,-tY,zPos);
		glTexCoord2f(1.0f,1.0f); glVertex3i( tX,-tY,zPos);
		glTexCoord2f(1.0f,0.0f); glVertex3i( tX, tY,zPos);
		glEnd();
		glPopMatrix();  // Restore modelview matrix
		glBindTexture(GL_TEXTURE_2D,0);
		glDisable(GL_TEXTURE_2D);
		glDisable(GL_BLEND);
		glEnable(GL_DEPTH_TEST);
		EndProjection();
	}
}


void loadspr()
{
	bubble_tex = loadDDSTextureMemory(bubble,bubble_len,false);
	for (int i=0;i<NUM_BUBBLES;i++)
	{
		float tSize=rand_range(10,16);
		bubbles[i].speed = rand_rangef(0.1,0.5);
		bubbles[i].rotspeed = 0;
		bubbles[i].x = 505+(i*30);
		bubbles[i].y = 680;
		bubbles[i].xsize = tSize;
		bubbles[i].ysize = tSize;
		bubbles[i].texture = bubble_tex;
		bubbles[i].rot = 0;
	}

	GLuint logo_spr = loadDDSTextureMemory(logo1,logo1_len,false);
	float tSize = 120;
	logospr.xsize = 315;
	logospr.ysize = 54;
	logospr.x = XRES/2;
	logospr.y = 693;
	logospr.texture = logo_spr;

#ifdef _DEBUG
	unsigned char* vertex = readShaderFile("vertex.inl");
	unsigned char* fragment = readShaderFile("fragment_logo.inl");
	initShader( &logoshader, (const char*)vertex, (const char*)fragment );
#else
	initShader( &logoshader, logo_vert, (const char*)logo_fragment );
#endif
}

void drawlogo()
{
	float time=timeGetTime();
	time /= 100.00;
	float resolution[2] = {315,58};
	GLfloat time2[2] = {time,0};
	GLuint resolution_uniform = oglGetUniformLocation(logoshader, "resolution" );
	GLuint timer_uniform = oglGetUniformLocation( logoshader, "time" );
	oglUseProgram(logoshader);
	GLuint shadertexture =  oglGetUniformLocation( logoshader, "tex0" );
	oglUniform1i(shadertexture,0);
	oglUniform1i(shadertexture,0);
	oglUniform2fv(resolution_uniform,1,resolution);
	oglUniform2fv(timer_uniform,1,time2);
	draw_sprite(logospr,XRES,YRES);
	oglUseProgram(0 );
	
}

void glenz_side(GLint r,GLint g,GLint b,GLfloat l, float point)
{
	GLfloat d=0.5f;
	GLfloat w[4]={1.0f,1.0f,1.0f,l};
	GLfloat c[4]={(GLfloat)(r),(GLfloat)(g),(GLfloat)(b),l};

	glBegin(GL_TRIANGLES);				
	glMaterialfv(GL_FRONT_AND_BACK,GL_AMBIENT_AND_DIFFUSE,w);
	glNormal3f(0.25f,-0.25f,-0.75f);
	glVertex3f(-d, d, d);
	glVertex3f( d, d, d);
	glVertex3f(0.0,0.0,(GLfloat)(d*point));		
	glMaterialfv(GL_FRONT_AND_BACK,GL_AMBIENT_AND_DIFFUSE,c);
	glNormal3f(-0.25f,-0.25f,-0.75f);
	glVertex3f( d, d, d);
	glVertex3f( d,-d, d);
	glVertex3f(0.0,0.0,(GLfloat)(d*point));		
	glMaterialfv(GL_FRONT_AND_BACK,GL_AMBIENT_AND_DIFFUSE,w);
	glNormal3f(-0.25f,0.25f,-0.75f);
	glVertex3f( d,-d, d);
	glVertex3f(-d,-d, d);
	glVertex3f(0.0,0.0,(GLfloat)(d*point));		
	glMaterialfv(GL_FRONT_AND_BACK,GL_AMBIENT_AND_DIFFUSE,c);
	glNormal3f(0.25f,0.25f,-0.75f);
	glVertex3f(-d,-d, d);
	glVertex3f(-d, d, d);
	glVertex3f(0.0,0.0,(GLfloat)(d*point));
	glEnd();
}

typedef struct{
	float x,y,z;
}rotate;

typedef struct{
	float r,g,b,a;
}colors;

void gen_glenz(float r,float g,float b,float l, rotate rot, float point = 1.75)
{
	//glLoadIdentity();
	



	glRotatef(rot.x,1.0f,0.0f,0.0f);
	glRotatef(rot.y,0.0f,1.0f,0.0f);
	glRotatef(rot.z,0.0f,0.0f,1.0f);

	glenz_side(r,g,b,l,point);
	glRotatef(90,0.0f,0.0f,1.0f);
	glRotatef(90,1.0f,0.0f,0.0f);
	glenz_side(r,g,b,l,point);
	glRotatef(180,0.0f,1.0f,0.0f);
	glenz_side(r,g,b,l,point);
	glRotatef(90,0.0f,0.0f,1.0f);
	glRotatef(270,0.0f,1.0f,0.0f);
	glenz_side(r,g,b,l,point);
	glRotatef(90,0.0f,0.0f,1.0f);
	glRotatef(90,0.0f,1.0f,0.0f);
	glenz_side(r,g,b,l,point);
	glRotatef(180,0.0f,1.0f,0.0f);
	glenz_side(r,g,b,l,point);
}

void draw_glenz(double x, double y, double z,rotate rot, colors col,float point = 1.75)
{
	BeginPerspective(45.0, (GLdouble)XRES / (GLdouble)YRES) ;
	glTranslatef(x,y,z);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	float posLight0[4] = {-1.0f, 1.0f, 1.0f, 0.0f};
	float ambLight0[4] = {0.5f, 0.5f, 0.5f, 0.5f};
	glLightfv(GL_LIGHT0, GL_POSITION, posLight0);
	glLightfv(GL_LIGHT0, GL_AMBIENT, ambLight0);
	float specLight0[4] = {0.5f, 0.5f, 0.5f, 1.0f};
	glLightfv(GL_LIGHT0, GL_SPECULAR, specLight0);
	gen_glenz(col.r,col.g,col.b,col.a,rot,point);
	glDisable(GL_LIGHTING);
	glDisable(GL_LIGHT0);
	glDisable(GL_BLEND);
	EndProjection();
}



float pulse(float time) {
	const float pi = 3.14;
	const float frequency = 10; // Frequency in Hz
	return 1.5*(1+sin(2 * pi * frequency * time));
}

void drawglenz(float scenetime, float deltatime)
{
	world_coords coords = GetWorldCoords(500,10);
	static rotate rot  = {0.00,0.0,0.0};
	static rotate rot2 = {0.00,0.0,0.0};
    static rotate rot3 = {0.00,0.0,0.0};
	const colors col = {75/255.0,0.0,1.0,0.7};
	const colors col2 = {69/255.0,43/255.0,131/255.0,0.7};
	rot.x += scenetime/20;
	rot.y += scenetime/5;
	rot.z += scenetime/10;
	rot2.x += scenetime/5;
	rot2.y += scenetime/5;
	rot2.z += scenetime/10;
	rot3.x += scenetime/6;
	rot3.y += scenetime/5.4;

	draw_glenz(-3.9, 10.7, -28, rot,col);
	draw_glenz(3.9, 10.7, -28,rot2,col);
	draw_glenz(0.0, 10.7, -28,rot3,col2);
	drawlogo();
	draw_bubbles();
}