#define WIN32_LEAN_AND_MEAN
#define WIN32_EXTRA_LEAN
#include <windows.h>

#include <string.h>
#include "ddraw.h"
#include "msys.h"

//--- d a t a ---------------------------------------------------------------
#include "msys_glext.h"


//--- c o d e ---------------------------------------------------------------

int msys_glextInit( void )
{
	return !gl3wInit();
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

GLuint loadTGATextureMemory(unsigned char* data, int size, bool blur)
{	
	GLuint texture;
	glGenTextures( 1, &texture );
	glBindTexture( GL_TEXTURE_2D, texture);
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, blur?GL_LINEAR:GL_NEAREST  );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,blur?GL_LINEAR:GL_NEAREST  );
	MEM *fTGA = mopen((int8_t*)data, size);
	mread((void*)&tgaheader,sizeof(TGAHeader),1,fTGA);
	if(memcmp(uTGAcompare, &tgaheader, sizeof(tgaheader)) == 0)				// See if header matches the predefined header of 
	{				
		mread(tga.header, sizeof(tga.header), 1, fTGA);
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
		mread(imagedata, 1, imagesize, fTGA);
		for(GLuint cswap = 0; cswap < (int)imagesize; cswap += bytesperpixel)
		{
			imagedata[cswap] ^= imagedata[cswap+2] ^=
				imagedata[cswap] ^= imagedata[cswap+2];
		}
		glTexImage2D(GL_TEXTURE_2D, 0, textype, width, height, 0, textype, GL_UNSIGNED_BYTE, imagedata);
		free(imagedata);
		mclose(fTGA);
		return texture;
	}
	mclose(fTGA);
	return NULL;

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
	pid[0] = glCreateProgram();                           
	const int vsId = glCreateShader( GL_VERTEX_SHADER ); 
	const int fsId = glCreateShader( GL_FRAGMENT_SHADER );
	glShaderSource( vsId, 1, &vs, 0 );
	glShaderSource( fsId, 1, &fs, 0 );
	glCompileShader( vsId );
	glCompileShader( fsId );
	glAttachShader( pid[0], fsId );
	glAttachShader( pid[0], vsId );
	glLinkProgram( pid[0] );


	int		result;
	char    info[1536];
	glGetShaderiv( vsId,   GL_COMPILE_STATUS, &result ); glGetShaderInfoLog( vsId,   1024, NULL, (char *)info );
	printf(info);
//	if( !result ) DebugBreak();
	glGetShaderiv( fsId,   GL_COMPILE_STATUS, &result );glGetShaderInfoLog( fsId,   1024, NULL, (char *)info );
	printf(info);
//	if( !result ) DebugBreak();
	glGetShaderiv( pid[0], GL_LINK_STATUS,    &result ); glGetShaderInfoLog( pid[0], 1024, NULL, (char*)info );
	printf(info);
	//if( !result ) DebugBreak();
}

FBOELEM init_fbo(int width, int height, bool fp)
{
	FBOELEM elem = {0};
	int current, enderr = 1;
	GLuint error = 0;
	glGenFramebuffers(1, &elem.fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, elem.fbo);
	glGenRenderbuffers(1, &elem.depthbuffer);
	glBindRenderbuffer (GL_RENDERBUFFER,elem.depthbuffer);
	glRenderbufferStorage (GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height);
	glFramebufferRenderbuffer (GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, elem.depthbuffer);
	glGenTextures (1, &elem.texture);
	glBindTexture (GL_TEXTURE_2D, elem.texture);
	glTexParameteri (GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri (GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexImage2D (GL_TEXTURE_2D, 0, fp?GL_RGB32F:GL_RGBA8,  width, height, 0, GL_RGBA,fp?GL_FLOAT: GL_UNSIGNED_BYTE, NULL);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, elem.texture, 0);

	// check if everything was ok with our requests above.
	error = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (error != GL_FRAMEBUFFER_COMPLETE) {
		FBOELEM err = {0};
		elem.status = 0;
		enderr = 0;
		return err;
	}
	elem.status = 1;
	// set Rendering Device to screen again.
	glBindTexture(GL_TEXTURE_2D,0);
	glBindFramebuffer (GL_FRAMEBUFFER, 0);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);
	return elem;
}
