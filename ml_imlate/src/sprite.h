#include "sys/msys.h"
#include "intro.h"

typedef struct  
{
	float x,y,z;
	float rotate_radians;
	int xsize,ysize;
	GLuint texture;
	shader_id program;
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

	spr->program =initShader(vertex_source, (const char*)fragment_source);
	spr->attribute_v_coord = glGetAttribLocation(spr->program.vsid, "v_coord");
	spr->attribute_v_texcoord = glGetAttribLocation(spr->program.vsid, "v_texcoord");
	spr->uniform_mvp = glGetUniformLocation(spr->program.vsid, "mvp");
	spr->uniform_mytexture = glGetUniformLocation(spr->program.fsid, "mytexture");
	spr->uniform_xoffset = glGetUniformLocation(spr->program.fsid, "xoffset");
	spr->uniform_yoffset = glGetUniformLocation(spr->program.fsid, "yoffset");
	spr->uniform_color = glGetUniformLocation(spr->program.fsid, "color");
}

void draw_sprite(sprite * spr, int xres, int yres)
{
	glBindProgramPipeline(spr->program.pid);
	gbMat4 projection;
	gbMat4 m_transform;
	gbMat4 model;
	gbMat4 mvp;
	gbMat4 scale;
	float  tX = spr->xsize / 2.0f;
    float tY = spr->ysize / 2.0f;
	gb_mat4_ortho2d(&projection, 0.0, (float)asset_xr, (float)asset_yr, 0.0);
	gb_mat4_translate(&m_transform, gb_vec3((float)spr->x-tX, (float)spr->y-tY, 0.0));
	gb_mat4_rotate(&model,gb_vec3(0., 0., 1.0), spr->rotate_radians);
	gb_mat4_scale(&scale, gb_vec3(2.0, 2.0, 2.0));
	gb_mat4_mul(&model, &scale, &model);
	gb_mat4_mul(&mvp, &m_transform, &model);
	gb_mat4_mul(&mvp, &projection, &mvp);
	glProgramUniformMatrix4fv(spr->program.vsid,spr->uniform_mvp, 1, GL_FALSE,(float*)gb_float44_m(&mvp));

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, spr->texture);
	glProgramUniform1i(spr->program.fsid,spr->uniform_mytexture,0);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);

	glProgramUniform1i(spr->program.fsid,spr->uniform_xoffset,spr->xtexoffset);
	glProgramUniform1i(spr->program.fsid,spr->uniform_yoffset,spr->ytexoffset);

	float fparams[4] = { spr->rcol/255.0,spr->gcol/255.0,spr->bcol/255.0,spr->acol/255.0 };
	glProgramUniform4fv(spr->program.fsid, spr->uniform_color, 1, fparams);
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
	glBindProgramPipeline(0);
}