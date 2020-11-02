#include "sys/msys.h"
#include "intro.h"

#include <stdint.h>

typedef struct  
{
	float x,y,z;
	int xsize,ysize;
	GLuint texture;
	shader_id program;
	float xtexoffset,ytexoffset;
	GLuint sprite_vbo, sprite_texcoords, vao;
	GLint uniform_mvp, uniform_mytexture,uniform_xoffset,uniform_yoffset;
	GLint uniform_color;
	float rcol,gcol,bcol,acol;
}sprite;

void init_sprite(sprite* spr)
{
	const char vertex_source [] =
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

	const char  fragment_source [] =
		"#version 430\n"
		"in vec2 ftexcoord;\n"
		"layout(location = 0) out vec4 FragColor;\n"
		"layout(location = 1) uniform sampler2D mytexture;\n"
		"layout(location = 2) uniform float xoffset;\n"
		"layout(location = 3) uniform float yoffset;\n"
		"layout(location = 4) uniform vec4  color;\n"
		"void main()"
		"{"
		"vec2 coords=ftexcoord;\n"
		"coords.x+=xoffset;\n"
		"coords.y+=yoffset;\n"
		"coords.y= -coords.y;\n"
		"FragColor=texture2D(mytexture,coords)*color;"
		"}";


	typedef struct
	{
		float   x; //vertices
		float   y;
		float   z;
		float   u; //tex coords
		float   v;
	} sprite_vbodat;


	sprite_vbodat sprite_vertices[] = {
		//X			//Y			//Z	//U  //V
		0,			0,			0,	0.0, 0.0,
		spr->xsize, 0,			0,	1.0, 0.0,
		0,			spr->ysize, 0,  0.0, 1.0,
		spr->xsize, spr->ysize, 0,  1.0, 1.0,
	};

	glGenVertexArrays(1, &spr->vao);
	glBindVertexArray(spr->vao);

	glGenBuffers(1, &spr->sprite_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, spr->sprite_vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(sprite_vbodat) * 4, sprite_vertices, GL_STATIC_DRAW);
	glBindVertexArray(0);
	spr->program = initShader(vertex_source, (const char*)fragment_source);
}

void draw_sprite(sprite * spr, int xres, int yres)
{
	glBindProgramPipeline(spr->program.pid);
	gbMat4 projection;
	gbMat4 m_transform;
	gbMat4 mvp;
	float  tX = spr->xsize / 2.0f;
	float  tY = spr->ysize / 2.0f;
	gb_mat4_ortho2d(&projection, 0.0, (float)xres, (float)yres, 0.0);
	gb_mat4_translate(&m_transform, gb_vec3((float)spr->x - tX, (float)spr->y - tY, 0.0));
	gb_mat4_mul(&mvp, &projection, &m_transform);
	glProgramUniformMatrix4fv(spr->program.vsid, 2, 1, GL_FALSE, (float*)gb_float44_m(&mvp));
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, spr->texture);
	glProgramUniform1i(spr->program.fsid, 1, 0);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);

	glProgramUniform1i(spr->program.fsid, 2, spr->xtexoffset);
	glProgramUniform1i(spr->program.fsid, 3, spr->ytexoffset);
	float fontcol[4] = { spr->rcol / 255.0, spr->gcol / 255.0, spr->bcol / 255.0, spr->acol / 255.0 };
	glProgramUniform4fv(spr->program.fsid, 4, 1, fontcol);

	glBindVertexArray(spr->vao);

	typedef struct
	{
		float   x; //vertices
		float   y;
		float   z;
		float   u; //tex coords
		float   v;
	} sprite_vbodat;


	sprite_vbodat sprite_vertices[] = {
		//X			//Y			//Z	//U  //V
		0, 0, 0, 0.0, 0.0,
		spr->xsize, 0, 0, 1.0, 0.0,
		0, spr->ysize, 0, 0.0, 1.0,
		spr->xsize, spr->ysize, 0, 1.0, 1.0,
	};

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glBindBuffer(GL_ARRAY_BUFFER,spr->sprite_vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(sprite_vbodat) * 4, sprite_vertices, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(sprite_vbodat), (void*)offsetof(sprite_vbodat, x));
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(sprite_vbodat), (void*)offsetof(sprite_vbodat, u));
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);
	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER,0);
	glDisable(GL_BLEND);
	glBindProgramPipeline(0);
}