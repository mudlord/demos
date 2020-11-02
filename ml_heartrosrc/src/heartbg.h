#include "heart_spr.h"
GLuint heart_texture;


#define NUM_HEARTS 10
sprite hearts[NUM_HEARTS];

sprite mr_sun;
float sun_angle = 210.0f; //we want rocking side to side.

void hearts_init()
{
	heart_texture = loadTGATextureMemory(heart_spr,heart_spr_len,true);

	for (int i=0;i<NUM_HEARTS;i++)
	{
		int tSize = 120;
		hearts[i].speed = rand_rangef(1.1,8.1);
		hearts[i].rotspeed = 0;
		hearts[i].x = rand_range(1,200)*i;
		hearts[i].y = YRES;
		hearts[i].xsize = tSize;
		hearts[i].ysize = tSize;
		hearts[i].texture = heart_texture;
		hearts[i].rot = 0;
	}
}

void draw_hearts()
{
	for (int i=0;i<NUM_HEARTS;i++)
	{
		float zPos = 0.0f;
		float  tX=hearts[i].xsize/2.0f;
		float  tY=hearts[i].ysize/2.0f;
		BeginOrtho2D(XRES,YRES);
		glEnable(GL_TEXTURE_2D);
		glDisable(GL_DEPTH_TEST);
		glBindTexture(GL_TEXTURE_2D,hearts[i].texture);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
		glPushMatrix();  // Save modelview matrix
		hearts[i].x+=hearts[i].speed;
		hearts[i].y-=hearts[i].speed;
		hearts[i].rot += hearts[i].rotspeed;
		if(hearts[i].x > XRES+86) hearts[i].x = -86;
		if(hearts[i].y < -86) hearts[i].y = YRES+86;
		glTranslatef(hearts[i].x,hearts[i].y,0.0f);  // Position sprite
		glRotatef(sun_angle,0.0f,0.0f,1.0f);
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

void hearts_render()
{
	draw_hearts();
}