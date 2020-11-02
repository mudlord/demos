#include "sys/msys.h"
#include "intro.h"

#include "checkerboard.h"
#include "moon.h"
#include "mountain.h"
#include "cloud.h"
#include "logo.h"
#include "star.h"


sprite moon_spr;
#define NUM_MOUNTAINS 28
sprite mountains[NUM_MOUNTAINS];
GLuint check_tex;
sprite logospr;

#define NUM_STARS 64
sprite starsprite[NUM_STARS];

#define NUM_CLOUDS 48
typedef struct  
{
	float x,y,rot,speed;
	int xsize,ysize;
	float acol;
	GLuint texture;
}cloud_str;
cloud_str clouds[NUM_CLOUDS];



GLuint loadStarTextureMemory(unsigned char* data, int size, bool blur)
{
	GLuint texture_obj;
	PALCOL *palette;
	head header_struct;
	memset(&header_struct,0,sizeof(head));
	BUF* fd = bufopen(data,size);
	bufread(&header_struct,sizeof(header_struct), 1, fd);
	int width = header_struct.width;
	int height = header_struct.height;
	palette = header_struct.pallete;

	BYTE * colortable = new BYTE[width * height * 2];
	ZeroMemory(colortable,sizeof(colortable));
	bufread(colortable,width*height*2, 1, fd);
	bufclose(fd);

	BYTE *texture = new BYTE[width * height * 4];
	for(int i=0,k=0; i<width * height * 2;)
	{
		//BGR to RGB from FreeImage
		texture[k++]=palette[colortable[i]].blue;
		texture[k++]=palette[colortable[i]].green;
		texture[k++]=palette[colortable[i]].red;
		i++;
		texture[k++]=colortable[i]; //alpha
		i++;
	} 

	glGenTextures( 1, &texture_obj );
	glBindTexture( GL_TEXTURE_2D, texture_obj );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, blur?GL_LINEAR:GL_NEAREST  );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, blur?GL_LINEAR:GL_NEAREST );

	glTexImage2D( GL_TEXTURE_2D, 0, 4, width, height, 0,
		GL_RGBA, GL_UNSIGNED_BYTE, texture );
	return texture_obj;
}

void floor_init()
{
	GLuint moon_sprite;
	GLuint mountain_sprite;
	GLuint stars_sprite;
	GLuint clouds_sprite;
	GLuint logotex;

	check_tex = loadTexGenTexMemory(checkerboard,checkerboard_len,512,512);
	clouds_sprite = loadMIFTextureMemory(cloud,cloud_len,false);
	mountain_sprite = loadMIFTextureMemory(mountain,mountain_len,true);
	moon_sprite = loadMIFTextureMemory(moon,moon_len,true);
	logotex = loadDDSTextureMemory(logo,logo_len,false);
	stars_sprite = loadStarTextureMemory(star,star_len,false);


	for (int i=0;i<NUM_CLOUDS;i++)
	{
		float tSize=rand_range(20,128);
		clouds[i].speed = rand_rangef(1.1,8.1);
		clouds[i].x = rand_range(1,200);
		clouds[i].y = 760-rand_range(20,200);
		clouds[i].xsize = tSize;
		clouds[i].ysize = tSize;
		clouds[i].texture = clouds_sprite;
		clouds[i].rot = 10;
		clouds[i].acol = rand_range(60,160);
	}

	for (int i=0;i<NUM_MOUNTAINS;i++)
	{
		float tSize = 128;
		mountains[i].xsize = tSize;
		mountains[i].ysize = tSize;
		mountains[i].x = 50*i;
		mountains[i].y = 440;
		mountains[i].texture = mountain_sprite;
		mountains[i].acol = 255;
		mountains[i].rcol = 255;
		mountains[i].gcol = 255;
		mountains[i].bcol = 255;

	}


	float tSize = 128;

	moon_spr.xsize = tSize;
	moon_spr.ysize = tSize;
	moon_spr.x = XRES-70;
	moon_spr.y = 70;
	moon_spr.texture = moon_sprite;
	moon_spr.acol = 255;
	moon_spr.rcol = 255;
	moon_spr.gcol = 255;
	moon_spr.bcol = 255;

	for (int i=0;i<NUM_STARS;i++)
	{
		float tSize = rand_range(32,64);
		starsprite[i].xsize = tSize;
		starsprite[i].ysize = tSize;
		starsprite[i].speed = rand_rangef(1.0,6.5);
		starsprite[i].x = rand_range(50,1000);
		starsprite[i].y = 760-rand_range(400,760);
		starsprite[i].texture = stars_sprite;
		starsprite[i].acol = 255;
		starsprite[i].rcol = 255;
		starsprite[i].gcol = 255;
		starsprite[i].bcol = 255;

	}

	logospr.xsize = 642;
	logospr.ysize = 112;
	logospr.x = XRES/2;
	logospr.y = 100;
	logospr.texture = logotex;
	logospr.acol = 255;
	logospr.rcol = 255;
	logospr.gcol = 255;
	logospr.bcol = 255;

}


void draw_clouds()
{

	for (int i=0;i<NUM_CLOUDS;i++)
	{
		glDepthMask(GL_FALSE); 
		float zPos = 0.0f;
		float  tX=clouds[i].xsize/2.0f;
		float  tY=clouds[i].ysize/2.0f;
		BeginOrtho2D(XRES,YRES);
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D,clouds[i].texture);

		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
		//glAlphaFunc(GL_GREATER,0.1f);

		glPushMatrix();  // Save modelview matrix
		clouds[i].x-=clouds[i].speed;
		clouds[i].rot += clouds[i].speed;
		if(clouds[i].x < -12) clouds[i].x = 1024;

		glTranslatef(clouds[i].x,clouds[i].y,0.0f);  // Position sprite
		glColor4ub(255,255,255,clouds[i].acol);

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
		EndProjection();
		glDepthMask(GL_TRUE); 
	}

}

void draw_star(sprite spr, int xres, int yres, bool flip_y,float time)
{
	float zPos = 0.0;
	float  tX=spr.xsize/2.0f;
	float  tY=spr.ysize/2.0f;
	if (flip_y)BeginOrtho2D(xres,yres,true);
	else
		BeginOrtho2D(xres,yres);
	glEnable(GL_TEXTURE_2D);
	glDisable(GL_DEPTH_TEST);
	glBindTexture(GL_TEXTURE_2D,spr.texture);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
	glPushMatrix();  // Save modelview matrix
	glTranslatef(spr.x,spr.y,0.0f);  // Position sprite
	float sinetime = sin(time*spr.speed);
	glScalef(sinetime, sinetime, 1.0);
	glColor4ub(spr.rcol,spr.gcol,spr.bcol,spr.acol);
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


void floor_render(float time)
{
	static float floor_move = 0.0;
	floor_move+=0.01;
	float f20 = 20.0;
	float f1 = 5.0;


	glLoadIdentity();
    //gradient
	BeginOrtho2D(1, 1); 
	glDepthMask(GL_FALSE); 
	glBegin(GL_QUADS); 
	glColor3f(0.0,0.0,1.0);
	glTexCoord2f(0.0f, 0.0f); glVertex2f(0.0f, 0.0f); 
	glTexCoord2f(1.0f, 0.0f); glVertex2f(1.0f, 0.0f); 
	glColor3f(0.0,0.0,0.0);
	glTexCoord2f(1.0f, 1.0f); glVertex2f(1.0f, 1.0f); 
	glTexCoord2f(0.0f, 1.0f); glVertex2f(0.0f, 1.0f); 
	glEnd(); 
	glDepthMask(GL_TRUE); 
	EndProjection(); 
	
	//draw_stars();
	for (int i=0;i<NUM_MOUNTAINS;i++)draw_sprite(mountains[i],XRES,YRES,true);
	for (int i=0;i<NUM_STARS;i++)draw_star(starsprite[i],XRES,YRES,true,time);
	draw_sprite(moon_spr,XRES,YRES,true);
	draw_sprite(logospr,XRES,YRES,true);
	draw_clouds();
	
	//floor
	glMatrixMode(GL_TEXTURE);
	glPushMatrix();
	glLoadIdentity();
	glEnable(GL_TEXTURE_2D);
	glBindTexture( GL_TEXTURE_2D,  check_tex);
	glTranslatef(floor_move,1.0,0.0);
	glBegin(GL_QUADS);
	glColor3f(0.0,0.0,1.0);
	glTexCoord2f( 0.0f,f1); glVertex3f(-f20,-1.0f, f20);
	glTexCoord2f( 0.0f, 0.0f); glVertex3f( f20,-1.0f, f20);
	glTexCoord2f(f1, 0.0f); glVertex3f( f20,-1.0f,-f20);
	glTexCoord2f(f1,f1); glVertex3f(-f20,-1.0f,-f20);
	glEnd();
	glMatrixMode(GL_TEXTURE);
	glPopMatrix();

	glDisable(GL_TEXTURE_2D);

	//moon
	
	


}