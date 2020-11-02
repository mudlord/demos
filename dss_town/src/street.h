#include "sys/msys.h"
#include "intro.h"

sprite house1_spr;
sprite house2_spr;
sprite house3_spr;
sprite pipsprite;


#define ROAD_SEGMENTS 12
sprite road[ROAD_SEGMENTS];

#define NUM_CARS 4
sprite car[NUM_CARS];
GLuint cartex[4];



void street_init()
{
	GLuint road_sprite;
	GLuint house1_sprite;
	GLuint house2_sprite;
	GLuint house3_sprite;
	house1_sprite = loadDDSTextureMemory(house1,house1_len,false);
	house2_sprite = loadDDSTextureMemory(house2,house2_len,false);
	house3_sprite = loadDDSTextureMemory(house3,house3_len,false);
	road_sprite = loadDDSTextureMemory(road_dat,road_len,false);
	cartex[0] = loadDDSTextureMemory(car1,car1_len,false);
	cartex[1] = loadDDSTextureMemory(car2,car2_len,false);
	cartex[2] = loadDDSTextureMemory(car3,car1_len,false);
	cartex[3]= loadDDSTextureMemory(car4,car4_len,false);




	float tSize2 = 256;
	//left
	house1_spr.xsize = 192;
	house1_spr.ysize = 256;
	house1_spr.x = 96;
	house1_spr.y = 178;
	//right
	house1_spr.texture = house1_sprite;
	house2_spr.xsize = 192;
	house2_spr.ysize = tSize2;
	house2_spr.x = XRES-96;
	house2_spr.y = 178;
	house2_spr.texture = house2_sprite;
	//middle
	house3_spr.xsize = tSize2;
	house3_spr.ysize = tSize2;
	house3_spr.x = XRES/2.0;
	house3_spr.y = 179;
	house3_spr.texture = house3_sprite;

	for (int i=0;i<2;i++)
	{
		float tSize = 100;
		car[i].speed = rand_rangef(3.1,7.1);
		car[i].rotspeed = 0;
		car[i].x = 70*i+1;
		car[i].y =50-(i*5);
		car[i].x=i;
		car[i].xsize = tSize;
		car[i].ysize = tSize;
		car[i].texture = cartex[i];
		car[i].rot = 0;
	}

	for (int i=2;i<NUM_CARS;i++)
	{
		float tSize = 100;
		car[i].speed = rand_rangef(3.1,7.1);
		car[i].rotspeed = 0;
		car[i].x = 70*i+1;
		car[i].y =30-(i*5);
		car[i].x=i;
		car[i].xsize = tSize;
		car[i].ysize = tSize;
		car[i].texture = cartex[i];
		car[i].rot = 0;
	}

/*	for (int i=3;i<NUM_CARS;i++)
	{
		float tSize = 100;
		car[i].speed = rand_rangef(3.1,8.1);
		car[i].rotspeed = 0;
		car[i].x = 70*i+1;
		car[i].y =30-(i*5);
		car[i].x=i;
		car[i].xsize = tSize;
		car[i].ysize = tSize;
		car[i].texture = cartex[i];
		car[i].rot = 0;
	}*/

	for (int i=0;i<ROAD_SEGMENTS;i++)
	{
		float tSize = 60;
		road[i].xsize = tSize;
		road[i].ysize = tSize;
		road[i].x = tSize*i;
		road[i].y = 21;
		road[i].texture = road_sprite;
	}
}


void draw_cars()
{
	for (int i=0;i<2;i++)
	{
		float zPos = 0.0f;
		float  tX=car[i].xsize/2.0f;
		float  tY=car[i].ysize/2.0f;
		BeginOrtho2D(XRES,YRES);
		glEnable(GL_TEXTURE_2D);
		glDisable(GL_DEPTH_TEST);
		glBindTexture(GL_TEXTURE_2D,car[i].texture);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
		glPushMatrix();  // Save modelview matrix
		
		car[i].x+=car[i].speed;
		car[i].rot += car[i].rotspeed;
		if(car[i].x > XRES) car[i].x = -100;
		glTranslatef(car[i].x,car[i].y,0.0f);  // Position sprite
		glRotatef(car[i].rot,0.0f,0.0f,1.0f);
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

	for (int i=2;i<NUM_CARS;i++)
	{
		float zPos = 0.0f;
		float  tX=car[i].xsize/2.0f;
		float  tY=car[i].ysize/2.0f;
		BeginOrtho2D(XRES,YRES);
		glEnable(GL_TEXTURE_2D);
		glDisable(GL_DEPTH_TEST);
		glBindTexture(GL_TEXTURE_2D,car[i].texture);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
		glPushMatrix();  // Save modelview matrix
		car[i].x-=car[i].speed;
		car[i].rot += car[i].rotspeed;
		if(car[i].x < -100) car[i].x = XRES;
		glTranslatef(car[i].x,car[i].y,0.0f);  // Position sprite
		glRotatef(car[i].rot,1.0f,0.0f,1.0f);
		glBegin(GL_QUADS);                                   // Draw sprite 
		glTexCoord2f(0.0f,0.0f); glVertex3i(tX, tY,zPos);
		glTexCoord2f(0.0f,1.0f); glVertex3i(tX,-tY,zPos);
		glTexCoord2f(1.0f,1.0f); glVertex3i( -tX,-tY,zPos);
		glTexCoord2f(1.0f,0.0f); glVertex3i( -tX, tY,zPos);
		glEnd();
		glPopMatrix();  // Restore modelview matrix
		glBindTexture(GL_TEXTURE_2D,0);
		glDisable(GL_TEXTURE_2D);
		glDisable(GL_BLEND);
		glEnable(GL_DEPTH_TEST);
		EndProjection();
	}
}

void street_do()
{
	draw_sprite(house1_spr, XRES, YRES);
	draw_sprite(house2_spr, XRES, YRES);
	draw_sprite(house3_spr, XRES, YRES);
	for (int i=0;i<ROAD_SEGMENTS;i++)draw_sprite(road[i],XRES,YRES);
	draw_cars();
}