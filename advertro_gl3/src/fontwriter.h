#include "sys/msys.h"
#include "intro.h"
#include "font.h"


typedef struct
{
	int texwidth, texheight;
	int font_size;
	int letter;
	int numletters;
	GLuint texture;
	GLuint program;
	GLuint fonite_vertices, fonite_texcoords, vao;
	GLint attribute_v_coord, attribute_v_texcoord;
	GLint uniform_mvp, uniform_mytexture, uniform_xoffset, uniform_yoffset;
	GLint uniform_color;
	float rcol, gcol, bcol, acol;
}font_t;




void init_font(font_t * fon)
{

	float	cx;											// Holds Our X Character Coord
	float	cy;											// Holds Our Y Character Coord
	int width, height, comp;
	unsigned char *data = stbi_load_from_memory(font, font_len, &width, &height, &comp, 4);
	GLuint fonttexture;
	glGenTextures(1, &fon->texture);
	glBindTexture(GL_TEXTURE_2D, fon->texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, fon->texwidth, fon->texheight, 0,
		GL_RGBA, GL_UNSIGNED_BYTE, data);
	glBindTexture(GL_TEXTURE_2D, 0);


	const char vertex_source[] =
		"#version 330\n"
		"in vec4 v_coord;\n"
		"in vec2 v_texcoord;\n"
		"out vec2 ftexcoord;\n"
		"uniform mat4 mvp;\n"
		"void main() {\n"
		"   ftexcoord = v_texcoord;\n"
		"   gl_Position = mvp * v_coord;\n"
		"}\n";

	const char  fragment_source[] =
		"#version 330\n"
		"in vec2 ftexcoord;\n"
		"layout(location = 0) out vec4 FragColor;\n"
		"uniform sampler2D mytexture;\n"
		"uniform vec4  color;\n"
		"void main()"
		"{"
		"vec2 coords=ftexcoord;\n"
		"FragColor=texture2D(mytexture,coords)*color;"
		"}";



	glGenVertexArrays(1, &fon->vao);
	glBindVertexArray(fon->vao);

	glGenBuffers(1, &fon->fonite_vertices);


	GLfloat fonite_texcoords[] = {
		0.0, 0.0,
		1.0, 0.0,
		0.0, 1.0,
		1.0, 1.0,
	};
	glGenBuffers(1, &fon->fonite_texcoords);

	glBindVertexArray(0);

	initShader((int*)&fon->program, vertex_source, (const char*)fragment_source);


	const char* attribute_name;
	attribute_name = "v_coord";
	fon->attribute_v_coord = glGetAttribLocation(fon->program, attribute_name);
	attribute_name = "v_texcoord";
	fon->attribute_v_texcoord = glGetAttribLocation(fon->program, attribute_name);
	const char* uniform_name;
	uniform_name = "mvp";
	fon->uniform_mvp = glGetUniformLocation(fon->program, uniform_name);
	uniform_name = "mytexture";
	fon->uniform_mytexture = glGetUniformLocation(fon->program, uniform_name);
	uniform_name = "color";
	fon->uniform_color = glGetUniformLocation(fon->program, uniform_name);
}


void draw_font(font_t* fon, int x,int y)
{
		glUseProgram(fon->program);
		glm::mat4 projection = glm::ortho(0.0f, (float)XRES, (float)YRES, 0.0f);
		glm::mat4 m_transform = glm::translate(glm::mat4(1.0f), glm::vec3((float)x, (float)y, 0.0));
		glm::mat4 model = glm::scale(glm::mat4(1.0f), glm::vec3(2.5f));
		glm::mat4 mvp = projection * m_transform * model; // * view * model * anim;
		glUniformMatrix4fv(fon->uniform_mvp, 1, GL_FALSE, glm::value_ptr(mvp));
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D,fon->texture);
		glUniform1i(fon->uniform_mytexture, 0);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glUniform4f(fon->uniform_color, fon->rcol / 255.0, fon->gcol / 255.0, fon->bcol / 255.0, fon->acol / 255.0);
		glBindVertexArray(fon->vao);


		float fSubRange_s = (1.0f / fon->texwidth) *
			((float)fon->font_size  * fon->numletters);

		float fSubRange_t = (1.0f / fon->texheight) *
			(fon->font_size);

		float fFrame_s = fSubRange_s / fon->numletters;
		float fFrame_t = fSubRange_t / 1;
		float fLowerLeft_s = fon->letter * fFrame_s;
		float fLowerLeft_t = 1.0f - (1 * fFrame_t) - fFrame_t;

		float fLowerRight_s = (fon->letter * fFrame_s) + fFrame_s;
		float fLowerRight_t = 1.0f - (1 * fFrame_t) - fFrame_t;

		float fUpperRight_s = (fon->letter * fFrame_s) + fFrame_s;
		float fUpperRight_t = 1.0f - (1 * fFrame_t);

		float fUpperLeft_s = fon->letter * fFrame_s;
		float fUpperLeft_t = 1.0f - (1 * fFrame_t);

		float fRelativeX = (fon->font_size / 2.0f);
		float fRelativeY = (fon->font_size / 2.0f);

		GLfloat fonite_vertices[] = {
			-fRelativeX, -fRelativeY,
			fRelativeX, -fRelativeY,
			fRelativeX, fRelativeY,
			-fRelativeX, fRelativeY,
		};

		GLfloat fonite_texcoords[] = {
			fLowerLeft_s, fLowerLeft_t,
			fLowerRight_s, fLowerRight_t,
			fUpperRight_s, fUpperRight_t,
			fUpperLeft_s, fUpperLeft_t,
		};

		glEnableVertexAttribArray(fon->attribute_v_coord);
		glBindBuffer(GL_ARRAY_BUFFER, fon->fonite_vertices);
		glBufferData(GL_ARRAY_BUFFER, sizeof(fonite_vertices), fonite_vertices, GL_STATIC_DRAW);

		glVertexAttribPointer(
			fon->attribute_v_coord,
			2,
			GL_FLOAT,
			GL_FALSE,
			0,
			0
			);

		glEnableVertexAttribArray(fon->attribute_v_texcoord);
		glBindBuffer(GL_ARRAY_BUFFER, fon->fonite_texcoords);
		glBufferData(GL_ARRAY_BUFFER, sizeof(fonite_texcoords), fonite_texcoords, GL_STATIC_DRAW);
		glVertexAttribPointer(
			fon->attribute_v_texcoord,
			2,
			GL_FLOAT,
			GL_FALSE,
			0,
			0
			);
		glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
		glDisableVertexAttribArray(fon->attribute_v_coord);
		glDisableVertexAttribArray(fon->attribute_v_texcoord);
		glBindVertexArray(0);
		glDisable(GL_BLEND);
		glUseProgram(0);
}



font_t textwriter;

int gettablepos(char c)
{
	int pos;
	if (c >= 'A' && c <= 'Z')
		pos = c - 'A';
	//space
	if (c == 0x20)pos = 26;
	//*
	if (c == 0x2A)pos = 27;
	//+
	if (c == 0x2B)pos = 28;
	//,
	if (c == 0x2C)pos = 29;
	// - 
	if (c == 0x2D)pos = 30;
	//.
	if (c == 0x2E)pos = 31;
	// /
	if (c == 0x2F)pos = 32;

	if (c >= '0' && c <= '9')
	{
		pos = c - '0';
		pos += 33;
	}
	//space

	return pos;
}

void init_fonts()
{
	textwriter.texheight = 16;
	textwriter.texwidth = 720;
	textwriter.font_size = 16;
	textwriter.numletters = 45;
	textwriter.rcol = 0.8 / 1.0 * 255;
		textwriter.gcol = 0.8 / 1.0*255;
	textwriter.bcol = 0.9 / 1.0*255;
	textwriter.acol = 255;
	init_font(&textwriter);


}

void draw_fonts()
{
	char* buffer = "HELLO";
	int l = strlen((char*)buffer);
	int positionx = XRES / 2;
	int positiony = YRES / 2;
	int letpos = positionx;
	for (int a = 0; a < l; a++)
	{
		unsigned char c = buffer[a];
		int pos = gettablepos(c);
		textwriter.letter = gettablepos(c);
		draw_font(&textwriter,letpos, positiony);
		letpos += 35;

	}


	
}