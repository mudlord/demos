#include "sys/msys.h"
#include "intro.h"
#include <sys/stat.h>
//data
#include "introtex.h"
#include "intropart_nurbs.h"
GLuint intropart_bg;
GLuint intropart_nurbs;


#define NRDIVS 32
float height[NRDIVS+1][NRDIVS+1];

void intropart_open()
{
	intropart_bg = loadTexGenTexMemory(introtex,introtex_len,512,512);
	intropart_nurbs = loadTexGenTexMemory(intopart_nurbs,intopart_nurbs_len,512,512);

	for(int i=0;i<=NRDIVS;i++)
	{
		for(int j=0;j<=NRDIVS;j++)
		{
			height[i][j]=0;
		}
	}

}

void drawFlag(float fTime)
{
	int x,y;
	float xp,yp;

	glPushMatrix();

	glTranslatef (0.0, 0.0, -5.0);


	glEnable(GL_BLEND);
	glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D,intropart_nurbs);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glRotatef((float)(7*sin(fTime*0.5)),0,1,0);
	glRotatef(240.0f,1,0,0);


	glBegin(GL_TRIANGLES);
	for(x=0;x<NRDIVS;x++)
	{
		for(y=0;y<NRDIVS;y++)
		{
			xp=-2.0f+x*(4.0f/NRDIVS);
			yp=-2.0f+y*(4.0f/NRDIVS);
			glColor4f(1,1,1,0.7f+height[x][y]);
			glTexCoord2f( x*(1.0f/NRDIVS), y*(1.0f/NRDIVS) );
			glVertex3f(xp,yp,height[x][y]);
			glColor4f(1,1,1,0.7f+height[x+1][y]);
			glTexCoord2f((x+1)*(1.0f/NRDIVS), y*(1.0f/NRDIVS) );
			glVertex3f(xp+(4.0f/NRDIVS),yp,height[x+1][y]);
			glColor4f(1,1,1,0.7f+height[x][y+1]);
			glTexCoord2f( x*(1.0f/NRDIVS), (y+1)*(1.0f/NRDIVS));
			glVertex3f(xp,yp+(4.0f/NRDIVS),height[x][y+1]);
			glColor4f(1,1,1,0.7f+height[x+1][y]);
			glTexCoord2f( (x+1)*(1.0f/NRDIVS), y*(1.0f/NRDIVS) );
			glVertex3f(xp+(4.0f/NRDIVS),yp,height[x+1][y]);
			glColor4f(1,1,1,0.7f+height[x][y+1]);
			glTexCoord2f( x*(1.0f/NRDIVS), (y+1)*(1.0f/NRDIVS));
			glVertex3f(xp,yp+(4.0f/NRDIVS),height[x][y+1]);
			glColor4f(1,1,1,0.7f+height[x+1][y+1]);
			glTexCoord2f( (x+1)*(1.0f/NRDIVS), (y+1)*(1.0f/NRDIVS));
			glVertex3f(xp+(4.0f/NRDIVS),yp+(4.0f/NRDIVS),height[x+1][y+1]);
		}
	}
	glEnd();

	for(x=0;x<=NRDIVS;x++)
	{
		for(y=0;y<=NRDIVS;y++)
		{
			height[x][y]=(float)(0.7*sin(x*0.5+fTime*0.9)*0.5*sin(y*0.5+fTime*0.6));
		}
	}

	glScalef(1.0,1.0,3.0);
	glDisable(GL_TEXTURE_2D);
	glPopMatrix();
	glDisable(GL_BLEND);



}

void intropart_draw(float time)
{
	oglBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fbo);
	
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);	
	glLoadIdentity();		
	glEnable(GL_TEXTURE_2D);

	glBindTexture(GL_TEXTURE_2D,intropart_bg);
	DrawDistort(time*2);
	glBindTexture(GL_TEXTURE_2D,0);
	glDisable(GL_TEXTURE_2D);

	drawFlag(time*3);

	if(time < 1)
	{
		glEnable(GL_BLEND);
		Texture::Enable(false);

		glBlendFunc(GL_ZERO, GL_ONE_MINUS_SRC_ALPHA);

		glColor4f(1, 1, 1, 1-time);

		DrawStaticBG();

		glDisable(GL_BLEND);
		Texture::Enable(true);
	}

	if(time > 18)
	{
		glEnable(GL_BLEND);
		glBlendFunc(GL_ZERO, GL_ONE_MINUS_SRC_ALPHA);
		glColor4f(1, 1, 1, time-40);
		Texture::Enable(false);
		DrawStaticBG();
		Texture::Enable(true);
		glDisable(GL_BLEND);
	}

	oglBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
	rendtotex(time);


}