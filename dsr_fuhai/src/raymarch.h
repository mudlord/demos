GLuint raymarch_vao, raymarch_texture;
shader_id raymarch_shader;
#define GLSL(src) #src

const char vertex_source[] =
"#version 330\n"
"layout(location = 0) in vec4 vposition;\n"
"layout(location = 1) in vec2 vtexcoord;\n"
"out vec2 ftexcoord;\n"
"void main() {\n"
"   ftexcoord = vtexcoord;\n"
"   gl_Position =vec4(vposition.xy, 0.0f, 1.0f);\n"
"}\n";

float cs_x = 1.;
float cs_y = 1.;
float cs_z = 1.;
float fs = 1.35;
float fc_x = 0.;
float fc_y = 0.;
float fc_z = 0.;
float fu = 0.;
float fd = 0.2;
float v_c = 1.;

float wobble = 0;


int sceneid = 0;

gbMat4 scenes[5] =
{
	//scene 1
	1., 1., 1., 0, //cs
	0., 0., 0., 1.35, //fc /fs
	0., 0.2, 1., 1., // fu/fd/v_c
	0., 0., 0., 0.,
	//scene2
	1.4, 1.4, 1.8, 0,
	2.1, 2.1, 2.1, 1.8,
	0., .1, 1.5, 1.,
	0., 0., 0., 0.,
	//scene3
	1.4, 1.4, 1.8, 0,
	2.6, 2.6, 2.6, 1.8,
	0., .1, 1.5, 1.,
	0., 0., 0., 0.,
	//last
	1.3, 1.2, 2., 0,
	2.6, 2.6, 2.6, 2.,
	0., 0.05, 1.7, 0.,
	0., 0., 0., 0.,
	//scene 4
	1.3, 1.3, 1., 0,
    1.3, .3, .3, 1.,
	0., .05, .7, 1.1,
	0., 0., 0., 0.,
};

const char scene1_shader[] = ""
"\n#version 430\n"
"layout(location = 0) out vec4 out_color;"
"layout(location = 1) uniform vec4 parameters;"
"layout(location = 2) uniform sampler2D tex;"
"layout(location = 3) uniform vec4 cs;"
"layout(location = 4) uniform vec4 fc;"
"layout(location = 5) uniform vec4 other;"
"layout(location = 6) uniform int sceneid;"
"float n(vec3 s)"
"{"
"float v=other.b;"
"for(int i=0;i<9;i++)"
"{"
"s=2.*clamp(s,-cs.rgb,cs.rgb)-s;"
"float n=max(fc.a/dot(s,s),1.);"
"s*=n;"
"v*=n;"
"s+=fc.rgb;"
"}"
"float i=length(s.rg)-other.r;"
"return other.g*max(i,floor(length(s.rg)*s.b)/dot(s,s))/abs(v);"
"}"
"vec3 n(vec3 s,vec3 i,int b)"
"{"
"float v=0.,f=0.,p=0.;"
"const float d=80.;"
"for(int e=0;e<b;++e)"
"{"
"p=n(s+i*v);"
"if(p<.001)"
"{"
"break;"
"}"
"if(v>d)"
"{"
"break;"
"}"
"v+=p;"
"f+=1.;"
"}"
"return vec3(v,f,p);"
"}"
"void main()"
"{"
"vec2 s=2.*gl_FragCoord.rg/parameters.rg-1.;"
"s.r*=parameters.r/parameters.g;"
"float v=parameters.b;"
"vec3 i;"
"if(v<10.5)"
"i=vec3(5.*cos(v*.3),2.3*sin(v*.1),.25*sin(v*.25)+.75);"
"else"
" if(v<16.)"
"i=vec3(5.*cos(v*.3),2.5*sin(v*.1),.25*sin(v*.25)+.75);"
"else"
" if(v<18.)"
"i=vec3(5.*cos(v*.1),5.*sin(v*.1),.75*sin(v*.25)+.75);"
"else"
" if(v<26.)"
"i=vec3(5.*cos(v*.1),sin(v*.1),.75*sin(v*.25)+.75);"
"else"
" if(v<40.)"
"i=vec3(6.*cos(v*.1),sin(v*.1),.75*sin(v*.25)+.75);"
"else"
" if(v<47.)"
"i=vec3(6.1*cos(v*.1),1.2*sin(v*.1),.15*sin(v*.25)+.75);"
"if(sceneid==2)"
"i=vec3(5.*cos(v*.1),5.*sin(v*.1),.25*sin(parameters.b*.25)+.75);"
"if(sceneid==3)"
"i=vec3(4.*cos(v*.1),sin(v*.1),.25*sin(v*.15)+.55);"
"if(sceneid==4)"
"i=vec3(4.*cos(v*.1),sin(v*.1),.25*sin(v*.15)+.55);"
"vec3 p=normalize(i*vec3(-1.,-1.,sin(parameters.b*.33)*2.5)),b=normalize(vec3(0.,1.,1.)),f=cross(p,b);"
"float c=.8;"
"vec3 d=normalize(f*s.r+b*s.g+p*c),r=i,m=n(r,d,80);"
"float g=m.r,a=m.g,o=g*.001;"
"vec2 e=gl_FragCoord.rg/parameters.rg;"
"float t=0.;"
"if(other.a==1)"
"t=clamp(.7+.5*abs(sin(parameters.b*3.)),.8,1.);"
"vec3 l=vec3(sin(e.r*2.+parameters.b),sin(e.g*2.-4.-parameters.b),sin(e.r*2.-4.+parameters.b));"
"float u=pow(sin(e.r*3.1416),.9)*pow(sin(e.g*3.1416),.9);"
"vec4 h=mix(vec4(vec3(u),1.),vec4(l,1.),.1);"
"h.rgb=mix(h.rgb,vec3(length(h.rgb)*.5),t);"
"vec4 k=vec4(vec3(o*h.r,o*h.g,o*h.b)*a,1.);"
"out_color=k;"
"}";


void init_raymarch()
{
	raymarch_shader = initShader( vertex_source, (const char*)scene1_shader);
	// get texture uniform location


	// vao and vbo handle
	GLuint vbo;

	// generate and bind the vao
	glGenVertexArrays(1, &raymarch_vao);
	glBindVertexArray(raymarch_vao);

	// generate and bind the vertex buffer object, to be used with VAO
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	// data for a fullscreen quad (this time with texture coords)
	// we use the texture coords for whenever a LUT is loaded
	typedef struct
	{
		float   x;
		float   y;
		float   z;
		float   u;
		float   v;
	} VBufVertex;
	VBufVertex vertexData[] = {
		//  X     Y     Z           U     V     
		1.0f, 1.0f, 0.0f, 1.0f, 1.0f, // vertex 0
		-1.0f, 1.0f, 0.0f, 0.0f, 1.0f, // vertex 1
		1.0f, -1.0f, 0.0f, 1.0f, 0.0f, // vertex 2
		1.0f, -1.0f, 0.0f, 1.0f, 0.0f, // vertex 2
		-1.0f, -1.0f, 0.0f, 0.0f, 0.0f, // vertex 3
		-1.0f, 1.0f, 0.0f, 0.0f, 1.0f, // vertex 1
	}; // 6 vertices with 5 components (floats) each
	// fill with data
	glBufferData(GL_ARRAY_BUFFER, sizeof(VBufVertex) * 6, vertexData, GL_STATIC_DRAW);
	// set up generic attrib pointers
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(VBufVertex), (void*)offsetof(VBufVertex, x));
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(VBufVertex), (void*)offsetof(VBufVertex, u));
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
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE,image);
	glBindTexture(GL_TEXTURE_2D, 0);
}

void draw_raymarch(float time, int program, int xres, int yres){
	glBindProgramPipeline(raymarch_shader.pid);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, raymarch_texture);
	glProgramUniform1i(raymarch_shader.fsid,2, 0);

	float fparams[4] = { xres,yres, time, 0.0 };
	glProgramUniform4fv(raymarch_shader.fsid, 1, 1, fparams);
/*	float cs[4] = { cs_x, cs_y, cs_z, 0. };
	glProgramUniform4fv(raymarch_shader.fsid, 3, 1, cs);
	float fc[4] = { fc_x, fc_y, fc_z, fs };
	glProgramUniform4fv(raymarch_shader.fsid, 4, 1, fc);
	float other[4] = { fu, fd, v_c, 0. };
	glProgramUniform4fv(raymarch_shader.fsid, 5, 1, other);*/
	float cs[4] = { scenes[sceneid].col[0].x, scenes[sceneid].col[0].y, scenes[sceneid].col[0].z, 0. };
	glProgramUniform4fv(raymarch_shader.fsid, 3, 1, cs);
	float fc[4] = { scenes[sceneid].col[1].x, scenes[sceneid].col[1].y, scenes[sceneid].col[1].z, scenes[sceneid].col[1].w };
	glProgramUniform4fv(raymarch_shader.fsid, 4, 1, fc);
	float other[4] = { scenes[sceneid].col[2].x, scenes[sceneid].col[2].y, scenes[sceneid].col[2].z, wobble};
	glProgramUniform4fv(raymarch_shader.fsid, 5, 1, other);
	glProgramUniform1i(raymarch_shader.fsid, 6, sceneid);
	// bind the vao
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glBindVertexArray(raymarch_vao);
	// draw
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glBindTexture(GL_TEXTURE_2D, 0);
	glBindVertexArray(0);
	glBindProgramPipeline(0);
	glDisable(GL_BLEND);
}