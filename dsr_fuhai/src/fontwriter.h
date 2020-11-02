#include "sys/msys.h"
#include "intro.h"
#include "font.h"

typedef struct
{
	int texwidth, texheight;
	int font_size;
	int letter;
	int numrows;
	int numcolumns;
	int numletters;
	GLuint texture;
	shader_id program;
	GLuint font_vbo, vao;
	float rcol, gcol, bcol, acol;
}font_t;


void init_font(font_t * fon)
{
	float	cx;											// Holds Our X Character Coord
	float	cy;											// Holds Our Y Character Coord
	int width, height, comp;
	unsigned char *data = LoadImageMemory(font, font_len, &width, &height);
	GLuint fonttexture;
	glGenTextures(1, &fon->texture);
	glBindTexture(GL_TEXTURE_2D, fon->texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, fon->texwidth, fon->texheight, 0,
		GL_RGBA, GL_UNSIGNED_BYTE, data);
	glBindTexture(GL_TEXTURE_2D, 0);


	const char vertex_source[] =
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

	glGenVertexArrays(1, &fon->vao);
	glBindVertexArray(fon->vao);
	glGenBuffers(1, &fon->font_vbo);
	glBindVertexArray(0);

	fon->program = initShader(vertex_source, (const char*)fragment_source);
}


void draw_font(font_t* fon, int x, int y)
{
	glBindProgramPipeline(fon->program.pid);
	gbMat4 projection;
	gbMat4 m_transform;
	gbMat4 model;
	gbMat4 mvp;
	gb_mat4_ortho2d(&projection, 0.0, (float)asset_xr, (float)asset_yr, 0.0);
	gb_mat4_translate(&m_transform, gb_vec3((float)x, (float)y, 0.0));
	gb_mat4_scale(&model, gb_vec3(2.0, 2.0, 2.0));
	gb_mat4_mul(&mvp, &m_transform, &model);
	gb_mat4_mul(&mvp, &projection, &mvp);
	glProgramUniformMatrix4fv(fon->program.vsid, 2, 1, GL_FALSE, (float*)gb_float44_m(&mvp));
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, fon->texture);
	glProgramUniform1i(fon->program.fsid, 1,0);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	float fontcol[4] = { fon->rcol / 255.0, fon->gcol / 255.0, fon->bcol / 255.0, fon->acol / 255.0 };
	glProgramUniform4fv(fon->program.fsid,2,1,fontcol);
	
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


	float m_nFrameWidth = fon->font_size;
	float m_nFrameHeight = fon->font_size;

	float m_nTextureWidth = fon->texwidth;
	float m_nTextureHeight = fon->texheight;
	float fSubRange_s = (1.0f / m_nTextureWidth) *
		(m_nFrameWidth  * fon->numcolumns);

	float fSubRange_t = (1.0f / m_nTextureHeight) *
		(m_nFrameHeight * fon->numrows);



	float fFrame_s = fSubRange_s / fon->numcolumns;
	float fFrame_t = fSubRange_t / fon->numrows;
	float fLowerLeft_s = m_nCurrentColumn * fFrame_s;
	float fLowerLeft_t = 1.0f - (m_nCurrentRow * fFrame_t) - fFrame_t;

	float fLowerRight_s = (m_nCurrentColumn * fFrame_s) + fFrame_s;
	float fLowerRight_t = 1.0f - (m_nCurrentRow * fFrame_t) - fFrame_t;

	float fUpperRight_s = (m_nCurrentColumn * fFrame_s) + fFrame_s;
	float fUpperRight_t = 1.0f - (m_nCurrentRow * fFrame_t);

	float fUpperLeft_s = m_nCurrentColumn * fFrame_s;
	float fUpperLeft_t = 1.0f - (m_nCurrentRow * fFrame_t);

	float fRelativeX = (fon->font_size / 2.0f);
	float fRelativeY = (fon->font_size / 2.0f);

	typedef struct
	{
		float	x;
		float   y;
		float   s;
		float   t;
	} font_data;
	font_data font_coords[] = {
		-fRelativeX, -fRelativeY, fLowerLeft_s, fLowerLeft_t,
		fRelativeX, -fRelativeY, fLowerRight_s, fLowerRight_t,
		fRelativeX, fRelativeY, fUpperRight_s, fUpperRight_t,
		-fRelativeX, fRelativeY, fUpperLeft_s, fUpperLeft_t,
	}; // 6 vertices with 5 components (floats) each


	glBindVertexArray(fon->vao);

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);

	glBindBuffer(GL_ARRAY_BUFFER, fon->font_vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(font_data)*4, font_coords, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(font_data), (void*)offsetof(font_data, x));
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(font_data), (void*)offsetof(font_data, s));

	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);
	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER,0);
	glDisable(GL_BLEND);
	glBindProgramPipeline(0);
}



font_t textwriter;
font_t textwriter_bw;



void init_fonts()
{
	textwriter.texheight = 96;
	textwriter.texwidth = 128;
	textwriter.font_size = 16;
	textwriter.numletters = 64;
	textwriter.numcolumns = 8;
	textwriter.numrows = 6;
	textwriter.rcol = 1. / 1.0 * 255;
	textwriter.gcol = 1. / 1.0 * 255;
	textwriter.bcol = 1. / 1.0 * 255;
	textwriter.acol = 255;
	init_font(&textwriter);

	textwriter_bw.texheight = 96;
	textwriter_bw.texwidth = 128;
	textwriter_bw.font_size = 16;
	textwriter_bw.numletters = 64;
	textwriter_bw.numcolumns = 8;
	textwriter_bw.numrows = 6;
	textwriter_bw.rcol = 0. / 1.0 * 255;
	textwriter_bw.gcol = 0. / 1.0 * 255;
	textwriter_bw.bcol = 0. / 1.0 * 255;
	textwriter_bw.acol = 255;
	init_font(&textwriter_bw);

}



//Customize according to each prod/typeface as needed
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

void draw_font(int positionx, int positiony, char* buffer)
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

		draw_font(&textwriter_bw, letpos2, positiony2);
		draw_font(&textwriter, letpos, positiony);

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