#define WIN32_LEAN_AND_MEAN
#define WIN32_EXTRA_LEAN
#include <windows.h>
#include <objidl.h>
#include <gdiplus.h>
using namespace Gdiplus;

#include <string.h>
#include "ddraw.h"
#include "msys.h"

//--- d a t a ---------------------------------------------------------------
#include "msys_glext.h"


//--- c o d e ---------------------------------------------------------------

int msys_glextInit( void )
{
	GdiplusStartupInput gdiplusStartupInput;
	ULONG_PTR           gdiplusToken;
	// Initialize GDI+.
	GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);
	return !gl3wInit();
}

unsigned char *LoadImageMemory(unsigned char* data, int size, int * width, int * height){
	IStream* pStream;
	HRESULT hr;
	using namespace Gdiplus;

	HGLOBAL hMem = ::GlobalAlloc(GMEM_MOVEABLE, size);
	if (!hMem)return 0;
	LPVOID pImage = ::GlobalLock(hMem);
	if (!pImage)
		return 0;
	CopyMemory(pImage, data, size);
	if (::CreateStreamOnHGlobal(hMem, FALSE, &pStream) != S_OK)
		return 0;
	else
	{
		Bitmap *pBitmap = Bitmap::FromStream(pStream);   //FAILS on WIN32
		//pBitmap->RotateFlip(RotateNoneFlipY);
		int height2;
		int width2;
		*width = pBitmap->GetWidth();
		*height = pBitmap->GetHeight();
		width2 = *width;
		height2 = *height;
		int pitch = ((*width * 32 + 31) & ~31) >> 3;
		BitmapData data2;
		Gdiplus::Rect rect(0, 0, width2, height2);
		unsigned char* pixels = (GLubyte *)malloc(pitch * height2);
		memset(pixels, 0, pitch*height2);
		if (pBitmap->LockBits(&rect, ImageLockModeRead, PixelFormat32bppARGB, &data2) != Gdiplus::Ok)
			return 0;
		//ARGB to RGBA
		uint8_t *p = static_cast<uint8_t *>(data2.Scan0);
		for (int y = 0; y < height2; y++)
			for (int x = 0; x < width2; x++)
			{
				uint8_t tmp = p[2];
				p[2] = p[0];
				p[0] = tmp;
				p += 4;
			}
		if (data2.Stride == pitch)
		{
			memcpy(pixels, data2.Scan0, pitch * height2);
		}
		else
		{
			for (int i = 0; i < height2; ++i)
				memcpy(&pixels[i * pitch], &p[i * data2.Stride], pitch);
		}
		pBitmap->UnlockBits(&data2);
		//image is now in RGBA
		delete[] pBitmap;
		GlobalUnlock(hMem);
		GlobalFree(hMem);
		return pixels;
	}
}


GLuint loadTexMemory(unsigned char* data, int size,int * width, int * height,int blur){
	unsigned char* pixels = LoadImageMemory(data, size, width, height);
	GLuint texture;
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, blur ? GL_LINEAR : GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, blur ? GL_LINEAR : GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, *width, *height, 0,
		GL_RGBA, GL_UNSIGNED_BYTE, pixels);
	free(pixels);
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
	glGetProgramiv(vsId, GL_COMPILE_STATUS, &result); glGetShaderInfoLog(vsId, 1024, NULL, (char *)info);
	printf(info);
//	if( !result ) DebugBreak();
	glGetProgramiv(fsId, GL_COMPILE_STATUS, &result); glGetShaderInfoLog(fsId, 1024, NULL, (char *)info);
	printf(info);
//	if( !result ) DebugBreak();
	glGetProgramiv(pid[0], GL_LINK_STATUS, &result); glGetShaderInfoLog(pid[0], 1024, NULL, (char*)info);
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
