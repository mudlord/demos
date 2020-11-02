GLuint raymarch_shader_program[5],raymarch_vao, raymarch_texture;
GLuint tex1_tex, tex2_tex;

#include "tex1.h"
#include "tex2.h"

#ifdef PARTY_VERSION
#include "shaders_party.h"
#else
#include "shaders_hq.h"
#endif

const char vertex_source[] =
"#version 330\n"
"layout(location = 0) in vec4 vposition;\n"
"layout(location = 1) in vec2 vtexcoord;\n"
"out vec2 ftexcoord;\n"
"void main() {\n"
"   ftexcoord = vtexcoord;\n"
"   gl_Position =vec4(vposition.xy, 0.0f, 1.0f);\n"
"}\n";

void init_raymarch()
{
	initShader((int*)&raymarch_shader_program[0], vertex_source, (const char*)scene1_shader);
	initShader((int*)&raymarch_shader_program[2], vertex_source, (const char*)scene3_shader);
	initShader((int*)&raymarch_shader_program[3], vertex_source, (const char*)scene4_shader);
	initShader((int*)&raymarch_shader_program[4], vertex_source, (const char*)scene5_shader);

	// get texture uniform location


	// vao and vbo handle
	GLuint vbo;

	// generate and bind the vao
	glGenVertexArrays(1, &raymarch_vao);
	glBindVertexArray(raymarch_vao);

	// generate and bind the vertex buffer object
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);

	// data for a fullscreen quad (this time with texture coords)
	GLfloat vertexData[] = {
		//  X     Y     Z           U     V     
		1.0f, 1.0f, 0.0f, 1.0f, 1.0f, // vertex 0
		-1.0f, 1.0f, 0.0f, 0.0f, 1.0f, // vertex 1
		1.0f, -1.0f, 0.0f, 1.0f, 0.0f, // vertex 2
		1.0f, -1.0f, 0.0f, 1.0f, 0.0f, // vertex 2
		-1.0f, -1.0f, 0.0f, 0.0f, 0.0f, // vertex 3
		-1.0f, 1.0f, 0.0f, 0.0f, 1.0f, // vertex 1
	}; // 6 vertices with 5 components (floats) each

	// fill with data
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 6 * 5, vertexData, GL_STATIC_DRAW);
	// set up generic attrib pointers
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (char*)0 + 0 * sizeof(GLfloat));

	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (char*)0 + 3 * sizeof(GLfloat));
	// "unbind" voa
	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	int width = 32;
	int height = 32;
	GLubyte image[4 * 32 * 32] = { 0 };
	for (int j = 0; j < height; ++j) {
		for (int i = 0; i < width; ++i) {
			size_t index = j*width + i;
			image[4 * index + 0] = 0xFF * (j / 10 % 2)*(i / 10 % 2); // R
			image[4 * index + 1] = 0xFF * (j / 13 % 2)*(i / 13 % 2); // G
			image[4 * index + 2] = 0xFF * (j / 17 % 2)*(i / 17 % 2); // B
			image[4 * index + 3] = 0xFF; // A
		}
	}

	glGenTextures(1, &raymarch_texture);
	glBindTexture(GL_TEXTURE_2D, raymarch_texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	// set texture content
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
	glBindTexture(GL_TEXTURE_2D, 0);
	int width2, height2;
	tex1_tex = loadTexMemory(tex1_png, tex1_png_len, &width2, &height2,1);
	tex2_tex = loadTexMemory(tex2_png, tex2_png_len, &width2, &height2,1);
}

void draw_raymarch(float time, float camx, float camy, float camz, int program){
	glUseProgram(raymarch_shader_program[program]);
	GLint texture_location = glGetUniformLocation(raymarch_shader_program[program], "tex");
	GLint texture_location2 = glGetUniformLocation(raymarch_shader_program[program], "texTex1");
	GLint texture_location3 = glGetUniformLocation(raymarch_shader_program[program], "texTex2");
	GLint uniform_resolution = glGetUniformLocation(raymarch_shader_program[program], "v2Resolution");
	GLint uniform_time = glGetUniformLocation(raymarch_shader_program[program], "fGlobalTime");

	GLint uniform_camx = glGetUniformLocation(raymarch_shader_program[program], "camx");
	GLint uniform_camy = glGetUniformLocation(raymarch_shader_program[program], "camy");
	GLint uniform_camz = glGetUniformLocation(raymarch_shader_program[program], "camz");


	GLint uniform_sc1_mengertweak1 = glGetUniformLocation(raymarch_shader_program[program], "sc1_mengertweak1");
	GLint uniform_sc1_mengertweak2 = glGetUniformLocation(raymarch_shader_program[program], "sc1_mengertweak2");
	GLint uniform_sc1_mengertweak3 = glGetUniformLocation(raymarch_shader_program[program], "sc1_mengertweak3");

	GLuint uniform_speed = glGetUniformLocation(raymarch_shader_program[program], "speed");

	GLint uniform_sc3_rotatex = glGetUniformLocation(raymarch_shader_program[program], "sc3_rotatex");
	GLint uniform_sc3_rotatey = glGetUniformLocation(raymarch_shader_program[program], "sc3_rotatey");
	GLint uniform_sc3_rotatez = glGetUniformLocation(raymarch_shader_program[program], "sc3_rotatez");

	GLint uniform_sc3_tweak1 = glGetUniformLocation(raymarch_shader_program[program], "sc3_tweak1");
	GLint uniform_sc3_tweak2 = glGetUniformLocation(raymarch_shader_program[program], "sc3_tweak2");
	GLint uniform_sc3_tweak3 = glGetUniformLocation(raymarch_shader_program[program], "sc3_tweak3");
	GLint uniform_sc3_tweak4 = glGetUniformLocation(raymarch_shader_program[program], "sc3_tweak4");
	GLint uniform_sc3_tweak5 = glGetUniformLocation(raymarch_shader_program[program], "sc3_tweak5");
	GLint uniform_sc3_mengerscale = glGetUniformLocation(raymarch_shader_program[program], "sc3_mengerscale");
	GLint uniform_sc3_lightscale = glGetUniformLocation(raymarch_shader_program[program], "sc3_lightscale");
	GLint uniform_sc3_fogscale = glGetUniformLocation(raymarch_shader_program[program], "sc3_fogscale");

	// set the uniform
	// bind texture to texture unit 0
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, raymarch_texture);
	glUniform1i(texture_location, 0);
	glActiveTexture(GL_TEXTURE0 + 1);
	glBindTexture(GL_TEXTURE_2D, tex1_tex);
	glUniform1i(texture_location2, 1);
	glActiveTexture(GL_TEXTURE0 + 2);
	glBindTexture(GL_TEXTURE_2D, tex2_tex);
	glUniform1i(texture_location3, 2);

	glUniform2f(uniform_resolution, XRES, YRES);
	glUniform1f(uniform_time, time);
	glUniform1f(uniform_camx, camx);
	glUniform1f(uniform_camy, camy);
	glUniform1f(uniform_camz, camz);


	glUniform1f(uniform_sc1_mengertweak1, sc1_mengertweak1);
	glUniform1f(uniform_sc1_mengertweak2, sc1_mengertweak2);
	glUniform1f(uniform_sc1_mengertweak3, sc1_mengertweak3);

	glUniform1f(uniform_speed, speed_f);

	glUniform1f(uniform_sc3_rotatex, sc3_rotatex);
	glUniform1f(uniform_sc3_rotatey, sc3_rotatey);
	glUniform1f(uniform_sc3_rotatez, sc3_rotatez);

	glUniform1f(uniform_sc3_tweak1, sc3_tweak1);
	glUniform1f(uniform_sc3_tweak2, sc3_tweak2);
	glUniform1f(uniform_sc3_tweak3, sc3_tweak3);
	glUniform1f(uniform_sc3_tweak4, sc3_tweak4);
	glUniform1f(uniform_sc3_tweak5, sc3_tweak5);
	glUniform1f(uniform_sc3_mengerscale, sc3_mengerscale);
	glUniform1f(uniform_sc3_lightscale, sc3_lightscale);
	glUniform1f(uniform_sc3_fogscale, sc3_fogscale);



	// bind the vao
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glBindVertexArray(raymarch_vao);
	// draw
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glBindTexture(GL_TEXTURE_2D, 0);
	glBindVertexArray(0);
	glUseProgram(0);
	glDisable(GL_BLEND);
}