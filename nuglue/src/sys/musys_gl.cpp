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

size_t font_len = 216;
unsigned char font[216] =
{
	0x89,0x50,0x4E,0x47,0x0D,0x0A,0x1A,0x0A,0x00,0x00,0x00,
	0x0D,0x49,0x48,0x44,0x52,0x00,0x00,0x00,0x2A,0x00,0x00,
	0x00,0x0C,0x04,0x03,0x00,0x00,0x00,0xE6,0x18,0x32,0x43,
	0x00,0x00,0x00,0x01,0x73,0x52,0x47,0x42,0x00,0xAE,0xCE,
	0x1C,0xE9,0x00,0x00,0x00,0x04,0x67,0x41,0x4D,0x41,0x00,
	0x00,0xB1,0x8F,0x0B,0xFC,0x61,0x05,0x00,0x00,0x00,0x30,
	0x50,0x4C,0x54,0x45,0x00,0x00,0x63,0xFC,0xFC,0xFC,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x82,0x57,0xCA,
	0x24,0x00,0x00,0x00,0x09,0x70,0x48,0x59,0x73,0x00,0x00,
	0x0E,0xC3,0x00,0x00,0x0E,0xC3,0x01,0xC7,0x6F,0xA8,0x64,
	0x00,0x00,0x00,0x31,0x49,0x44,0x41,0x54,0x28,0xCF,0x63,
	0x60,0x14,0x04,0x01,0x01,0x41,0x08,0x05,0xE5,0x31,0x08,
	0x62,0x03,0x0C,0x82,0x0C,0x20,0x20,0x88,0x4A,0x91,0x20,
	0xCA,0x80,0x6A,0xAE,0x00,0x01,0x51,0xEC,0x26,0x90,0xE1,
	0x06,0xEC,0xBE,0x40,0xF2,0xB1,0xA0,0x00,0x00,0xE2,0x37,
	0x08,0xF4,0xDA,0x5B,0x1D,0x65,0x00,0x00,0x00,0x00,0x49,
	0x45,0x4E,0x44,0xAE,0x42,0x60,0x82,
};



const unsigned font_getposition(char c)
{
	int pos;

	if (c >= 'A' && c <= 'Z')
		pos = c - 'A';

	if (c >= '0' && c <= '9')
	{
		pos = c - '0';
		pos += 26;
	}

	//space
	if (c == '.')pos = 36;
	if (c == '(')pos = 37;
	if (c == ')')pos = 38;
	if (c == '{')pos = 39;
	if (c == '}')pos = 40;
	if (c == ',')pos = 41;
	if (c == '"') pos == 42;
	if (c == ';') pos == 43;
	if (c == '^') pos == 44;
	if (c == '<') pos == 45;
	if (c == '>')pos == 46;
	if (c == '[')pos == 47;
	if (c == ']')pos == 48;
	if (c == ' ')pos == 49;

	return pos;
}

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

const char  sprite_pixel[] =
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
	GdiplusStartupInput gdiplus_startinput;
	ULONG_PTR           gdiplus_tok;
	// Initialize GDI+.
	GdiplusStartup(&gdiplus_tok, &gdiplus_startinput, NULL);
	for (int i = 0; i <NUMFUNCS; i++)
	{
		msys_oglfunc[i] = wglGetProcAddress(funcs[i]);
		if (!msys_oglfunc[i])
		return(0);	
	}
	return 1;
}
inline void* __cdecl  operator new(size_t size) { return(malloc((uint32_t)size)); }
inline void __cdecl  operator delete[](void* ptr,size_t size) { free(ptr); }
inline void __cdecl  operator delete(void* ptr, size_t size) { free(ptr); }

unsigned char *LoadImageMemory(unsigned char* data, int size, int * width, int * height){
	IStream* p_istream;
	HRESULT hr;
	using namespace Gdiplus;
	HGLOBAL h_mem = ::GlobalAlloc(GMEM_MOVEABLE, size);
	LPVOID p_image = ::GlobalLock(h_mem);
	CopyMemory(p_image, data, size);
	if (::CreateStreamOnHGlobal(h_mem, FALSE, &p_istream) != S_OK)
	return 0;
	else
	{
		Bitmap *p_bitmap = Bitmap::FromStream(p_istream,false);   //FAILS on WIN32
		*width = p_bitmap->GetWidth();
		*height = p_bitmap->GetHeight();
		int pitch = ((*width * 32 + 31) & ~31) >> 3;
		BitmapData data;
		Gdiplus::Rect rect(0, 0, *width, *height);
		unsigned char* pixels = (GLubyte *)malloc(pitch * *height);
		memset(pixels, 0, pitch* *height);
		if (p_bitmap->LockBits(&rect, ImageLockModeRead, PixelFormat32bppARGB, &data) != Gdiplus::Ok)
			return 0;
		//ARGB to RGBA
		uint8_t *p = static_cast<uint8_t *>(data.Scan0);
		for (int y = 0; y < *height; y++)
			for (int x = 0; x < *width; x++)
			{
				uint8_t tmp = p[2];
				p[2] = p[0];
				p[0] = tmp;
				p += 4;
			}
		if (data.Stride == pitch)
		{
			memcpy(pixels, data.Scan0, pitch * *height);
		}
		else
		{
			for (int i = 0; i < *height; ++i)
				memcpy(&pixels[i * pitch], &p[i * data.Stride], pitch);
		}
		p_bitmap->UnlockBits(&data);
		//image is now in RGBA
		delete[] p_bitmap;
		GlobalUnlock(h_mem);
		GlobalFree(h_mem);
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
	if (spr->font)
	{
		const char  fragment_source[] =
			"#version 430\n"
			"in vec2 ftexcoord;\n"
			"layout(location = 0) out vec4 FragColor;\n"
			"layout(location = 1) uniform sampler2D mytexture;\n"
			"layout(location = 2) uniform vec4  color;\n"
			"void main()"
			"{"
			"vec2 coords=ftexcoord;\n"
			"FragColor=texture2D(mytexture,coords)*color;"
			"}";
		spr->program = initShader(font_vertex, (const char*)fragment_source);
	}
	else
	spr->program = initShader(font_vertex, (const char*)sprite_pixel);
}

void draw_sprite(sprite_t* fon, int xres, int yres, int x, int y)
{
	glBindProgramPipeline(fon->program.pid);
	gbMat4  mvp, projection, m_transform;
	gb_mat4_ortho2d(&projection, 0.0, (float)xres, (float)yres, 0.0);
	if (!fon->font)
	{
		float  tX = fon->xsize / 2.0f;
		float  tY = fon->ysize / 2.0f;
		
		gb_mat4_translate(&m_transform, gb_vec3((float)x - tX, (float)y - tY, 0.0));
		gb_mat4_mul(&mvp, &projection, &m_transform);
	}
	else
	{
		gbMat4 model;
		gb_mat4_translate(&m_transform, gb_vec3((float)x, (float)y, 0.0));
		gb_mat4_scale(&model, gb_vec3(2.0, 2.0, 2.0));
		gb_mat4_mul(&mvp, &m_transform, &model);
		gb_mat4_mul(&mvp, &projection, &mvp);
	}
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
			//X //Y			//Z	//U  //V
			0, 0, 0, 0.0, 0.0,
			fon->xsize, 0, 0, 1.0, 0.0,
			0, fon->ysize, 0, 0.0, 1.0,
			fon->xsize, fon->ysize, 0, 1.0, 1.0,
		};
		glBufferData(GL_ARRAY_BUFFER, sizeof(fontsprite_data) * 4, sprite_vertices, GL_DYNAMIC_DRAW);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(fontsprite_data), (void*)offsetof(fontsprite_data, x));
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(fontsprite_data), (void*)offsetof(fontsprite_data, s));
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	}
	else
	{
		float subrange_s = (1.0f / fon->xsize) *
			(fon->font_sizex * fon->numframes);
		float subrange_t = (1.0f / fon->ysize) *
			(fon->font_sizey * 1);
		float frame_s = subrange_s / fon->numframes;
		float frame_t = subrange_t / 1;
		float lleft_s = fon->letter * frame_s;
		float lleft_t = 1.0f - ((fon->numframes - 1) * frame_t) - frame_t;
		float lr_s = (fon->letter * frame_s) + frame_s;
		float ur_s = (fon->letter * frame_s) + frame_s;
		float ur_t = 1.0f - ((fon->numframes - 1) * frame_t);
		float ul_s = fon->letter * frame_s;
		float rel_x = (fon->font_sizex / 2.0f);
		float rel_y = (fon->font_sizey / 2.0f);

		typedef struct
		{
			float	x;
			float   y;
			float   s;
			float   t;
		} font_data;

		font_data font_coords[] = {
			-rel_x, -rel_y, lleft_s, lleft_t,
			rel_x, -rel_y, lr_s, lleft_t,
			rel_x, rel_y, ur_s, ur_t,
			-rel_x, rel_y, ul_s, ur_t,
		}; // 6 vertices with 5 components (floats) each

		glBufferData(GL_ARRAY_BUFFER, sizeof(font_data) * 4, font_coords, GL_DYNAMIC_DRAW);
		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(font_data), (void*)offsetof(font_data, x));
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(font_data), (void*)offsetof(font_data, s));
		glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
	}
	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);
	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glDisable(GL_BLEND);
	glBindProgramPipeline(0);
}



void init_fontwriter()
{
	textwriter.xsize = 42;
	textwriter.ysize = 12;
	textwriter.font_sizex = 14;
	textwriter.font_sizey = 12;
	textwriter.numframes = 3;
	textwriter.rcol = 1. / 1.0 * 255;
	textwriter.gcol = 1. / 1.0 * 255;
	textwriter.bcol = 1. / 1.0 * 255;
	textwriter.acol = 255;
	textwriter.font = true;
	init_sprite(&textwriter, font, font_len);
}

void draw_font(int positionx, int positiony,int xres,int yres, char* buffer)
{
	int l = strlen((char*)buffer);

	int letpos = positionx;
	for (int a = 0; a < l; a++)
	{
		unsigned char c = buffer[a];
		int pos = font_getposition(c);
		textwriter.letter = font_getposition(c);
		draw_sprite(&textwriter,xres,yres, letpos, positiony);

		switch (c)
		{
		case 'R':
			letpos += 25;
			break;
		case 'F':
			letpos += 20;
			break;
		case 'I':
			letpos += 20;
			break;
		case 'N':
			if (buffer[a + 1] == 'T')
			{
				letpos += 20;
			}
			else
			{
				letpos += 32;
			}

			break;
		case 'L':
			if (buffer[a + 1] == 'I' || buffer[a + 1] == 'L' || buffer[a + 1] == 'D')
			{
				letpos += 10;
			}
			else
			{
				letpos += 20;
			}
			break;
		default:
			if (buffer[a + 1] == 'I' || buffer[a + 1] == 'L')
			{
				letpos += 20;
				break;
			}
			else
			{
				letpos += 42;
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
