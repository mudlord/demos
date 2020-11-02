#include "sys/msys.h"
#include "intro.h"

#include <stdint.h>
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp" 

typedef struct  
{
	float x,y,z;
	int xsize,ysize;
	GLuint texture;
	GLuint program;
	float xtexoffset,ytexoffset;
	GLuint sprite_vertices, sprite_texcoords, vao;
	GLint attribute_v_coord, attribute_v_texcoord;
	GLint uniform_mvp, uniform_mytexture,uniform_xoffset,uniform_yoffset;
	GLint uniform_color;
	float rcol,gcol,bcol,acol;
}sprite;

void init_sprite(sprite* spr)
{
	const char vertex_source [] =
		"#version 330\n"
		"in vec4 v_coord;\n"
		"in vec2 v_texcoord;\n"
		"out vec2 ftexcoord;\n"
		"uniform mat4 mvp;\n"
		"void main() {\n"
		"   ftexcoord = v_texcoord;\n"
		"   gl_Position = mvp * v_coord;\n"
		"}\n";

	const char  fragment_source [] =
		"#version 330\n"
		"in vec2 ftexcoord;\n"
		"layout(location = 0) out vec4 FragColor;\n"
		"uniform sampler2D mytexture;\n"
		"uniform float xoffset;\n"
		"uniform float yoffset;\n"
		"uniform vec4  color;\n"
		"void main()"
		"{"
		"vec2 coords=ftexcoord;\n"
		"coords.x+=xoffset;\n"
		"coords.y+=yoffset;\n"
		"coords.y= -coords.y;\n"
		"FragColor=texture2D(mytexture,coords)*color;"
		"}";

	GLfloat sprite_vertices[] = {
		0,    0, 0,
		spr->xsize,  0, 0,
		0,  spr->ysize, 0,
		spr->xsize,spr->ysize, 0,
	};

	glGenVertexArrays(1, &spr->vao);
	glBindVertexArray(spr->vao);

	glGenBuffers(1, &spr->sprite_vertices);
	glBindBuffer(GL_ARRAY_BUFFER, spr->sprite_vertices);
	glBufferData(GL_ARRAY_BUFFER, sizeof(sprite_vertices), sprite_vertices, GL_STATIC_DRAW);


	GLfloat sprite_texcoords[] = {
		0.0, 0.0,
		1.0, 0.0,
		0.0, 1.0,
		1.0, 1.0,
	};
	glGenBuffers(1, &spr->sprite_texcoords);
	glBindBuffer(GL_ARRAY_BUFFER, spr->sprite_texcoords);
	glBufferData(GL_ARRAY_BUFFER, sizeof(sprite_texcoords), sprite_texcoords, GL_STATIC_DRAW);

	glBindVertexArray(0);

	initShader(  (int*)&spr->program, vertex_source, (const char*)fragment_source);


	const char* attribute_name;
	attribute_name = "v_coord";
	spr->attribute_v_coord = glGetAttribLocation(spr->program, attribute_name);
	attribute_name = "v_texcoord";
	spr->attribute_v_texcoord = glGetAttribLocation(spr->program, attribute_name);
	const char* uniform_name;
	uniform_name = "mvp";
	spr->uniform_mvp = glGetUniformLocation(spr->program, uniform_name);
	uniform_name = "mytexture";
	spr->uniform_mytexture = glGetUniformLocation(spr->program, uniform_name);
	uniform_name = "xoffset";
	spr->uniform_xoffset = glGetUniformLocation(spr->program, uniform_name);
	uniform_name = "yoffset";
	spr->uniform_yoffset = glGetUniformLocation(spr->program, uniform_name);
	uniform_name = "color";
	spr->uniform_color = glGetUniformLocation(spr->program, uniform_name);
}

void draw_sprite(sprite * spr, int xres, int yres)
{
	glUseProgram(spr->program);

	glm::mat4 projection = glm::ortho(0.0f,(float)XRES, (float)YRES, 0.0f);
	glm::mat4 m_transform = glm::translate(glm::mat4(1.0f), glm::vec3((float)spr->x, (float)spr->y, 0.0));
	glm::mat4 mvp = projection * m_transform; // * view * model * anim;
	glUniformMatrix4fv(spr->uniform_mvp, 1, GL_FALSE, glm::value_ptr(mvp));

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, spr->texture);
	glUniform1i(spr->uniform_mytexture,0);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);

	glUniform1f(spr->uniform_xoffset,spr->xtexoffset);
	glUniform1f(spr->uniform_yoffset,spr->ytexoffset);
	glUniform4f(spr->uniform_color,spr->rcol/255.0,spr->gcol/255.0,spr->bcol/255.0,spr->acol/255.0);

	glBindVertexArray(spr->vao);

	GLfloat sprite_vertices[] = {
		0,    0, 0,
		spr->xsize,  0, 0,
		0, spr->ysize, 0,
		spr->xsize,spr->ysize, 0,
	};

	glEnableVertexAttribArray(spr->attribute_v_coord);
	glBindBuffer(GL_ARRAY_BUFFER,spr->sprite_vertices);
	glBufferData(GL_ARRAY_BUFFER, sizeof(sprite_vertices), sprite_vertices, GL_STATIC_DRAW);

	glVertexAttribPointer(
		spr->attribute_v_coord, 
		3,                
		GL_FLOAT,         
		GL_FALSE,          
		0,                
		0                  
		);

	glEnableVertexAttribArray(spr->attribute_v_texcoord);
	glBindBuffer(GL_ARRAY_BUFFER, spr->sprite_texcoords);
	glVertexAttribPointer(
		spr->attribute_v_texcoord, 
		2,                 
		GL_FLOAT,           
		GL_FALSE,           
		0,                 
		0                  
		);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glDisableVertexAttribArray(spr->attribute_v_coord);
	glDisableVertexAttribArray(spr->attribute_v_texcoord);
	glBindVertexArray(0);
	glDisable(GL_BLEND);
}