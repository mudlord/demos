#define WIN32_LEAN_AND_MEAN
#define WIN32_EXTRA_LEAN
#include <windows.h>


#include <GL/gl.h>
#include <gl/GLU.h>
#include "glext.h"

#include <string.h>
#include "ddraw.h"
#include "msys.h"

//--- d a t a ---------------------------------------------------------------
#include "msys_glext.h"

static char *funciones = {
	// multitexture
	"wglSwapIntervalEXT\x0"
};

void *msys_oglfunc[NUMFUNCIONES];

//--- c o d e ---------------------------------------------------------------

int msys_glextInit( void )
{
    char *str = funciones;
    for( int i=0; i<NUMFUNCIONES; i++ )
        {
        msys_oglfunc[i] = wglGetProcAddress( str );
        str += 1+strlen( str );

        if( !msys_oglfunc[i] )
			return( 0 );
        }


    return( 1 );
}

// Sets an orthographic projection. x: 0..width, y: 0..height 
void BeginOrtho2D(float width, float height, bool flip_y) 
{ 
	glMatrixMode(GL_PROJECTION); 
	glPushMatrix(); 
	glLoadIdentity(); 
	if (!flip_y)
	{
		gluOrtho2D(0, width, 0, height); 
	}
	else
	{
	    glOrtho(0, width, height, 0, -1, 1);
	}
	
	
	glMatrixMode(GL_MODELVIEW); 
	glPushMatrix(); 
	glLoadIdentity(); 
} 

// Sets an orthographic projection. x: x1..x2, y: y1..y2 
void BeginOrtho2D(float x1, float y1, float x2, float y2) 
{ 
	glMatrixMode(GL_PROJECTION); 
	glPushMatrix(); 
	glLoadIdentity(); 
	gluOrtho2D(x1, x2, y1, y2); 
	glMatrixMode(GL_MODELVIEW); 
	glPushMatrix(); 
	glLoadIdentity(); 
} 

void draw_sprite(sprite spr, int xres, int yres, bool flip_y)
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
	//glColor4f(spr.rcol,spr.gcol,spr.bcol,spr.acol);
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


world_coords GetWorldCoords(int x, int y)
{
	world_coords coords;
	double objX, objY, objZ;//holder for world coordinates
	GLint view[4];//viewport dimensions+pos
	GLdouble  p[16];//projection matrix
	GLdouble  m[16];//modelview matrix
	GLdouble z = 10.0;//Z-Buffer Value?
	glGetDoublev (GL_MODELVIEW_MATRIX, m);
	glGetDoublev (GL_PROJECTION_MATRIX,p);
	glGetIntegerv( GL_VIEWPORT, view );
	//view[3]-cursorY = conversion from upper left (0,0) to lower left (0,0)
	//Unproject 2D Screen coordinates into wonderful world coordinates
	gluUnProject(x, view[3]-y, 1, m, p, view, &objX, &objY, &objZ);
	objX /= z;
	objY /= z;

	coords.x = objX;
	coords.y = objY;
	return coords;
}

screenspace_coords GetScreenCoords(double fx, double fy, double fz)
{
	screenspace_coords coords;
	GLdouble x, y;
	GLint view[4];//viewport dimensions+pos
	GLdouble  p[16];//projection matrix
	GLdouble  m[16];//modelview matrix
	GLdouble z = 10.0;//Z-Buffer Value?
	glGetDoublev (GL_MODELVIEW_MATRIX, m);
	glGetDoublev (GL_PROJECTION_MATRIX,p);
	glGetIntegerv( GL_VIEWPORT, view );

	gluProject(fx, fy, fz,
		m,p,view,
		&x, &y, &z);
	x /= z;
	y /= z;
	coords.x = (int)x;
	coords.y = (int)y;

	return coords;
}

// Sets a perspective projection with a different field-of-view 
void BeginPerspective(float fovy, float xratio) 
{ 
	glMatrixMode(GL_PROJECTION); 
	glPushMatrix(); 
	glLoadIdentity(); 
	gluPerspective(fovy, xratio, 0.1f, 100.0f); 
	glMatrixMode(GL_MODELVIEW); 
	glPushMatrix(); 
	glLoadIdentity(); 
} 

// Restore the previous projection 
void EndProjection() 
{ 
	glMatrixMode(GL_PROJECTION); 
	glPopMatrix(); 
	glMatrixMode(GL_MODELVIEW); 
	glPopMatrix(); 
} 
void Resize(int x, int y) 
{ 
	// Prevent div by zero 
	y = y ? y : 1; 

	// Resize viewport 
	glViewport(0, 0, x, y); 

	// Recalc perspective 
	glMatrixMode(GL_PROJECTION); 
	glLoadIdentity(); 
	gluPerspective(60.0f, static_cast<float>(x)/static_cast<float>(y), 0.1f, 100.0f); 
	glMatrixMode(GL_MODELVIEW); 
	glLoadIdentity(); 
} 

