#include "sys/msys.h"
#include "intro.h"

#include <stdint.h>
#include "ra_gradient.h"

GLuint gradient_tex; 
unsigned char *gradient_data;
int gradient_h, gradient_w;

typedef struct  
{
	float x,y,z;
	int xsize,ysize;
	int letter_size;
	int frame_number;
	GLuint texture;
	GLuint program;
	float xtexoffset,ytexoffset;
	GLuint sprite_vertices, sprite_texcoords, vao;
	GLint attribute_v_coord, attribute_v_texcoord;
	GLint uniform_mvp, uniform_mytexture,uniform_xoffset,uniform_yoffset;
	GLint uniform_color;
	float rcol,gcol,bcol,acol;
}sprite;

sprite topgradient;
sprite bottomgradient;

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
		0,    0,
		spr->xsize,  0, 
		0, spr->ysize,
		spr->xsize,spr->ysize,
	};

	glEnableVertexAttribArray(spr->attribute_v_coord);
	glBindBuffer(GL_ARRAY_BUFFER,spr->sprite_vertices);
	glBufferData(GL_ARRAY_BUFFER, sizeof(sprite_vertices), sprite_vertices, GL_STATIC_DRAW);

	glVertexAttribPointer(
		spr->attribute_v_coord, 
		2,                
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

GLuint make_texture (int width,int height, unsigned char* data)
{
	GLuint texture;
	glGenTextures( 1, &texture );
	glBindTexture( GL_TEXTURE_2D, texture );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
	glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB, width,height, 0,
		GL_RGB, GL_UNSIGNED_BYTE, data );
	return texture;
}

unsigned char* flip(int width, int height,int channels, unsigned char* data)
{
	unsigned char *img = (unsigned char*)malloc( width*height*channels );
	memcpy( img, data, width*height*channels );
	int i, j;
	for( j = 0; j*2 < height; ++j )
	{
		int index1 = j * width * channels;
		int index2 = (height - 1 - j) * width * channels;
		for( i = width * channels; i > 0; --i )
		{
			unsigned char temp = img[index1];
			img[index1] = img[index2];
			img[index2] = temp;
			++index1;
			++index2;
		}
	}
	return img;
}

void init_gradients()
{

	int comp;

	unsigned char *gradient1 = stbi_load_from_memory(ra_gradient,ra_gradient_len,&gradient_w,&gradient_h,&comp,0);
	gradient_data = flip(gradient_w,gradient_h,comp,gradient1);
	stbi_image_free( gradient1 );
	//gradient textures
	memset(&topgradient,0,sizeof(sprite));
	memset(&bottomgradient,0,sizeof(sprite));

	GLuint tex_topgrad = make_texture(gradient_w,gradient_h,gradient_data);

	topgradient.xsize = XRES;
	topgradient.ysize = 50;
	topgradient.x = 0;
	topgradient.y = 0;
	topgradient.texture =tex_topgrad;
	topgradient.acol = 255;
	topgradient.rcol = 255;
	topgradient.gcol = 255;
	topgradient.bcol = 255;

	bottomgradient.xsize = XRES;
	bottomgradient.ysize = 50;
	bottomgradient.x = 0;
	bottomgradient.y = YRES - 50;
	bottomgradient.texture =tex_topgrad;
	bottomgradient.acol = 255;
	bottomgradient.rcol = 255;
	bottomgradient.gcol = 255;
	bottomgradient.bcol = 255;

	init_sprite(&topgradient);
	init_sprite(&bottomgradient);
}

void draw_gradients(float scenetime){

	static float floor_move_x = 0.0;
	static float floor_move_y = 0.0;

	floor_move_x-=0.011;
	floor_move_y+=0.011;

	topgradient.xtexoffset=floor_move_x;
	topgradient.ytexoffset=0.0;
	bottomgradient.xtexoffset=floor_move_y;
	bottomgradient.ytexoffset=0.0;

	draw_sprite(&topgradient,XRES,YRES);
	draw_sprite(&bottomgradient,XRES,YRES);
}