#define WIN32_LEAN_AND_MEAN
#define WIN32_EXTRA_LEAN
#include <windows.h>
#include <objidl.h>
#include <gdiplus.h>
using namespace Gdiplus;

#include <string.h>
#include "musys.h"

//--- d a t a ---------------------------------------------------------------
#include "musys_gl.h"

size_t font_len = 507;
unsigned char font[507] =
{
	0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A, 0x00, 0x00, 0x00,
	0x0D, 0x49, 0x48, 0x44, 0x52, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00,
	0x00, 0x60, 0x02, 0x03, 0x00, 0x00, 0x00, 0xA8, 0x14, 0x92, 0xDA,
	0x00, 0x00, 0x00, 0x09, 0x50, 0x4C, 0x54, 0x45, 0x00, 0x00, 0x00,
	0xFF, 0xFF, 0xFF, 0xF8, 0xFF, 0xFF, 0x76, 0x37, 0xB3, 0xE6, 0x00,
	0x00, 0x00, 0x01, 0x74, 0x52, 0x4E, 0x53, 0x00, 0x40, 0xE6, 0xD8,
	0x66, 0x00, 0x00, 0x01, 0xA0, 0x49, 0x44, 0x41, 0x54, 0x48, 0xC7,
	0xE5, 0x96, 0x51, 0x6E, 0xC2, 0x30, 0x0C, 0x86, 0x9D, 0x87, 0xDC,
	0x60, 0xBE, 0xC4, 0x4E, 0x91, 0x97, 0xBD, 0x07, 0xC9, 0xBE, 0xDF,
	0x8E, 0xBA, 0xBF, 0xB6, 0x03, 0x81, 0x26, 0x69, 0x85, 0x10, 0xDA,
	0xB4, 0x5F, 0x18, 0x53, 0xF8, 0x6A, 0x3B, 0xAE, 0x12, 0x43, 0xAF,
	0x51, 0xA5, 0xA6, 0x5C, 0x08, 0x62, 0xF5, 0x2B, 0x99, 0x02, 0xD4,
	0x01, 0x49, 0x95, 0xD4, 0xAD, 0x34, 0x03, 0x70, 0xFD, 0xCE, 0xF8,
	0x0A, 0x53, 0xE2, 0x22, 0x94, 0xCD, 0xD8, 0xEE, 0x66, 0x58, 0x0E,
	0xC0, 0x2F, 0xC6, 0x00, 0xC2, 0x04, 0x40, 0x34, 0x04, 0xF0, 0x63,
	0x0F, 0x78, 0x1D, 0x32, 0x04, 0x22, 0x05, 0xC0, 0x41, 0x04, 0x25,
	0x9D, 0x01, 0xBA, 0x5B, 0xA6, 0x50, 0x2C, 0xD3, 0x81, 0xB9, 0xD8,
	0xDD, 0xF3, 0x00, 0x52, 0xBC, 0x48, 0xEC, 0x0F, 0xF1, 0x18, 0x10,
	0xB8, 0x65, 0x56, 0x99, 0xD5, 0xAD, 0x16, 0x06, 0x11, 0x52, 0x6D,
	0x0D, 0xAA, 0xD6, 0x2C, 0xF3, 0xDA, 0x52, 0xDC, 0x80, 0xCA, 0xA9,
	0x8A, 0x35, 0x7A, 0xF3, 0x7C, 0x1E, 0xB0, 0x67, 0xB1, 0x8E, 0x40,
	0xF9, 0xED, 0x40, 0x2C, 0xD3, 0x24, 0x00, 0x26, 0xCD, 0x3B, 0x09,
	0xA8, 0xCE, 0x80, 0xF7, 0x2B, 0xD5, 0x27, 0x80, 0xD8, 0x34, 0x95,
	0x18, 0x96, 0x55, 0x62, 0x0B, 0x32, 0x09, 0x1C, 0xC3, 0xFB, 0xD6,
	0x8B, 0x47, 0x0D, 0x77, 0x07, 0xE0, 0xB5, 0x06, 0xD2, 0x05, 0x5F,
	0x7C, 0xD3, 0xA7, 0x03, 0xC8, 0xF3, 0x08, 0x90, 0x96, 0x88, 0xE0,
	0x25, 0xAE, 0x01, 0x31, 0x20, 0x97, 0xBB, 0x14, 0x5A, 0x67, 0x80,
	0x7A, 0x91, 0x17, 0xA1, 0x38, 0xA3, 0xCC, 0xD7, 0x74, 0x3B, 0x0A,
	0x09, 0x26, 0x5C, 0xF4, 0xE4, 0x39, 0xF0, 0x6B, 0xC4, 0x44, 0xB1,
	0xC8, 0x67, 0x80, 0xF5, 0xB4, 0x91, 0xFD, 0xBC, 0x50, 0xB5, 0x33,
	0x63, 0x6B, 0x9C, 0x01, 0xFD, 0x59, 0xDD, 0xE6, 0x46, 0x8C, 0x8A,
	0x00, 0x92, 0x18, 0x10, 0xB6, 0x07, 0xE0, 0x87, 0x80, 0xB6, 0x14,
	0x78, 0x1B, 0x02, 0x12, 0x45, 0xE2, 0x33, 0xAF, 0x01, 0xA1, 0x05,
	0xA0, 0xEA, 0xCB, 0xEC, 0x97, 0xB8, 0x59, 0xAB, 0xE1, 0x70, 0x0F,
	0x1F, 0x03, 0x09, 0x29, 0xFE, 0xBE, 0x7C, 0x80, 0x32, 0xB1, 0xA2,
	0x31, 0xB0, 0xBE, 0x69, 0x81, 0x04, 0xD0, 0x3A, 0xD2, 0xFD, 0x97,
	0xA0, 0x8F, 0x33, 0x40, 0x8E, 0xD9, 0xCD, 0xD7, 0xB0, 0xBC, 0x5D,
	0x1B, 0x90, 0x3C, 0x0B, 0x87, 0xC5, 0x5D, 0x31, 0xD4, 0x01, 0x4D,
	0x00, 0xE4, 0x31, 0x1F, 0x29, 0x8E, 0x01, 0x9F, 0x09, 0xDC, 0x6A,
	0x30, 0xC0, 0xFC, 0x66, 0xFF, 0x49, 0x42, 0x94, 0xBE, 0xA8, 0x13,
	0xA3, 0x07, 0x15, 0x4E, 0x66, 0x80, 0xC6, 0x96, 0x9A, 0x01, 0x49,
	0x69, 0x07, 0xDC, 0x29, 0xCB, 0x01, 0xC0, 0xF5, 0x0A, 0x64, 0xAD,
	0x03, 0x40, 0xCB, 0x1A, 0x40, 0x09, 0xEB, 0x14, 0x59, 0x1E, 0x81,
	0x5D, 0x09, 0xAB, 0x08, 0xD5, 0xDA, 0x44, 0xDE, 0xA8, 0x09, 0xF0,
	0x7A, 0xFD, 0x00, 0x2B, 0x51, 0xD2, 0x22, 0x1C, 0xAC, 0x23, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x49, 0x45, 0x4E, 0x44, 0xAE, 0x42, 0x60,
	0x82,
};

typedef struct
{
	float	x;
	float   y;
	float   z;
	float   s;
	float   t;
} fontsprite_data;

const char font_vertex[] =
"#version 430\n"
"layout(location = 0) in vec4 v_coord;\n"
"layout(location = 1)in vec2 v_texcoord;\n"
"out gl_PerVertex"
"{"
"	vec4 gl_Position;"
"};"
"out vec2 ftexcoord;\n"
"layout(location = 2)uniform mat4 mvp;\n"
"void main() {\n"
"   ftexcoord = v_texcoord;\n"
"   gl_Position = mvp * v_coord;\n"
"}\n";

const char  font_pixel[] =
"#version 430\n"
"in vec2 ftexcoord;\n"
"layout(location = 0) out vec4 FragColor;\n"
"layout(location = 1) uniform sampler2D mytexture;\n"
"layout(location = 2) uniform vec4  color;\n"
"layout(location = 3) uniform float  flip_y;\n"
"void main()"
"{"
"vec2 coords=ftexcoord;\n"
"if(flip_y == 1.0)"
"coords.y= -coords.y;\n"
"FragColor=texture2D(mytexture,coords)*color;"
"}";

sprite_t textwriter;
sprite_t textwriter_bw;


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

unsigned char *LoadImageMemory(unsigned char* data, int size, int * width, int * height){
	IStream* pStream;
	HRESULT hr;
	using namespace Gdiplus;
	HGLOBAL hMem = ::GlobalAlloc(GMEM_MOVEABLE, size);
	LPVOID pImage = ::GlobalLock(hMem);
	CopyMemory(pImage, data, size);
	if (::CreateStreamOnHGlobal(hMem, FALSE, &pStream) != S_OK)
	return 0;
	else
	{
		Bitmap *pBitmap = Bitmap::FromStream(pStream,false);   //FAILS on WIN32
		*width = pBitmap->GetWidth();
		*height = pBitmap->GetHeight();
		int pitch = ((*width * 32 + 31) & ~31) >> 3;
		BitmapData data2;
		Gdiplus::Rect rect(0, 0, *width, *height);
		unsigned char* pixels = (GLubyte *)malloc(pitch * *height);
		memset(pixels, 0, pitch* *height);
		if (pBitmap->LockBits(&rect, ImageLockModeRead, PixelFormat32bppARGB, &data2) != Gdiplus::Ok)
			return 0;
		//ARGB to RGBA
		uint8_t *p = static_cast<uint8_t *>(data2.Scan0);
		for (int y = 0; y < *height; y++)
			for (int x = 0; x < *width; x++)
			{
				uint8_t tmp = p[2];
				p[2] = p[0];
				p[0] = tmp;
				p += 4;
			}
		if (data2.Stride == pitch)
		{
			memcpy(pixels, data2.Scan0, pitch * *height);
		}
		else
		{
			for (int i = 0; i < *height; ++i)
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



void init_sprite(sprite_t* spr, uint8_t* data, int data_len)
{
	int width, height;
	if(data_len)
	spr->texture = loadTexMemory(data, data_len, &width, &height, false);
	glGenVertexArrays(1, &spr->vao);
	glBindVertexArray(spr->vao);
	glGenBuffers(1, &spr->vbo);
	spr->program = initShader(font_vertex, (const char*)font_pixel);
}

void draw_sprite(sprite_t* fon, int xres, int yres, int x, int y)
{
	glBindProgramPipeline(fon->program.pid);
	gbMat4 projection, m_transform, mvp;
	float  tX = 0;
	if (!fon->font) fon->xsize / 2.0f;
	float  tY = 0;
	if (!fon->font) fon->ysize / 2.0f;
	gb_mat4_ortho2d(&projection, 0.0, (float)xres, (float)yres, 0.0);
	gb_mat4_translate(&m_transform, gb_vec3((float)x - tX, (float)x - tY, 0.0));
	gb_mat4_mul(&mvp, &projection, &m_transform);
	glProgramUniformMatrix4fv(fon->program.vsid, 2, 1, GL_FALSE, (float*)gb_float44_m(&mvp));
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, fon->texture);
	glProgramUniform1i(fon->program.fsid, 1, 0);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	float fontcol[4] = { fon->rcol / 255.0, fon->gcol / 255.0, fon->bcol / 255.0, fon->acol / 255.0 };
	glProgramUniform4fv(fon->program.fsid, 2, 1, fontcol);

	glBindVertexArray(fon->vao);
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glBindBuffer(GL_ARRAY_BUFFER, fon->vbo);

	if (!fon->font)
	{
		fontsprite_data sprite_vertices[] = {
			//X			//Y			//Z	//U  //V
			0, 0, 0, 0.0, 0.0,
			fon->xsize, 0, 0, 1.0, 0.0,
			0, fon->ysize, 0, 0.0, 1.0,
			fon->xsize, fon->ysize, 0, 1.0, 1.0,
		};
		glBufferData(GL_ARRAY_BUFFER, sizeof(fontsprite_data) * 4, sprite_vertices, GL_STATIC_DRAW);
	}
	else
	{
		float m_nCurrentRow = fon->numrows - 1;
		float m_nCurrentColumn = 0;
		for (int frame = 0; frame != fon->letter; frame++)
		{
			++m_nCurrentColumn;
			if (m_nCurrentColumn >= fon->numcolumns)
			{
				m_nCurrentColumn = 0;
				--m_nCurrentRow;
			}

		}
		float fFrame_s = ((1.0f / fon->xsize) * (fon->font_size * fon->numcolumns)) / fon->numcolumns;
		float fFrame_t = ((1.0f / fon->ysize) * (fon->font_size * fon->numrows)) / fon->numrows;
		float fLowerLeft_s = m_nCurrentColumn * fFrame_s;
		float fLowerRight_t = 1.0f - (m_nCurrentRow * fFrame_t) - fFrame_t;
		float fUpperRight_s = (m_nCurrentColumn * fFrame_s) + fFrame_s;
		float fUpperLeft_t = 1.0f - (m_nCurrentRow * fFrame_t);
		fontsprite_data sprite_vertices[] = {
			-fon->font_size, -fon->font_size, 0, fLowerLeft_s, fLowerRight_t,
			fon->font_size, -fon->font_size, 0, fUpperRight_s, fLowerRight_t,
			fon->font_size, fon->font_size, 0, fUpperRight_s, fUpperLeft_t,
			-fon->font_size, fon->font_size, 0, fLowerLeft_s, fUpperLeft_t,
		}; // 6 vertices with 5 components (floats) each
		glBufferData(GL_ARRAY_BUFFER, sizeof(fontsprite_data) * 4, sprite_vertices, GL_STATIC_DRAW);
	}
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(fontsprite_data), (void*)offsetof(fontsprite_data, x));
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(fontsprite_data), (void*)offsetof(fontsprite_data, s));
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);
	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glDisable(GL_BLEND);
	glBindProgramPipeline(0);
}

int gettablepos(char c)
{
	int pos;
	if (c >= 'A' && c <= 'Z')
		pos = c - 'A';
	//space
	if (c == 0x20)pos = 49;

	if (c >= '0' && c <= '9')
	{
		pos = c - '0';
		pos += 29;
	}
	//space
	if (c == ' ')pos = 42;
	//.
	if (c == '.')pos = 43;
	//~ - funkenstort!
	if (c == '!') pos == 24;
	//if (c == '!') pos == 47;
	return pos;
}

void init_fontwriter()
{
	textwriter.ysize = 96;
	textwriter.xsize = 128;
	textwriter.font_size = 16;
	textwriter.numletters = 64;
	textwriter.numcolumns = 8;
	textwriter.numrows = 6;
	textwriter.rcol = 1. / 1.0 * 255;
	textwriter.gcol = 1. / 1.0 * 255;
	textwriter.bcol = 1. / 1.0 * 255;
	textwriter.acol = 255;
	init_sprite(&textwriter, font, font_len);
	textwriter_bw.ysize = 96;
	textwriter_bw.xsize = 128;
	textwriter_bw.font_size = 16;
	textwriter_bw.numletters = 64;
	textwriter_bw.numcolumns = 8;
	textwriter_bw.numrows = 6;
	textwriter_bw.rcol = 0. / 1.0 * 255;
	textwriter_bw.gcol = 0. / 1.0 * 255;
	textwriter_bw.bcol = 0. / 1.0 * 255;
	textwriter_bw.acol = 255;
	init_sprite(&textwriter_bw, font, font_len);
}

void draw_font(int positionx, int positiony,int xres,int yres, char* buffer)
{
	int l = strlen((char*)buffer);

	int positionx2 = positionx + 7;
	int positiony2 = positiony + 7;
	int letpos = positionx;
	int letpos2 = positionx2;
	for (int a = 0; a < l; a++)
	{
		unsigned char c = buffer[a];
		int pos = gettablepos(c);
		textwriter.letter = gettablepos(c);
		textwriter_bw.letter = gettablepos(c);

		draw_sprite(&textwriter_bw,xres,yres, letpos2, positiony2);
		draw_sprite(&textwriter,xres,yres, letpos, positiony);

		switch (c)
		{
		case 'R':
			letpos += 25;
			letpos2 += 25;
			break;
		case 'F':
			letpos += 20;
			letpos2 += 20;
			break;
		case 'I':
			letpos += 20;
			letpos2 += 20;
			break;
		case 'N':
			if (buffer[a + 1] == 'T')
			{
				letpos += 20;
				letpos2 += 20;
			}
			else
			{
				letpos += 32;
				letpos2 += 32;
			}

			break;
		case 'L':
			if (buffer[a + 1] == 'I' || buffer[a + 1] == 'L' || buffer[a + 1] == 'D')
			{
				letpos += 10;
				letpos2 += 10;
			}
			else
			{
				letpos += 20;
				letpos2 += 20;
			}
			break;
		default:
			if (buffer[a + 1] == 'I' || buffer[a + 1] == 'L')
			{
				letpos += 20;
				letpos2 += 20;
				break;
			}
			else
			{
				letpos += 32;
				letpos2 += 32;
			}
			break;
		}

	}
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

FBOELEM init_fbo(int width, int height, BOOL fp)
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
