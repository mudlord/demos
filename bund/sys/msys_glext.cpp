#define WIN32_LEAN_AND_MEAN
#define WIN32_EXTRA_LEAN
#include <windows.h>

#include "../libretro.h"

#include <GL/gl.h>
#include <gl/GLU.h>
#include "glext.h"

#include <string.h>
#include "ddraw.h"
#include "msys.h"
#include "msys_texgen.h"

//--- d a t a ---------------------------------------------------------------
#include "msys_glext.h"



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
	glColor4f(spr.rcol,spr.gcol,spr.bcol,spr.acol);
	//glColor4ub(spr.rcol,spr.gcol,spr.bcol,spr.acol);
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


//draw fullscreen quad
//useful for postproc
void DrawFullScreenQuad( int iWidth, int iHeight )
{
	glBegin(GL_QUADS);
	// Display the top left point of the 2D image
	glTexCoord2f(0.0f, 1.0f);	glVertex2f(0, 0);
	// Display the bottom left point of the 2D image
	//glTexCoord2f(0.0f, 0.0f);	glVertex2f(0, SCREEN_HEIGHT);
	glTexCoord2f(0.0f, 0.0f);	glVertex2f(0, iHeight);
	// Display the bottom right point of the 2D image
	//glTexCoord2f(1.0f, 0.0f);	glVertex2f(SCREEN_WIDTH, SCREEN_HEIGHT);
	glTexCoord2f(1.0f, 0.0f);	glVertex2f(iWidth, iHeight);
	// Display the top right point of the 2D image
	//glTexCoord2f(1.0f, 1.0f);	glVertex2f(SCREEN_WIDTH, 0);
	glTexCoord2f(1.0f, 1.0f);	glVertex2f(iWidth, 0);
	// Stop drawing
	glEnd();
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

void DrawStaticBG()
{
	BeginOrtho2D(1, 1);
	glDepthMask(GL_FALSE);

	glBegin(GL_QUADS);
	glTexCoord2f(0.0f, 0.0f); glVertex2f(0.0f, 0.0f);
	glTexCoord2f(1.0f, 0.0f); glVertex2f(1.0f, 0.0f);
	glTexCoord2f(1.0f, 1.0f); glVertex2f(1.0f, 1.0f);
	glTexCoord2f(0.0f, 1.0f); glVertex2f(0.0f, 1.0f);
	glEnd();

	glDepthMask(GL_TRUE);
	EndProjection();
}

void DrawShaky()
{
	BeginOrtho2D(1, 1);
	glDepthMask(GL_FALSE);

	float x = (rand()%10)/100.0f;
	float y = (rand()%10)/100.0f;

	glTranslatef(-x, -y, 0);

	glBegin(GL_QUADS);
	glTexCoord2f(0.0f, 0.0f); glVertex2f(0.0f, 0.0f);
	glTexCoord2f(1.0f, 0.0f); glVertex2f(1.2f, 0.0f);
	glTexCoord2f(1.0f, 1.0f); glVertex2f(1.2f, 1.2f);
	glTexCoord2f(0.0f, 1.0f); glVertex2f(0.0f, 1.2f);
	glEnd();

	glDepthMask(GL_TRUE);
	EndProjection();
}

// Draws a background with a rotating cube viewed at a
// very high field-of-view
void DrawRot(float time)
{
	BeginPerspective(140, 1.33f);
	glDepthMask(GL_FALSE);

	glRotatef(time*32, 1, 0, 0);
	glRotatef(time*67, 0, 1, 0);

	glBegin(GL_QUADS);
	// Front
	glNormal3f(0, 0, 1);
	glTexCoord2f(0, 0); glVertex3f(-1, -1, -1);
	glTexCoord2f(1, 0); glVertex3f( 1, -1, -1);
	glTexCoord2f(1, 1); glVertex3f( 1,  1, -1);
	glTexCoord2f(0, 1); glVertex3f(-1,  1, -1);

	// Left
	glNormal3f(1, 0, 0);
	glTexCoord2f(0, 0); glVertex3f(-1, -1,  1);
	glTexCoord2f(1, 0); glVertex3f(-1, -1, -1);
	glTexCoord2f(1, 1); glVertex3f(-1,  1, -1);
	glTexCoord2f(0, 1); glVertex3f(-1,  1,  1);

	// Bottom
	glNormal3f(0, 1, 0);
	glTexCoord2f(0, 0); glVertex3f(-1, -1,  1);
	glTexCoord2f(1, 0); glVertex3f( 1, -1,  1);
	glTexCoord2f(1, 1); glVertex3f( 1, -1, -1);
	glTexCoord2f(0, 1); glVertex3f(-1, -1, -1);

	// Back
	glNormal3f(0, 0, -10);
	glTexCoord2f(0, 0); glVertex3f( 1, -1,  1);
	glTexCoord2f(1, 0); glVertex3f(-1, -1,  1);
	glTexCoord2f(1, 1); glVertex3f(-1,  1,  1);
	glTexCoord2f(0, 1); glVertex3f( 1,  1,  1);

	// Right
	glNormal3f(0, -1, 0);
	glTexCoord2f(0, 0); glVertex3f( 1, -1, -1);
	glTexCoord2f(1, 0); glVertex3f( 1, -1,  1);
	glTexCoord2f(1, 1); glVertex3f( 1,  1,  1);
	glTexCoord2f(0, 1); glVertex3f( 1,  1, -1);

	// Top
	glNormal3f(0, -1, 0);
	glTexCoord2f(0, 0); glVertex3f(-1,  1,  1);
	glTexCoord2f(1, 0); glVertex3f( 1,  1,  1);
	glTexCoord2f(1, 1); glVertex3f( 1,  1, -1);
	glTexCoord2f(0, 1); glVertex3f(-1,  1, -1);

	glEnd();

	glDepthMask(GL_TRUE);
	EndProjection();
}

// Draws a background rotating around its center
void DrawRotateTile(float rot)
{
	BeginOrtho2D(-1, -1, 1, 1);
	glDepthMask(GL_FALSE);

	glRotatef(rot, 0, 0, 1);

	glBegin(GL_QUADS);
	glTexCoord2f(0.0f, 0.0f); glVertex2f(0.0f, 0.0f);
	glTexCoord2f(1.0f, 0.0f); glVertex2f(1.0f, 0.0f);
	glTexCoord2f(1.0f, 1.0f); glVertex2f(1.0f, 1.0f);
	glTexCoord2f(0.0f, 1.0f); glVertex2f(0.0f, 1.0f);

	glTexCoord2f(0.0f, 0.0f); glVertex2f(0.0f, 0.0f);
	glTexCoord2f(1.0f, 0.0f); glVertex2f(-1.0f, 0.0f);
	glTexCoord2f(1.0f, 1.0f); glVertex2f(-1.0f, 1.0f);
	glTexCoord2f(0.0f, 1.0f); glVertex2f(0.0f, 1.0f);

	glTexCoord2f(0.0f, 0.0f); glVertex2f(0.0f, 0.0f);
	glTexCoord2f(1.0f, 0.0f); glVertex2f(-1.0f, 0.0f);
	glTexCoord2f(1.0f, 1.0f); glVertex2f(-1.0f, -1.0f);
	glTexCoord2f(0.0f, 1.0f); glVertex2f(0.0f, -1.0f);

	glTexCoord2f(0.0f, 0.0f); glVertex2f(0.0f, 0.0f);
	glTexCoord2f(1.0f, 0.0f); glVertex2f(1.0f, 0.0f);
	glTexCoord2f(1.0f, 1.0f); glVertex2f(1.0f, -1.0f);
	glTexCoord2f(0.0f, 1.0f); glVertex2f(0.0f, -1.0f);
	glEnd();

	glDepthMask(GL_TRUE);
	EndProjection();
}

// Draw a sine distorted background
void DrawDistort(float time)
{
	BeginOrtho2D(-1, -1, 1, 1);
	glScalef(1.2f, 1.2f, 1.0f);

	glDepthMask(GL_FALSE);

	//time *= ;

	float step = 0.05f;
	float kx = 0.04f;
	float ky = 0.03f;

	float ax = 14.3f;
	float bx = 7.2f;
	float cx = 9.12f;
	float dx = 21.7f;

	for(float a = -1; a < 1; a += step)
	{
		glBegin(GL_TRIANGLE_STRIP);
		for(float b = -1; b <= 1.01f; b += step)
		{
			glTexCoord2f(b*0.5f+0.5f, a*0.5f+0.5f);
			glVertex2f(b + sinf(b*dx+time*bx) * kx, a + sinf(a*ax+time*0.7f*cx) * ky);

			glTexCoord2f(b*0.5f+0.5f, (a+step)*0.5f+0.5f);
			glVertex2f(b + sinf(b*dx+time*bx) * kx, a + step + sinf((a+step)*ax+time*0.7f*cx) * ky);
		}
		glEnd();
	}

	glDepthMask(GL_TRUE);
	EndProjection();
}

// Draw a waving background
void DrawWaving(float time)
{
	BeginOrtho2D(-1, -1, 1, 1);
	glScalef(1.2f, 1.2f, 1.0f);

	glDepthMask(GL_FALSE);

	time *= 0.5f;

	float step = 0.05f;
	float k = 0.2f;

	for(float a = -1; a < 1; a += step)
	{
		glBegin(GL_TRIANGLE_STRIP);
		for(float b = -1; b <= 1.01f; b += step)
		{
			glTexCoord2f(b*0.5f+0.5f, a*0.5f+0.5f);
			glVertex2f(b + sinf(a+time) * k * (sqrtf(2) - sqrtf(a*a+b*b)), a + sinf(b+time*0.7f) * k * (sqrtf(2) - sqrtf(a*a+b*b)));

			glTexCoord2f(b*0.5f+0.5f, (a+step)*0.5f+0.5f);
			glVertex2f(b + sinf(a+step+time) * k *  (sqrtf(2) - sqrtf((a+step)*(a+step)+b*b)), a + step + sinf(b+time*0.7f) * k * (sqrtf(2) - sqrtf((a+step)*(a+step)+b*b)));
		}
		glEnd();
	}

	glDepthMask(GL_TRUE);
	EndProjection();
}

// And finally the zooming background
void DrawZooming(float time)
{
	BeginOrtho2D(1, 1);
	glDepthMask(GL_FALSE);

	time *= 0.5f;

	glBegin(GL_QUADS);
	glTexCoord2f(0.0f+time, 0.0f+time); glVertex2f(0.0f, 0.0f);
	glTexCoord2f(1.0f-time, 0.0f+time); glVertex2f(1.0f, 0.0f);
	glTexCoord2f(1.0f-time, 1.0f-time); glVertex2f(1.0f, 1.0f);
	glTexCoord2f(0.0f+time, 1.0f-time); glVertex2f(0.0f, 1.0f);
	glEnd();

	glDepthMask(GL_TRUE);
	EndProjection();
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

struct DDS_IMAGE_DATA
{
	GLsizei  width;
	GLsizei  height;
	GLint    components;
	GLenum   format;
	int      numMipMaps;
	GLubyte *pixels;
};


GLuint loadTexGenTexMemory(unsigned char *data,int size, int width, int height)
{
	GLuint texture;
	RGBA *image = (RGBA*)calloc(width*height, sizeof(RGBA));
	TextureGenerator *tg=getTextureGenerator();
	tg->tex2mem(&image, data,size, width, height, true);
	glGenTextures( 1, &texture );
	glBindTexture( GL_TEXTURE_2D, texture );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
	glTexImage2D( GL_TEXTURE_2D, 0, 4, width, height, 0,
		GL_RGBA, GL_UNSIGNED_BYTE, image );
	free(image);
	return texture;
}

GLuint loadTexGenTex(char* tex_filename, int width, int height)
{
	GLuint texture;
	RGBA *image = (RGBA*)calloc(width*height, sizeof(RGBA));
	TextureGenerator *tg=getTextureGenerator();
	tg->tex2mem(&image, tex_filename, width, height, true);


	glGenTextures( 1, &texture );
	glBindTexture( GL_TEXTURE_2D, texture );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
	glTexImage2D( GL_TEXTURE_2D, 0, 4, width, height, 0,
		GL_RGBA, GL_UNSIGNED_BYTE, image );
	free(image);
	return texture;
}


typedef struct
{
  GLubyte Header[12];									// TGA File Header
} TGAHeader;
typedef struct
{
	GLubyte		header[6];								// First 6 Useful Bytes From The Header
	GLuint		bytesPerPixel;							// Holds Number Of Bytes Per Pixel Used In The TGA File
	GLuint		imageSize;								// Used To Store The Image Size When Setting Aside Ram
	GLuint		temp;									// Temporary Variable
	GLuint		type;
	GLuint		Height;									//Height of Image
	GLuint		Width;									//Width ofImage
	GLuint		Bpp;									// Bits Per Pixel
} TGA;
TGAHeader tgaheader;									// TGA header
TGA tga;												// TGA image data
GLubyte uTGAcompare[12] = {0,0,2, 0,0,0,0,0,0,0,0,0};	// Uncompressed TGA Header
GLubyte cTGAcompare[12] = {0,0,10,0,0,0,0,0,0,0,0,0};	// Compressed TGA Header

GLuint loadTGATextureFile(const char* filename, bool blur)
{
	GLuint texture;
	FILE *filep = fopen(filename,"rb");
	long size;
	fseek (filep, 0, SEEK_END);   // non-portable
	size=ftell (filep);
	fseek (filep, 0, SEEK_SET);   // non-portable
	unsigned char *buffer = new unsigned char[size];
	int bytes = fread( buffer, 1, size, filep );
	texture = loadTGATextureMemory(buffer,size,blur);
		fclose( filep );
	delete [] buffer;
	return texture;
}



GLuint loadTGATextureMemory(unsigned char* data, int size, bool blur)
{
	GLuint texture;
	glGenTextures( 1, &texture );
	glBindTexture( GL_TEXTURE_2D, texture);
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, blur?GL_LINEAR:GL_NEAREST  );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,blur?GL_LINEAR:GL_NEAREST  );
	BUF *fTGA = bufopen(data, size);
	bufread((void*)&tgaheader,sizeof(TGAHeader),1,fTGA);
	if(memcmp(uTGAcompare, &tgaheader, sizeof(tgaheader)) == 0)				// See if header matches the predefined header of
	{
		bufread(tga.header, sizeof(tga.header), 1, fTGA);
		int width  = tga.header[1] * 256 + tga.header[0];					// Determine The TGA Width	(highbyte*256+lowbyte)
		int height = tga.header[3] * 256 + tga.header[2];					// Determine The TGA Height	(highbyte*256+lowbyte)
		int bpp	= tga.header[4];										// Determine the bits per pixel
		int textype;
		if(bpp == 24)													// If the BPP of the image is 24...
			textype	= GL_RGB;											// Set Image type to GL_RGB
		else																	// Else if its 32 BPP
			textype	= GL_RGBA;
		int bytesperpixel	= (bpp / 8);									// Compute the number of BYTES per pixel
		int imagesize		= (bytesperpixel * width * height);		// Compute the total amout ofmemory needed to store data
		unsigned char* imagedata = (GLubyte *)malloc(imagesize);
		bufread(imagedata, 1, imagesize, fTGA);
		for(GLuint cswap = 0; cswap < (int)imagesize; cswap += bytesperpixel)
		{
			imagedata[cswap] ^= imagedata[cswap+2] ^=
				imagedata[cswap] ^= imagedata[cswap+2];
		}
		glTexImage2D(GL_TEXTURE_2D, 0, textype, width, height, 0, textype, GL_UNSIGNED_BYTE, imagedata);
		free(imagedata);

		return texture;
	}
	bufclose(fTGA);
	return NULL;

}




GLuint loadMIFTextureFile(const char* filename, bool blur)
{
	GLuint texture;
	FILE *filep = fopen(filename,"rb");
	long size;
	fseek (filep, 0, SEEK_END);   // non-portable
	size=ftell (filep);
	fseek (filep, 0, SEEK_SET);   // non-portable
	unsigned char *buffer = new unsigned char[size];
	int bytes = fread( buffer, 1, size, filep );
	texture = loadMIFTextureMemory(buffer,size,blur);
	fclose( filep );
	delete [] buffer;
	return texture;
}

GLuint loadMIFTextureMemory(unsigned char* data, int size, bool blur)
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

	unsigned char * colortable = new unsigned char[width * height * 2];
	ZeroMemory(colortable,sizeof(colortable));
	bufread(colortable,width*height*2, 1, fd);
	bufclose(fd);

	unsigned char *texture = new unsigned char[width * height * 4];
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

GLuint loadDDSTextureMemory(unsigned char* data, int size, bool blur)
{
	GLuint texture;
	DDS_IMAGE_DATA *pDDSImageData;
	DDSURFACEDESC2 ddsd;
	char filecode[4] = {0};
	int factor;
	int bufferSize;
	unsigned char* dataptr = data;
	memcpy(filecode,dataptr,4);
	dataptr+=4;
	if( strncmp( filecode, "DDS ", 4 ) != 0 )
	{
		char str[255];
		sprintf( str, "%s","The file doesn't appear to be a valid .dds file!");
		MessageBox( NULL, str, "ERROR", MB_OK|MB_ICONEXCLAMATION );
		return NULL;
	}
	memcpy( &ddsd,dataptr, sizeof(ddsd));
	dataptr+=sizeof(ddsd);

	pDDSImageData = (DDS_IMAGE_DATA*) malloc(sizeof(DDS_IMAGE_DATA));
	memset( pDDSImageData, 0, sizeof(DDS_IMAGE_DATA) );
	switch( ddsd.ddpfPixelFormat.dwFourCC )
	{
	case FOURCC_DXT1:
		// DXT1's compression ratio is 8:1
		pDDSImageData->format = GL_COMPRESSED_RGBA_S3TC_DXT1_EXT;
		factor = 2;
		break;

	case FOURCC_DXT3:
		// DXT3's compression ratio is 4:1
		pDDSImageData->format = GL_COMPRESSED_RGBA_S3TC_DXT3_EXT;
		factor = 4;
		break;

	case FOURCC_DXT5:
		// DXT5's compression ratio is 4:1
		pDDSImageData->format = GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
		factor = 4;
		break;
	default:
		char str[255];
		sprintf( str, "%s","The file doesn't appear to be compressed using DXT1, DXT3, or DXT5!");
		MessageBox( NULL, str, "ERROR", MB_OK|MB_ICONEXCLAMATION );
		return NULL;
	}
	//
	// How big will the buffer need to be to load all of the pixel data
	// including mip-maps?
	//

	if( ddsd.dwLinearSize == 0 )
	{
		MessageBox( NULL, "dwLinearSize is 0!","ERROR",
			MB_OK|MB_ICONEXCLAMATION);
	}

	if( ddsd.dwMipMapCount > 1 )
		bufferSize = ddsd.dwLinearSize * factor;
	else
		bufferSize = ddsd.dwLinearSize;

	pDDSImageData->pixels = (unsigned char*)malloc(bufferSize * sizeof(unsigned char));
	memcpy(pDDSImageData->pixels,dataptr,bufferSize);

	pDDSImageData->width      = ddsd.dwWidth;
	pDDSImageData->height     = ddsd.dwHeight;
	pDDSImageData->numMipMaps = ddsd.dwMipMapCount;

	if( ddsd.ddpfPixelFormat.dwFourCC == FOURCC_DXT1 )
		pDDSImageData->components = 3;
	else
		pDDSImageData->components = 4;


	if( pDDSImageData != NULL )
	{
		int nHeight     = pDDSImageData->height;
		int nWidth      = pDDSImageData->width;
		int nNumMipMaps = pDDSImageData->numMipMaps;

		int nBlockSize;

		if( pDDSImageData->format == GL_COMPRESSED_RGBA_S3TC_DXT1_EXT )
			nBlockSize = 8;
		else
			nBlockSize = 16;

		glGenTextures( 1, &texture );
		glBindTexture( GL_TEXTURE_2D, texture);

		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, blur?GL_LINEAR:GL_NEAREST  );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, blur?GL_LINEAR:GL_NEAREST );

		int nSize;
		int nOffset = 0;

		if (nNumMipMaps == 0)
		{
			if( nWidth  == 0 ) nWidth  = 1;
			if( nHeight == 0 ) nHeight = 1;
			nSize = ((nWidth+3)/4) * ((nHeight+3)/4) * nBlockSize;
			glCompressedTexImage2DARB( GL_TEXTURE_2D,
				0,
				pDDSImageData->format,
				nWidth,
				nHeight,
				0,
				nSize,
				pDDSImageData->pixels + nOffset );

			nOffset += nSize;
		}
		else
		{
			for( int i = 0; i < nNumMipMaps; ++i )
			{
				if( nWidth  == 0 ) nWidth  = 1;
				if( nHeight == 0 ) nHeight = 1;
				nSize = ((nWidth+3)/4) * ((nHeight+3)/4) * nBlockSize;
				glCompressedTexImage2DARB( GL_TEXTURE_2D,
					i,
					pDDSImageData->format,
					nWidth,
					nHeight,
					0,
					nSize,
					pDDSImageData->pixels + nOffset );

				nOffset += nSize;

				// Half the image size for the next mip-map level...
				nWidth  = (nWidth  / 2);
				nHeight = (nHeight / 2);
			}
		}


	}
	if( pDDSImageData != NULL )
	{
		if( pDDSImageData->pixels != NULL )
			free( pDDSImageData->pixels );

		free( pDDSImageData );
	}
	return texture;
}


#include <sys/stat.h>
unsigned char *readShaderFile( const char *fileName )
{
	FILE *file = fopen( fileName, "r" );
	long size;
	if( file == NULL )
	{
		MessageBox( NULL, "Cannot open shader file!", "ERROR",
			MB_OK | MB_ICONEXCLAMATION );
		return 0;
	}
	fseek (file, 0, SEEK_END);   // non-portable
	size=ftell (file);
	fseek (file, 0, SEEK_SET);   // non-portable
	unsigned char *buffer = new unsigned char[size];
	int bytes = fread( buffer, 1, size, file );
	buffer[bytes] = 0;
	fclose( file );
	return buffer;
}


void initShader( int *pid, const char *vs, const char *fs )
{
	pid[0] = oglCreateProgram();
	const int vsId = oglCreateShader( GL_VERTEX_SHADER );
	const int fsId = oglCreateShader( GL_FRAGMENT_SHADER );
	oglShaderSource( vsId, 1, &vs, 0 );
	oglShaderSource( fsId, 1, &fs, 0 );
	oglCompileShader( vsId );
	oglCompileShader( fsId );
	oglAttachShader( pid[0], fsId );
	oglAttachShader( pid[0], vsId );
	oglLinkProgram( pid[0] );


	int		result;
	char    info[1536];
	oglGetObjectParameteriv( vsId,   GL_OBJECT_COMPILE_STATUS_ARB, &result ); oglGetInfoLog( vsId,   1024, NULL, (char *)info ); if( !result ) DebugBreak();
	oglGetObjectParameteriv( fsId,   GL_OBJECT_COMPILE_STATUS_ARB, &result ); oglGetInfoLog( fsId,   1024, NULL, (char *)info ); if( !result ) DebugBreak();
	oglGetObjectParameteriv( pid[0], GL_OBJECT_LINK_STATUS_ARB,    &result ); oglGetInfoLog( pid[0], 1024, NULL, (char*)info ); if( !result ) DebugBreak();
}

void start_fbo(GLuint fbo, int width, int height, bool twod)
{
	oglBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fbo);
	glPushAttrib(GL_VIEWPORT_BIT);
	glViewport(0, 0, width, height);
}

void end_fbo()
{
	glPopAttrib();
	oglBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
}

void draw_fbotexture(GLuint texture,int width, int height )
{
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear (GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);		// Clear Screen And Depth Buffer
	glEnable(GL_TEXTURE_2D);
	glDisable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
	BeginOrtho2D(width,height,false);
	glBindTexture( GL_TEXTURE_2D,  texture);
	DrawFullScreenQuad(width,height);
	glBindTexture( GL_TEXTURE_2D,  0);
	glDisable(GL_BLEND);
	glEnable(GL_DEPTH_TEST);
	EndProjection();
}

FBOELEM init_fbo(int width, int height)
{
	FBOELEM elem = {0};
	int current, enderr = 1;
	GLuint error = 0;
	oglGenFramebuffersEXT (1, &elem.fbo);
	oglBindFramebufferEXT (GL_FRAMEBUFFER_EXT, elem.fbo);
	oglGenRenderbuffersEXT (1, &elem.depthbuffer);
	oglBindRenderbufferEXT (GL_RENDERBUFFER_EXT,elem.depthbuffer);
	oglRenderbufferStorageEXT (GL_RENDERBUFFER_EXT, GL_DEPTH_COMPONENT, width, height);
	oglFramebufferRenderbufferEXT (GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GL_RENDERBUFFER_EXT, elem.depthbuffer);
	glGenTextures (1, &elem.texture);
	glBindTexture (GL_TEXTURE_2D, elem.texture);
	glTexParameteri (GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri (GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexImage2D (GL_TEXTURE_2D, 0, GL_RGBA8,  width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	oglFramebufferTexture2DEXT (GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, elem.texture, 0);

	// check if everything was ok with our requests above.
	error = oglCheckFramebufferStatusEXT (GL_FRAMEBUFFER_EXT);
	if (error != GL_FRAMEBUFFER_COMPLETE_EXT) {
		FBOELEM err = {0};
		elem.status = 0;
		enderr = 0;
		return err;
	}
	elem.status = 1;
	// set Rendering Device to screen again.
	glBindTexture(GL_TEXTURE_2D,0);
	oglBindFramebufferEXT (GL_FRAMEBUFFER_EXT, 0);
	oglBindRenderbufferEXT (GL_RENDERBUFFER_EXT, 0);
	return elem;
}

GLuint init_rendertexture(int resx, int resy)
{
	GLuint texture;
	//texture
	glEnable(GL_TEXTURE_2D);
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D,  texture);
	glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, resx, resy, 0, GL_RGBA,
		GL_UNSIGNED_BYTE, NULL);
	glCopyTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,0,0,resx,resy,0);
	glBindTexture(GL_TEXTURE_2D,  0);
	return texture;
}
