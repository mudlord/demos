#include "sys/msys.h"
#include "intro.h"

#define NUM_CLOUDS 6
sprite cloud[NUM_CLOUDS];
sprite mr_sun;
#define NUM_MOUNTAINS 8
sprite mountain[NUM_MOUNTAINS];


void sky_init()
{
	GLuint cloud_sprite;
	GLuint mountain_sprite;
	GLuint sun_sprite;
	cloud_sprite = loadDDSTextureMemory(cloud_dat,cloud_len,false);
	sun_sprite = loadDDSTextureMemory(sun_dat,sun_len,false);
	mountain_sprite = loadDDSTextureMemory(mountain_dat,mountain_len,false);
	float tSize;

	for (int i=0;i<NUM_CLOUDS;i++)
	{
		tSize=rand_range(20,128);
		cloud[i].speed = rand_rangef(1.1,8.1);
		cloud[i].rotspeed = 0;
		cloud[i].x = rand_range(1,200);
		cloud[i].y = 480-rand_range(20,200);
		cloud[i].xsize = tSize;
		cloud[i].ysize = tSize;
		cloud[i].texture = cloud_sprite;
		cloud[i].rot = 0;
	}

	for (int i=0;i<NUM_MOUNTAINS;i++)
	{
		float tSize = rand_rangef(250,300);
		mountain[i].xsize = tSize;
		mountain[i].ysize = tSize;
		mountain[i].x = rand_range(100,200)*i;
		mountain[i].y = 150;
		mountain[i].texture = mountain_sprite;
	}

	tSize = 120;
	mr_sun.xsize = tSize;
	mr_sun.ysize = tSize;
	mr_sun.x = XRES-50;
	mr_sun.y = 430;
	mr_sun.texture = sun_sprite;
}

void draw_sun(float time)
{
	float sun_rate    = 1.1f;
	float sun_angle = 30.0f; //we want rocking side to side.
	float zPos = 0.0f;
	float  tX=mr_sun.xsize/2.0f;
	float  tY=mr_sun.ysize/2.0f;
	BeginOrtho2D(XRES,YRES);
	glEnable(GL_TEXTURE_2D);
	glDisable(GL_DEPTH_TEST);
	glBindTexture(GL_TEXTURE_2D,mr_sun.texture);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
	glPushMatrix();  // Save modelview matrix
	glTranslatef(mr_sun.x,mr_sun.y,0.0f);  // Position sprite
	glRotatef(sin(time * sun_rate) * sun_angle,0.0f,0.0f,1.0f);
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

void draw_clouds()
{
	for (int i=0;i<NUM_CLOUDS;i++)
	{
		float zPos = 0.0f;
		float  tX=cloud[i].xsize/2.0f;
		float  tY=cloud[i].ysize/2.0f;
		BeginOrtho2D(XRES,YRES);
		glEnable(GL_TEXTURE_2D);
		glDisable(GL_DEPTH_TEST);
		glBindTexture(GL_TEXTURE_2D,cloud[i].texture);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
		glPushMatrix();  // Save modelview matrix
		cloud[i].x-=cloud[i].speed;
		cloud[i].rot += cloud[i].rotspeed;
		if(cloud[i].x < -12) cloud[i].x = XRES;
		glTranslatef(cloud[i].x,cloud[i].y,0.0f);  // Position sprite
		glRotatef(cloud[i].rot,0.0f,0.0f,1.0f);
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

void sky_do(float time)
{

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);	
	//sky colour
	glLoadIdentity();

	
	draw_sun(time);
	draw_clouds();
    for (int i=0;i<NUM_MOUNTAINS;i++)draw_sprite(mountain[i],XRES,YRES);
	
}