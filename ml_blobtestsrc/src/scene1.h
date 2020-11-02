#include "sys/msys.h"
#include "intro.h"

#include "bg_tex.h"

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp" 
#include <new>

#include "sync/sync.h"
#include "syncs.h"

const struct sync_track *sc1_a;
const struct sync_track *sc1_b;
const struct sync_track *sc1_m;
const struct sync_track *sc1_n1;
const struct sync_track *sc1_n2;
const struct sync_track *sc1_n3;
const struct sync_track *sc1_red;
const struct sync_track *sc1_green;
const struct sync_track *sc1_blue;
const struct sync_track *transition_x;
const struct sync_track *transition_y;
const struct sync_track *transition_xscale;
const struct sync_track *transition_yscale;
struct sync_device *rocket;

template <class T>
class CBuffer
{
public:
	typedef typename std::size_t size_type;
private:
	size_type _size;
	size_type _capacity;
	T* _data;

	static T*
		allocate(size_type size)
	{
		return static_cast<T*>(malloc(sizeof(T) * size));
	}
	static void
		copyRange(T* begin, T* end, T* dest)
	{
		while(begin != end)
		{
			new((void*)dest) T(*begin);
			++begin;
			++dest;
		}
	}
	static void
		deleteRange(T* begin, T* end)
	{
		while(begin != end)
		{
			begin->~T();
			++begin;
		}
	}

public:
	typedef T* iterator;
	typedef T value_type;

	CBuffer()
	{
		_size = 0;
		_capacity = 0;
		_data = 0;
	}
	~CBuffer()
	{
		deleteRange(_data, _data + _size);
		free(_data);
	}

	// needs some other constructors
	// size(), empty() and a bunch of other methods!

	int size()
	{
		return _size;
	}

	void clear()
	{
		deleteRange(_data, _data + _size);
		free(_data);
		_size = 0;
		_capacity = 0;
		_data = 0;
	}

	void
		push_back(const T& value)
	{
		if(_size != _capacity)
		{
			new((void*)(_data + _size)) T(value);
			++_size;
			return;
		}
		size_type newCapacity = _capacity ? _capacity * 2 : 1;
		T* newData = allocate(newCapacity);
		copyRange(_data, _data + _size, newData);
		new((void*)(newData + _size)) T(value);
		deleteRange(_data, _data + _size);
		free(_data);
		_data = newData;
		_capacity = newCapacity;
		++_size;
	}
	T&
		operator[](size_type index)
	{
		return _data[index];
	}
	const T&
		operator[](size_type index) const
	{
		return _data[index];
	}
	T*
		begin() const
	{
		return _data;
	}
	T*
		end() const
	{
		return _data + _size;
	}
};


#define M_PI 3.14159265358979323846
#define TWOPI M_PI*2
#define S_SLICES  64
#define S_BANDS  64
#define S_RADIUS  1.0

typedef struct VECTOR3D
{
	float x, y, z;
}vectors;

CBuffer<glm::vec3>sshape_normals;
CBuffer<glm::vec3>sshape_vertices;
CBuffer<glm::vec3>sshape_colour;
GLuint sshape_shader_program,sshape_vao,sshape_postpoc,sshape_postpoc_vao;
FBOELEM sshape_fbo;
GLuint sshape_vertexbuffer;
GLuint sshape_normalbuffer;
GLuint sshape_colourbuffer;

float sshape_red = 60.0;
float sshape_green = 175.0;
float sshape_blue = 185.0;

float sshape_a ;
float sshape_b;
float sshape_m ;
float sshape_n1;
float sshape_n2;
float sshape_n3;

float transitions_x;
float transitions_y;
float transitions_xscale;
float transitions_yscale;

void normalize(VECTOR3D *v)
{
	float mag;
	mag = 1 / sqrt(v->x * v->x + v->y * v->y + v->z * v->z);
	if (mag != 0)
	{
		v->x = v->x * mag;
		v->y = v->y * mag;
		v->z = v->z * mag;
	}
	else
	{
		v->x = 0.0;
		v->y = 0.0;
		v->z = 1.0;
	}


}

static void Eval2D(float a,float b,float m, float n1, float n2, float n3, float phi, float * x, float * y){
	double r;
	double t1, t2;

	t1 = cos(m * phi / 4) / a;
	t1 = abs(t1);
	t1 = pow(t1, (double)n2);

	t2 = sin(m * phi / 4) / b;
	t2 = abs(t2);
	t2 =pow(t2, (double)n3);

	r = pow(t1 + t2, (double)1 / n1);
	if (abs(r) == 0) {
		x = 0;
		y = 0;
	}
	else {
		r = 1 / r;
		*x = (float)(r * cos(phi));
		*y = (float)(r * sin(phi));
	}
}

static void Eval3D(float a, float b,float m, float n1, float n2, float n3, float phi, float theta, float * x, float * y, float * z){
	double r;
	double t1, t2;

	Eval2D(a,b,m, n1, n2, n3, phi, x, y);

	t1 = cos(m * theta / 4) / a;
	t1 = abs(t1);
	t1 = pow(t1, (double)n2);

	t2 = sin(m * theta / 4) / b;
	t2 = abs(t2);
	t2 = pow(t2, (double)n3);

	r = pow(t1 + t2, (double)1 / n1);
	if (abs(r) == 0) {
		x = 0;
		y = 0;
		z = 0;
	}
	else {
		r = 1 / r;
		(*x) *= (float)(r * cos(theta));
		(*y) *= (float)(r * cos(theta));
		*z = (float)(r * sin(theta));
	}
}

void DoCalc(float stepsx, float stepsy,float a,float b, float m, float n1, float n2, float n3) {
	float phi = -(float)M_PI;
	float addPhi = (float)M_PI * 2 / stepsx;
	float theta = 0.0;
	float addTheta = (float)M_PI / stepsy;
	float x1 = 0, y1 = 0, z1 = 0;
	float x2 = 0, y2 = 0, z2 = 0;
	float x3 = 0, y3 = 0, z3 = 0;
	float x4 = 0, y4 = 0, z4 = 0;
	float scale = 1;
	for (int i = 0; i <= stepsx; i++) {
		theta = -(float)M_PI / 2;
		for (int j = 0; j < stepsy; j++) {
			Eval3D(a,b,m, n1, n2, n3, phi, theta, &x1, &y1, &z1);
			Eval3D(a, b, m, n1, n2, n3, phi, theta + addTheta, &x2, &y2, &z2);
			Eval3D(a, b, m, n1, n2, n3, phi + addPhi, theta + addTheta, &x3, &y3, &z3);
			Eval3D(a, b, m, n1, n2, n3, phi + addPhi, theta, &x4, &y4, &z4);
			//i know, splitting glquads to triangles
			VECTOR3D vertices = { x1,y1,z1 };
			normalize(&vertices);
			sshape_normals.push_back(glm::vec3(vertices.x, vertices.y,vertices.z));
			sshape_vertices.push_back(glm::vec3(x1*scale, y1*scale, z1*scale)); 
			sshape_colour.push_back(glm::vec3((float)sshape_red/255, (float)sshape_green/255,(float)sshape_blue/255)); 
			vertices.x = x2, vertices.y =  y2,vertices.z= z2 ;
			normalize(&vertices);
			sshape_normals.push_back(glm::vec3(vertices.x, vertices.y, vertices.z));
			sshape_vertices.push_back(glm::vec3(x2*scale, y2*scale, z2*scale));
			sshape_colour.push_back(glm::vec3((float)sshape_red/255, (float)sshape_green/255,(float)sshape_blue/255)); 
			vertices.x = x3, vertices.y =  y3,vertices.z= z3 ;
			normalize(&vertices);
			sshape_normals.push_back(glm::vec3(vertices.x, vertices.y, vertices.z));
			sshape_vertices.push_back(glm::vec3(x3*scale, y3*scale, z3*scale));
			sshape_colour.push_back(glm::vec3((float)sshape_red/255, (float)sshape_green/255,(float)sshape_blue/255)); 
			vertices.x = x3, vertices.y =  y3,vertices.z= z3 ;
			normalize(&vertices);
			sshape_normals.push_back(glm::vec3(vertices.x, vertices.y, vertices.z));
			sshape_vertices.push_back(glm::vec3(x3*scale, y3*scale, z3*scale));
			sshape_colour.push_back(glm::vec3((float)sshape_red/255, (float)sshape_green/255,(float)sshape_blue/255)); 
			vertices.x = x4, vertices.y =  y4,vertices.z= z4 ;
			normalize(&vertices);
			sshape_normals.push_back(glm::vec3(vertices.x, vertices.y, vertices.z));
			sshape_vertices.push_back(glm::vec3(x4*scale, y4*scale, z4*scale));
			sshape_colour.push_back(glm::vec3((float)sshape_red/255, (float)sshape_green/255,(float)sshape_blue/255)); 
			vertices.x = x1,vertices.y = y1,vertices.z =z1;
			normalize(&vertices);
			sshape_normals.push_back(glm::vec3(vertices.x, vertices.y,vertices.z));
			sshape_vertices.push_back(glm::vec3(x1*scale, y1*scale, z1*scale)); 
			sshape_colour.push_back(glm::vec3((float)sshape_red/255, (float)sshape_green/255,(float)sshape_blue/255)); 

			theta += addTheta;
		}
		phi += addPhi;
	}
}


void init_sshape()
{
	const char vertex_source [] =
		"#version 330\n"
		"uniform mat4 MVP;\n" // the modelviewprojection matrix uniform
		"uniform mat4 modelviewMatrix;\n" // the modelview matrix
		"layout(location = 0) in vec4 vposition;\n"
		"layout(location = 1) in vec4 vcolor;\n"
		"layout(location = 2) in vec4 vnormal;\n"
		"out vec4 fcolor;\n"
		"out vec3 LD, Normal, LDR;\n"
		"void main() {\n"
		"   mat4 normalMatrix = transpose(inverse(modelviewMatrix));\n"
		"   LD = -(modelviewMatrix* vposition).xyz;"
		"   Normal = (normalMatrix * vnormal).xyz;\n"
		"   LDR = reflect(-LD, Normal);\n"
		"   gl_Position = MVP*vposition;\n"
		"   fcolor = vcolor;\n"

		"}\n";
	const char fragment_source [] =
		"#version 330\n"
		"in vec4 fcolor;\n"
		"in vec4 gFrontColor;\n"
		"in vec3 LD, Normal, LDR;\n"
		"layout(location = 0) out vec4 FragColor;\n"
		"void main() {\n"
		"vec3 L=normalize(LD),d=normalize(Normal),r=normalize(LDR);\n"
		"float l=max(dot(d,L),0.0);\n"
		"FragColor = vec4(fcolor.rgb *l,1.0);\n"
		"}\n";
	
	initShader(  (int*)&sshape_shader_program, vertex_source, (const char*)fragment_source);

	glGenVertexArrays(1, &sshape_vao);
	glBindVertexArray(sshape_vao);


	float a = 1.0, b = 1.0,m = 12.0, n1 = 1.0 , n2 = 1., n3 = 1.0f;
	DoCalc(40, 40,a,b, m, n1, n2, n3);

	glGenBuffers(1, &sshape_vertexbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, sshape_vertexbuffer);
	glBufferData(GL_ARRAY_BUFFER,sshape_vertices.size() * sizeof(glm::vec3),NULL, GL_DYNAMIC_DRAW);
	void *ptr = (void*)glMapBuffer(GL_ARRAY_BUFFER,GL_WRITE_ONLY);
	int err = glGetError();
	if(ptr)
	{
		memcpy(ptr, &sshape_vertices[0], sshape_vertices.size() * sizeof(glm::vec3));
		glUnmapBuffer(GL_ARRAY_BUFFER);
	}


	glGenBuffers(1, &sshape_normalbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, sshape_normalbuffer);
	glBufferData(GL_ARRAY_BUFFER,sshape_normals.size() * sizeof(glm::vec3),&sshape_normals[0], GL_DYNAMIC_DRAW);
	glGenBuffers(1, &sshape_colourbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, sshape_colourbuffer);
	glBufferData(GL_ARRAY_BUFFER,sshape_colour.size() * sizeof(glm::vec3),&sshape_colour[0], GL_DYNAMIC_DRAW);

	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, sshape_vertexbuffer);
	glVertexAttribPointer(
		0,                  // attribute. No particular reason for 0, but must match the layout in the shader.
		3,                  // size
		GL_FLOAT,           // type
		GL_FALSE,           // normalized?
		0,                  // stride
		(void*)0            // array buffer offset
		);
	// 2nd attribute buffer : colors
	glEnableVertexAttribArray(1);
	glBindBuffer(GL_ARRAY_BUFFER, sshape_colourbuffer);
	glVertexAttribPointer(
		1,                                // attribute. No particular reason for 1, but must match the layout in the shader.
		3,                                // size
		GL_FLOAT,                         // type
		GL_FALSE,                         // normalized?
		0,                                // stride
		(void*)0                          // array buffer offset
		);

	glEnableVertexAttribArray(2);
	glBindBuffer(GL_ARRAY_BUFFER, sshape_normalbuffer);
	glVertexAttribPointer(
		2,                                // attribute. No particular reason for 1, but must match the layout in the shader.
		3,                                // size
		GL_FLOAT,                         // type
		GL_FALSE,                         // normalized?
		0,                                // stride
		(void*)0                          // array buffer offset
		);

	// "unbind" vao
	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	sshape_fbo = init_fbo(XRES,YRES);
}

#define PI 3.1415926535897932384626433832795
#define ANG2RAD PI/180.0f


static GLfloat colCnt = 0.0f;


void draw_sshape(float scenetime)
{

	glClearColor(sshape_red/255, sshape_green/255,sshape_blue/255, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	float t = scenetime;
	glUseProgram(sshape_shader_program);
	GLuint cube_projection = glGetUniformLocation(sshape_shader_program, "MVP");
	// calculate ViewProjection matrix
	glm::mat4 Projection = glm::perspective(45.0f, (float)XRES / (float)YRES, 0.1f, 100.f);
	glm::mat4 Model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -3.0f));
	Model = glm::rotate(Model, 90.0f*t, glm::vec3(1.0f, 1.0f, 1.0f)); 
	glm::mat4 View = glm::mat4(1.0f);
	glm::mat4 ViewProjection = Projection*View*Model;
	// set the uniform
	glm::mat4 MV = View * Model;
	GLuint modelmatrix_loc = glGetUniformLocation(sshape_shader_program, "modelviewMatrix");
	glUniformMatrix4fv(modelmatrix_loc, 1, GL_FALSE, glm::value_ptr(MV));
	glUniformMatrix4fv(cube_projection, 1, GL_FALSE, glm::value_ptr(ViewProjection)); 
	// bind the vao

	static long lastTime =  scenetime;
	long currTime = scenetime;
	static double delta = 0.0;
	long diff= currTime - lastTime;
	delta = diff;

	static float synctime = 0.0;

	double row = synctime * 5.0;
	synctime += (float)delta/1000.f;

	sshape_vertices.clear();
	sshape_normals.clear();
	sshape_colour.clear();

	//float a = 1.0, b = 1.0,m = 20.0, n1 = 1.0 , n2 = 1., n3 = 1.0f;

#ifndef SYNC_PLAYER
	if (sync_update(rocket, (int)floor(row), NULL, NULL))
		sync_connect(rocket, "localhost", SYNC_DEFAULT_PORT);
#endif

	sshape_a = sync_get_val(sc1_a,row);
	sshape_b = sync_get_val(sc1_b,row);
	sshape_m =  sync_get_val(sc1_m, row);
	sshape_n1 = sync_get_val(sc1_n1,row);
	sshape_n2 = sync_get_val(sc1_n2,row);
	sshape_n3 = sync_get_val(sc1_n3,row);

	transitions_x = sync_get_val(transition_x,row);
	transitions_y = sync_get_val(transition_y,row);
	transitions_xscale = sync_get_val(transition_xscale,row);
	transitions_yscale = sync_get_val(transition_yscale,row);



	if(row > 670 && row  < 1550)
	{


		sshape_red = .31 + sin(scenetime * 7.  + ( (float)1 * 3. * ANG2RAD ) ) * 0.3;
		sshape_red *= 255.0;
		sshape_green = .68 + sin(scenetime * 7.  + ( (float)1 * 3. * ANG2RAD ) ) * 0.3;
		sshape_green *= 255.0;
		sshape_blue = .72 + sin(scenetime * 7.  + ( (float)1 * 3. * ANG2RAD ) ) * 0.3;
		sshape_blue *= 255.0;

	}
	else
	{
		sshape_red = sync_get_val(sc1_red,row);
		sshape_green = sync_get_val(sc1_green,row);
		sshape_blue = sync_get_val(sc1_blue,row);
	}


	DoCalc(40, 40,sshape_a,sshape_b, sshape_m, sshape_n1, sshape_n2, sshape_n3);

	glEnable(GL_DEPTH_TEST);

	glBindVertexArray(sshape_vao);

	glBindBuffer(GL_ARRAY_BUFFER,sshape_vertexbuffer);
	glBufferData(GL_ARRAY_BUFFER,sshape_vertices.size() * sizeof(glm::vec3),NULL, GL_DYNAMIC_DRAW);
	int err = glGetError();
	void *ptr = (void*)glMapBuffer(GL_ARRAY_BUFFER,GL_WRITE_ONLY);
	err = glGetError();
	if(ptr)
	{
		memcpy(ptr, &sshape_vertices[0], sshape_vertices.size() * sizeof(glm::vec3));
		glUnmapBuffer(GL_ARRAY_BUFFER);
	}

	glBindBuffer(GL_ARRAY_BUFFER,sshape_normalbuffer);
	glBufferData(GL_ARRAY_BUFFER,sshape_normals.size() * sizeof(glm::vec3),NULL, GL_DYNAMIC_DRAW);
	err = glGetError();
	ptr = (void*)glMapBuffer(GL_ARRAY_BUFFER,GL_WRITE_ONLY);
	err = glGetError();
	if(ptr)
	{
		memcpy(ptr, &sshape_normals[0], sshape_normals.size() * sizeof(glm::vec3));
		glUnmapBuffer(GL_ARRAY_BUFFER);
	}

	glBindBuffer(GL_ARRAY_BUFFER,sshape_colourbuffer);
	glBufferData(GL_ARRAY_BUFFER,sshape_colour.size() * sizeof(glm::vec3),NULL, GL_DYNAMIC_DRAW);
	err = glGetError();
	ptr = (void*)glMapBuffer(GL_ARRAY_BUFFER,GL_WRITE_ONLY);
	err = glGetError();
	if(ptr)
	{
		memcpy(ptr, &sshape_colour[0], sshape_colour.size() * sizeof(glm::vec3));
		glUnmapBuffer(GL_ARRAY_BUFFER);
	}

	// draw
	glDrawArrays(GL_TRIANGLES, 0,sshape_vertices.size());   
	glBindVertexArray(0);
	glDisable(GL_DEPTH_TEST);
}


void init_sshape_postproc()
{
	const char vertex_source [] = 
		"#version 330\n"
		"uniform mat4 ViewProjection;\n" // the projection matrix uniform
		"layout(location = 0) in vec4 vposition;\n"
		"layout(location = 1) in vec4 vuv;\n"
		"out vec2 vertexuv;\n"
		"void main() {\n"
		"   vertexuv = vuv.xy;\n"
		"   gl_Position =ViewProjection*vposition;\n"
		"}\n";

	const char fragment_source [] = 
		"#version 330\n"
		"in vec2 vertexuv;\n"
		"layout(location = 0) out vec4 FragColor;\n"
		"uniform sampler2D myTextureSampler;\n"
		"uniform float time;\n"
		"uniform float choir;\n"
		"uniform vec2 resolution;\n"
		"float r=64.0;\n"
		"float C=0.025,t=0.96875;\n"
		"void main() {\n"
		"if(choir==1.)\n"
		"{\n"
		"C=0.035;\n"
		"t=0.975;\n"
		"}\n"
		//"f.r=f.r+sin(f.g*10.+time)/30.*(1.+cos(time*80.))*r;"
		
		"vec2 m=vertexuv,v=vec2(0.5)-m;\n"
		"v/=r;\n"
		"vec4 g=texture2D(myTextureSampler,m);\n"
		"for(float f=0.0;f<r;f+= 1.0)\n"
		"g+=texture2D(myTextureSampler,m)*C,C*=t,m+=v;\n"
		"FragColor=g;\n"
		"}\n";
	initShader(  (int*)&sshape_postpoc, vertex_source, (const char*)fragment_source);

	GLuint vbo, ibo;
	// generate and bind the vao
	glGenVertexArrays(1, &sshape_postpoc_vao);
	glBindVertexArray(sshape_postpoc_vao);

	// generate and bind the vertex buffer object
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);

	// data for a fullscreen quad (this time with texture coords)
	GLfloat vertexData[] = {
		//  X     Y     Z           U     V     
		1.0f, 1.0f, 0.0f,       1.0f, 1.0f, // vertex 0
		-1.0f, 1.0f, 0.0f,       0.0f, 1.0f, // vertex 1
		1.0f,-1.0f, 0.0f,       1.0f, 0.0f, // vertex 2
		-1.0f,-1.0f, 0.0f,       0.0f, 0.0f, // vertex 3
	}; // 4 vertices with 5 components (floats) each

	// fill with data
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)*4*5, vertexData, GL_STATIC_DRAW);


	// set up generic attrib pointers
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5*sizeof(GLfloat), (char*)0 + 0*sizeof(GLfloat));

	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5*sizeof(GLfloat), (char*)0 + 3*sizeof(GLfloat));


	// generate and bind the index buffer object
	glGenBuffers(1, &ibo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);

	GLuint indexData[] = {
		0,1,2, // first triangle
		2,1,3, // second triangle
	};

	// fill with data
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint)*2*3, indexData, GL_STATIC_DRAW);

	// "unbind" vao
	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

}

float choir;
void draw_sshape_postproc(float scenetime)
{
	glUseProgram(sshape_postpoc);
	GLint texture_location = glGetUniformLocation(sshape_postpoc, "myTextureSampler");
	glm::mat4 Projection =  glm::mat4(1.0); 
	glm::mat4 ViewProjection = Projection;
	// set the uniform
	GLuint cube_projection = glGetUniformLocation(sshape_postpoc, "ViewProjection");
	glUniformMatrix4fv(cube_projection, 1, GL_FALSE, glm::value_ptr(ViewProjection)); 
	// bind texture to texture unit 0
	glActiveTexture(GL_TEXTURE0 );
	glBindTexture(GL_TEXTURE_2D, sshape_fbo.texture);
	// set texture uniform
	glUniform1i(texture_location, 0);

	float time = scenetime;
	GLuint time_loc = glGetUniformLocation(sshape_postpoc, "time");
	glUniform1f(time_loc,time);
	GLuint choir_loc = glGetUniformLocation(sshape_postpoc, "choir");
	glUniform1f(choir_loc,choir);
	GLuint resolution_loc = glGetUniformLocation(sshape_postpoc, "resolution");
	glUniform2f(resolution_loc,XRES,YRES);
	// bind the vao
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glBindVertexArray(sshape_postpoc_vao);
	// draw
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
	glBindTexture(GL_TEXTURE_2D,0);
	glBindVertexArray(0);
	glUseProgram(0);
	glDisable(GL_BLEND);
}


GLuint postproc_shader_program, postproc_vao,postproc_stock_program;
GLuint noise_tex;
FBOELEM postproc_fbo;

#define NUM_POSTPROC 3
FBOELEM ntsc_fbos[NUM_POSTPROC+1];
GLuint ntsc_shaders[NUM_POSTPROC];

BOOL postproc_compiled = FALSE;
void init_postproc()
{
	//const char *fragment_source = (const char*)readShaderFile("shader4.frag");

	const char fragment_source [] = ""
		"#version 330\n"
		"in vec2 vertexuv;"
		"in vec2 vpos;"
		"uniform sampler2D tex;"
		"uniform float time;"
		"uniform vec2 resolution;"
		"uniform float snarehit;"
		"uniform float snarehit2;"
		"layout(location = 0) out vec4 FragColor;"
		"float t(vec2 t)"
		"{"
		"int v=int(t.r*40+t.g*6400);"
		"v=v<<13^v;"
		"return 1-float(v*(v*v*15731+789221)+1376312589&2147483647)/1073741824;"
		"}"
		"float s(vec2 v)"
		"{"
		"v=mod(v,1000.);"
		"vec2 e=fract(v);"
		"v-=e;"
		"vec2 f=e*e*(3.-2.*e);"
		"return mix(mix(t(v+vec2(0,0)),t(v+vec2(1,0)),f.r),mix(t(v+vec2(0,1)),t(v+vec2(1,1)),f.r),f.g);"
		"}"
		"float n(vec2 v)"
		"{"
		"float f=t(v*time);"
		"return clamp(f,.9,1.2);"
		"}"
		"float n(float v,float t,float f)"
		"{"
		"return step(f,sin(time+v*cos(time*t)));"
		"}"
		"float s(float v,float t,float f)"
		"{"
		"float r=step(t,v)-step(f,v),e=(v-t)/(f-t)*r;"
		"return(1.-e)*r;"
		"}"
		"float v(vec2 v)"
		"{"
		"float r=t(v*time*5.);"
		"return s(mod(v.g*4.+time/2.+sin(time+sin(time*.63)),1.),.5,.6)*r;"
		"}"
		"vec2 p(vec2 v)"
		"{"
		"return vec2(v.r,v.g+sin(time)*.02);"
		"}"
		"vec2 p(vec2 v,float t,float e)"
		"{"
		"return vec2(v.r,v.g+t*e);"
		"}"
		"vec3 d(vec2 v)"
		"{"
		"vec2 f=v;"
		"float r=1./(1.+20.*(f.g-mod(time/4.,1.))*(f.g-mod(time/4.,1.)));"
		"if(snarehit==1.)"
		"f.r=f.r+sin(f.g*10.+time)/30.*(1.+cos(time*80.))*r;"
		"vec3 e,u;"
		"u.r=t(v*time*4.);"
		"u.b=t(v*time*3.);"
		"u.g=t(v*time*2.);"
		"e.r=texture2D(tex,p(f,.025,u.r)).r;"
		"e.g=texture2D(tex,p(f,.01,u.g)).g;"
		"e.b=texture2D(tex,p(f,.024,u.b)).b;"
		"return e;"
		"}"
		"vec2 d(vec2 v,float f)"
		"{"
		"return v=(v-.5)*2.,v*=.5,v.r*=1.+pow(abs(v.g)/f,2.),v.g*=1.+pow(abs(v.r)/f,2.),v=v+.5,v;"
		"}"
		"vec2 f(vec2 v)"
		"{"
		"float e=clamp(cos(v.g*2.+time),0.,1.),f=clamp(cos(v.g*2.+time+4.)*10.,0.,1.),r=e*f*v.r;"
		"v.r-=.05*mix(t(v*time)*r,r,.9);"
		"return v;"
		"}"
		"void main()"
		"{"
		"vec2 e=vertexuv,r=f(e);"
		"vec3 g=d(e);"
		"float s=3.+.3*sin(time+5.*cos(time*5.)),u=(1.-s*(e.g-.5)*(e.g-.5))*(1.-s*(e.r-.5)*(e.r-.5));"
		"if(snarehit2==1.)"
		"g+=v(e);"
		"g+=t(e*time)/4.;"
		"float mod_factor = e.x * resolution.x * resolution.x / resolution.x;"
		"vec3 dotMaskWeights = mix(vec3(1.0, 0.7, 1.0),vec3(0.7, 1.0, 0.7),floor(mod(mod_factor, 2.0)));"
		"g*= dotMaskWeights;"
		"g*=u;"
		"if(snarehit2==1.)"
		"g*=(12.+mod(e.g*30.+time,1.))/13.;"
		"float frameLimit = 0.16;"
		"float frameShape = 0.44;"
		"float frameSharpness = 4.80;"
		"float interference = 0.43;"
		"const float base_brightness = 0.95;"
		"float f = (1.0 - vpos.x *vpos.x) * (1.0 - vpos.y *vpos.y);"
		"float frame = clamp(frameSharpness * (pow(f, frameShape) - frameLimit), 0.0, 1.0);"
		"g*=frame;"
		"FragColor=vec4(g,1.);"
		"}";


	const char vertex_source [] = 
		"#version 330\n"
		"layout(location = 0) in vec4 vposition;\n"
		"layout(location = 1) in vec4 vuv;\n"
		"out vec2 vertexuv;\n"
		"out vec2 vpos;\n"
		"void main() {\n"
		"   vertexuv = vuv.xy;\n"
		"   vpos = vposition.xy;\n"
		"   gl_Position =vposition;\n"
		"}\n";
	initShader(  (int*)&postproc_shader_program, vertex_source, (const char*)fragment_source);
	GLuint vbo, ibo;
	// generate and bind the vao
	glGenVertexArrays(1, &postproc_vao);
	glBindVertexArray(postproc_vao);

	// generate and bind the vertex buffer object
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);

	// data for a fullscreen quad (this time with texture coords)
	GLfloat vertexData[] = {
		//  X     Y     Z           U     V     
		1.0f, 1.0f, 0.0f, 1.0f, 1.0f, // vertex 0
		-1.0f, 1.0f, 0.0f, 0.0f, 1.0f, // vertex 1
		1.0f, -1.0f, 0.0f, 1.0f, 0.0f, // vertex 2
		-1.0f, -1.0f, 0.0f, 0.0f, 0.0f, // vertex 3
	}; // 4 vertices with 5 components (floats) each

	// fill with data
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)* 4 * 5, vertexData, GL_STATIC_DRAW);


	// set up generic attrib pointers
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (char*)0 + 0 * sizeof(GLfloat));

	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (char*)0 + 3 * sizeof(GLfloat));


	// generate and bind the index buffer object
	glGenBuffers(1, &ibo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);

	GLuint indexData[] = {
		0, 1, 2, // first triangle
		2, 1, 3, // second triangle
	};

	// fill with data
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint)* 2 * 3, indexData, GL_STATIC_DRAW);

	// "unbind" vao
	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	postproc_fbo = init_fbo(XRES, YRES);
}
float bassdrumhit = 0.0;
float snarehit = 0.0;
void draw_postproc(float scenetime, FBOELEM elem, GLuint shader)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glUseProgram(shader);
	GLint texture_location = glGetUniformLocation(shader, "tex");
	// bind texture to texture unit 0
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, elem.texture);
	// set texture uniform
	glUniform1i(texture_location, 0);
	float time = scenetime;
	GLuint time_loc = glGetUniformLocation(shader, "time");
	glUniform1f(time_loc, time);
	GLuint snare_loc = glGetUniformLocation(postproc_shader_program, "snarehit");
	glUniform1f(snare_loc, bassdrumhit);
	GLuint snare_loc2 = glGetUniformLocation(postproc_shader_program, "snarehit2");
	glUniform1f(snare_loc2, snarehit);
	GLuint resolution_loc = glGetUniformLocation(shader, "resolution");
	glUniform2f(resolution_loc, XRES, YRES);
	// bind the vao
	glBindVertexArray(postproc_vao);
	// draw
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
	glBindTexture(GL_TEXTURE_2D, 0);
	glBindVertexArray(0);
	glUseProgram(0);
}



GLuint bg_shader_program,bg_texture,bg_vao; 

void init_bg()
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
		"in vec2 vertexuv;\n"
		"layout(location = 0) out vec4 FragColor;\n"
		"uniform sampler2D tex;\n"
		"uniform float time;\n"
		"uniform vec2 resolution;\n"
		"const float f=3.14159,r=.2,t=.3,n=.3,e=3.;\n"
		"const int i=4;\n"
		"const float s=4.;\n"
		"const int v=3;\n"
		"const float c=20.,g=400.,o=.7;\n"
		"\n"
		"float m(vec2 c)"
		"{"
		"float o=2.*f/float(v),g=0.,u=0.;"
		"for(int m=0;m<i;m++)"
		"{"
		"vec2 d=c;"
		"u=o*float(m);"
		"d.r+=cos(u)*time*r+time*t;"
		"d.g-=sin(u)*time*r-time*n;"
		"g=g+cos((d.r*cos(u)-d.g*sin(u))*s)*e;"
		"}"
		"return cos(g);"
		"}"
		"\n"
		"void main()"
		"{"
		"vec2 r=gl_FragCoord.rg/resolution.rg,u=r,d=r;"
		"float f=m(u);"
		"d.r+=resolution.r/c;"
		"float t=o*(f-m(d))/c;"
		"d.r=r.r;"
		"d.g+=resolution.g/c;"
		"float i=o*(f-m(d))/c;"
		"u.r+=t;"
		"u.g=-(u.g+i);"
		"float s=1.+dot(t,i)*g;"
		"FragColor=texture2D(tex,u)*s*0.5;"
		"}";

	
	initShader(  (int*)&bg_shader_program, vertex_source, (const char*)fragment_source);

	// get texture uniform location
	GLint texture_location = glGetUniformLocation(bg_shader_program, "myTextureSampler");

	// vao and vbo handle
	GLuint vbo, ibo;

	// generate and bind the vao
	glGenVertexArrays(1, &bg_vao);
	glBindVertexArray(bg_vao);

	// generate and bind the vertex buffer object
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);

	// data for a fullscreen quad (this time with texture coords)
	GLfloat vertexData[] = {
		//  X     Y     Z           U     V     
		1.0f, 1.0f, 0.0f,       1.0f, 1.0f, // vertex 0
		-1.0f, 1.0f, 0.0f,       0.0f, 1.0f, // vertex 1
		1.0f,-1.0f, 0.0f,       1.0f, 0.0f, // vertex 2
		-1.0f,-1.0f, 0.0f,       0.0f, 0.0f, // vertex 3
	}; // 4 vertices with 5 components (floats) each

	// fill with data
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)*4*5, vertexData, GL_STATIC_DRAW);


	// set up generic attrib pointers
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5*sizeof(GLfloat), (char*)0 + 0*sizeof(GLfloat));

	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5*sizeof(GLfloat), (char*)0 + 3*sizeof(GLfloat));


	// generate and bind the index buffer object
	glGenBuffers(1, &ibo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);

	GLuint indexData[] = {
		0,1,2, // first triangle
		2,1,3, // second triangle
	};

	// fill with data
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint)*2*3, indexData, GL_STATIC_DRAW);

	// "unbind" vao
	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	// texture handle
	bg_texture = loadTexGenTexMemory(bg_tex,bg_tex_len,512,512);
	
}

void draw_bg(float scenetime){
	glViewport(0,0,XRES,YRES);
	glUseProgram(bg_shader_program);
	GLint texture_location = glGetUniformLocation(bg_shader_program, "tex");
	// bind texture to texture unit 0
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, bg_texture);
	// set texture uniform
	glUniform1i(texture_location, 0);
	float time = scenetime;
	GLuint time_loc = glGetUniformLocation(bg_shader_program, "time");
	glUniform1f(time_loc,time);
	GLuint resolution_loc = glGetUniformLocation(bg_shader_program, "resolution");
	glUniform2f(resolution_loc,XRES,YRES);
	// bind the vao
	glBindVertexArray(bg_vao);
	// draw
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
	glBindTexture(GL_TEXTURE_2D,0);
	glDisable(GL_TEXTURE_2D);
	glBindVertexArray(0);
}