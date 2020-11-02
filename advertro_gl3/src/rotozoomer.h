#include "sys/msys.h"
#include "intro.h"


#include <stdint.h>
#include "ra_tile.h"

GLuint rotozoomer_tex; 
unsigned char* rotozoomer_data;


#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp" 


#include <iostream>
#include <string>
#include <vector>


int rotoheight = 512;
int rotowidth = 512;
GLuint rototex;
unsigned char rotodata[512*512*4];





static void rotozoom(uint8_t *dest,
	const uint32_t width, const uint32_t height,
	const uint8_t *texture,
	const float cX, const float cY,
	const float t)
{
	if (!dest || !texture || width < 1U || height < 1U)
		return;
	const float scale = (sin(t) + 1.0f) * 1.0f + 0.1f,
		s = sin(t) * scale,
		c = cos(t) * scale;
	for (uint32_t y = 0U; y < height; y++) {
		const float sy = (y - cY) * s,
			cy = (y - cY) * c;
		float pX = -cX * c - sy,
			pY = -cY * s + cy;
		const float dX = ((1.0f - cX) * c - sy) - pX,
			dY = ((1.0f - cY) * s + cy) - pY;
		pX += cX + 2048.0f;
		pY += cY + 2048.0f;

		for (uint32_t x = 0U; x < width; x++) {
			const int32_t tX = (int32_t)pX,
				tY = (int32_t)pY;
			const uint32_t utX = (uint32_t)tX,
				utY = (uint32_t)tY;
			//use point sampling
			const uint8_t *pa = texture +  ((((utY       & 255) << 8) +  (utX       & 255)) * 3);
			*dest++ = (uint8_t)pa[0];
			*dest++ = (uint8_t)pa[1];
			*dest++ = (uint8_t)pa[2];
			// next pixel
			pX += dX;
			pY += dY;
			dest++;
		}
	}
}


GLuint bg_shader_program,bg_texture,bg_vao; 

void init_rotozoomer()
{

	const char vertex_source [] =
        "#version 330\n"
		"layout(location = 0) in vec4 vposition;\n"
		"layout(location = 1) in vec2 vtexcoord;\n"
		"out vec2 ftexcoord;\n"
		"void main() {\n"
		"   ftexcoord = vtexcoord;\n"
		"   gl_Position =vposition;\n"
		"}\n";

	const char  fragment_source [] =
		"#version 330\n"
		"in vec2 ftexcoord;\n"
		"layout(location = 0) out vec4 FragColor;\n"
		"uniform sampler2D tex;\n"
		"uniform float time;\n"
		"uniform vec2 resolution;\n"
		"void main()"
		"{"
		"FragColor=texture2D(tex,ftexcoord);"
		"}";

	
	initShader(  (int*)&bg_shader_program, vertex_source, (const char*)fragment_source);
	GLint texture_location = glGetUniformLocation(bg_shader_program, "myTextureSampler");
	GLuint vbo;
	glGenVertexArrays(1, &bg_vao);
	glBindVertexArray(bg_vao);
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);

	// data for a fullscreen quad (this time with texture coords)
	//originally was a quad, but converted to triangles
	GLfloat vertexData[] = {
		//  X     Y     Z           U     V     
		1.0f, 1.0f, 0.0f,       1.0f, 1.0f, // vertex 0
		-1.0f, 1.0f, 0.0f,       0.0f, 1.0f, // vertex 1
		1.0f,-1.0f, 0.0f,       1.0f, 0.0f, // vertex 2
		1.0f,-1.0f, 0.0f,       1.0f, 0.0f, // vertex 2
		-1.0f,-1.0f, 0.0f,       0.0f, 0.0f, // vertex 3
		-1.0f, 1.0f, 0.0f,       0.0f, 1.0f, // vertex 1
	}; // 6 vertices with 5 components (floats) each
	
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)*6*5, vertexData, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5*sizeof(GLfloat), (char*)0 + 0*sizeof(GLfloat));

	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5*sizeof(GLfloat), (char*)0 + 3*sizeof(GLfloat));
	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	memset(rotodata,0xff,rotowidth*rotoheight*4);
	glGenTextures( 1, &rototex );
	glBindTexture( GL_TEXTURE_2D, rototex );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
	glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, rotowidth, rotoheight, 0,
		GL_RGBA, GL_UNSIGNED_BYTE, rotodata );
	int comp;
	int rotozoomer_h, rotozoomer_w;
	rotozoomer_data= stbi_load_from_memory(ra_tile,ra_tile_len,&rotozoomer_w,&rotozoomer_h,&comp,3);
	
}



static void lissajous(const uint32_t width, const uint32_t height, const float t, float &x, float &y)
{
	static const float kMaxCoverage = 0.75f / 2.0f;
	const float AA = width * kMaxCoverage,           
		BB = height * kMaxCoverage,
		a = 2.0f,                          
		b = 3.0f,
		gamma = 0.0f;

	x = width / 2.0f  + AA * sin(a * t + gamma);
	y = height / 2.0f + BB * sin(b * t);
}


void draw_rotozoomer(float scenetime){
	static float t = 0.0f;
	float cX, cY;
	lissajous(rotowidth, rotoheight, t / 5.0f, cX, cY);
	rotozoom(rotodata, rotowidth, rotoheight, rotozoomer_data, cX, cY, t);


	glViewport(0,0,XRES,YRES);
	glUseProgram(bg_shader_program);
	GLint texture_location = glGetUniformLocation(bg_shader_program, "tex");
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D,rototex);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, rotowidth, rotoheight, 
		GL_RGBA, GL_UNSIGNED_BYTE, rotodata);
	glUniform1i(texture_location, 0);
	float time = scenetime;
	GLuint time_loc = glGetUniformLocation(bg_shader_program, "time");
	glUniform1f(time_loc,time);
	GLuint resolution_loc = glGetUniformLocation(bg_shader_program, "resolution");
	glUniform2f(resolution_loc,XRES,YRES);
	glBindVertexArray(bg_vao);
	glDrawArrays(GL_TRIANGLES, 0,6);
	glBindTexture(GL_TEXTURE_2D,0);
	glBindVertexArray(0);


	t += 0.01f;
}