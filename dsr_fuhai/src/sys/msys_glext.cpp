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

static char *funcs[] = {
"glActiveTexture",
"glCompressedTexImage2D",
"glCompressedTexSubImage2D",
"glGetCompressedTexImage",
"glBlendFuncSeparate",
"glBindBuffer",
"glDeleteBuffers",
"glGenBuffers",
"glIsBuffer",
"glBufferData",
"glBufferSubData",
"glGetBufferSubData",
"glMapBuffer",
"glUnmapBuffer",
"glGetBufferParameteriv",
"glGetBufferPointerv",
"glBlendEquationSeparate",
"glDrawBuffers",
"glStencilOpSeparate",
"glStencilFuncSeparate",
"glStencilMaskSeparate",
"glBindAttribLocation",
"glDisableVertexAttribArray",
"glEnableVertexAttribArray",
"glGetAttribLocation",
"glGetUniformLocation",
"glVertexAttribPointer",
"glClampColor",
"glBindFragDataLocation",
"glGetFragDataLocation",
"glFramebufferTexture",
"glIsRenderbuffer",
"glBindRenderbuffer",
"glDeleteRenderbuffers",
"glGenRenderbuffers",
"glRenderbufferStorage",
"glGetRenderbufferParameteriv",
"glIsFramebuffer",
"glBindFramebuffer",
"glDeleteFramebuffers",
"glGenFramebuffers",
"glCheckFramebufferStatus",
"glFramebufferTexture2D",
"glFramebufferRenderbuffer",
"glGetFramebufferAttachmentParameteriv",
"glGenerateMipmap",
"glBlitFramebuffer",
"glBindVertexArray",
"glDeleteVertexArrays",
"glGenVertexArrays",
"glCreateShaderProgramv",
"glGenProgramPipelines",
"glBindProgramPipeline",
"glUseProgramStages",
"glProgramUniform4fv",
"glProgramUniform1i",
"glProgramUniformMatrix4fv",
"glGetProgramResourceLocation",
#ifdef DEBUG
"glGetProgramiv",
"glGetProgramInfoLog",
#endif

};
static HMODULE libgl;
void *msys_oglfunc[NUMFUNCS];

#define LOAD_ENTRYPOINT(name, var, type) \
    if (!var) \
		    { \
        var = reinterpret_cast<type>(wglGetProcAddress(name)); \
		    }

//--- c o d e ---------------------------------------------------------------

int msys_glextInit( void )
{
	GdiplusStartupInput gdiplusStartupInput;
	ULONG_PTR           gdiplusToken;
	// Initialize GDI+.
	GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);
	for (int i = 0; i <NUMFUNCS; i++)
	{
		msys_oglfunc[i] = wglGetProcAddress(funcs[i]);
		if (!msys_oglfunc[i])
		return(0);	
	}
	return 1;
}

HGLRC wglCreateContextAttribsARB(HDC hDC, HGLRC hShareContext, const int *attribList)
{
	// An OpenGL 3.1 rendering context is created using the new
	// wglCreateContextAttribsARB() function. This new function was introduced
	// in OpenGL 3.0 to maintain backwards compatibility with existing OpenGL
	// 2.1 and older applications. To create an OpenGL 3.1 rendering context
	// first create an OpenGL 2.1 or older rendering context using the
	// wglCreateContext() function. Activate the context and then call the new
	// wglCreateContextAttribsARB() function to create an OpenGL 3.1 rendering
	// context. Once the context is created activate it to enable OpenGL 3.1
	// functionality.
	//
	// For further details see:
	// http://www.opengl.org/registry/specs/ARB/wgl_create_context.txt

	typedef HGLRC(APIENTRY * PFNWGLCREATECONTEXTATTRIBSARBPROC)(HDC hDC, HGLRC hShareContext, const int *attribList);
	static PFNWGLCREATECONTEXTATTRIBSARBPROC pfnCreateContextAttribsARB = 0;

	HGLRC hContext = 0;
	HGLRC hCurrentContext;
	if (!(hCurrentContext = wglCreateContext(hDC)))
	return 0;

	if (!wglMakeCurrent(hDC, hCurrentContext))
	{
		wglDeleteContext(hCurrentContext);
		return 0;
	}

	LOAD_ENTRYPOINT("wglCreateContextAttribsARB", pfnCreateContextAttribsARB, PFNWGLCREATECONTEXTATTRIBSARBPROC);

        if (pfnCreateContextAttribsARB)
	hContext = pfnCreateContextAttribsARB(hDC, hShareContext, attribList);
	if (!hContext)return 0;

	wglMakeCurrent(hDC, 0);
	wglDeleteContext(hCurrentContext);
	return hContext;
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


shader_id initShader(const char *vsh, const char *fsh)
{
	shader_id shad = { 0 };
	shad.vsid = glCreateShaderProgramv(GL_VERTEX_SHADER, 1, &vsh);
	shad.fsid = glCreateShaderProgramv(GL_FRAGMENT_SHADER, 1, &fsh);
	glGenProgramPipelines(1, &shad.pid);
	glBindProgramPipeline(shad.pid);
	glUseProgramStages(shad.pid, GL_VERTEX_SHADER_BIT, shad.vsid);
	glUseProgramStages(shad.pid, GL_FRAGMENT_SHADER_BIT, shad.fsid);
#ifdef DEBUG
	int		result;
	char    info[1536];
	glGetProgramiv(shad.vsid, GL_LINK_STATUS, &result); glGetProgramInfoLog(shad.vsid, 1024, NULL, (char *)info); if (!result) DebugBreak();
	glGetProgramiv(shad.fsid, GL_LINK_STATUS, &result); glGetProgramInfoLog(shad.fsid, 1024, NULL, (char *)info); if (!result) DebugBreak();
	glGetProgramiv(shad.pid, GL_LINK_STATUS, &result); glGetProgramInfoLog(shad.pid, 1024, NULL, (char *)info); if (!result) DebugBreak();
#endif
	glBindProgramPipeline(0);
	return shad;
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
