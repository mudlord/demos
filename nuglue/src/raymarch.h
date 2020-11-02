GLuint raymarch_vao, raymarch_texture;
shader_id raymarch_shader;
#define GLSL(src) #src

const char vertex_source[] =
"#version 430\n"
"out gl_PerVertex{vec4 gl_Position;};"
"out vec2 ftexcoord;"
"void main()"
"{"
"	float x = -1.0 + float((gl_VertexID & 1) << 2);"
"	float y = -1.0 + float((gl_VertexID & 2) << 1);"
"	ftexcoord.x = (x + 1.0)*0.5;"
"	ftexcoord.y = (y + 1.0)*0.5;"
"	gl_Position = vec4(x, y, 0, 1);"
"}";



const char* scene1_shader = GLSL(
\n#version 430\n
layout(location = 0) out vec4 out_color;
layout(location = 1) uniform vec4 parameters;
layout(location = 2) uniform sampler2D tex;
in vec2 ftexcoord;
vec3 rotatex(in vec3 p, float ang) { return vec3(p.x, p.y*cos(ang) - p.z*sin(ang), p.y*sin(ang) + p.z*cos(ang)); }

vec3 rotatey(in vec3 p, float ang) { return vec3(p.x*cos(ang) - p.z*sin(ang), p.y, p.x*sin(ang) + p.z*cos(ang)); }

vec3 rotatez(in vec3 p, float ang) { return vec3(p.x*cos(ang) - p.y*sin(ang), p.x*sin(ang) + p.y*cos(ang), p.z); }

float scene(vec3 p)
{
	p = rotatex(p, 0.18*parameters.z);
	p = rotatez(p, 0.20*parameters.z);
	p = rotatey(p, 0.22*parameters.z);

	float d0 = length(max(abs(p) - 0.5, 0.0)) - 0.01 + clamp(sin((p.x +p.y + p.z)*20.0)*0.03, 0.0, 1.0);
	float d1 = length(p) - 0.5;
	return sin(max(d0, -d1));
}

vec3 get_normal(vec3 p)
{
	vec3 eps = vec3(0.01, 0.0, 0.0);
	float nx = scene(p + eps.xyy) - scene(p - eps.xyy);
	float ny = scene(p + eps.yxy) - scene(p - eps.yxy);
	float nz = scene(p + eps.yyx) - scene(p - eps.yyx);
	return normalize(vec3(nx, ny, nz));
}

void main(void)
{
	vec2 p = 2.0 * gl_FragCoord.xy / parameters.xy - 1.0;
	
	p.x *= parameters.x / parameters.y;

	vec3 ro = vec3(0.0, 0.0, 1.7);
	vec3 rd = normalize(vec3(p.x, p.y, -1.4));
	vec3 color = (1.0 - vec3(length(p*0.5)))*0.2;

	vec3 pos = ro;
	float dist = 0.0;
	for (int i = 0; i < 64; i++)
	{
		float d = scene(pos);
		pos += rd*d;
		dist += d;
	}

	if (dist < 100.0)
	{
		vec3 n = get_normal(pos);
		vec3 r = reflect(normalize(pos - ro), n);
		vec3 h = -normalize(n + pos - ro);
		float diff = 1.0*clamp(dot(n, normalize(vec3(1, 1, 1))), 0.0, 1.0);
		float diff2 = 0.2*clamp(dot(n, normalize(vec3(0.7, -1, 0.5))), 0.0, 1.0);
		float diff3 = 0.1*clamp(dot(n, normalize(vec3(-0.7, -0.4, 0.7))), 0.0, 1.0);
		//float spec = pow(clamp(dot(r, normalize(vec3(1,1,1))), 0.0, 1.0), 50.0); 
		float spec = pow(clamp(dot(h, normalize(vec3(1, 1, 1))), 0.0, 1.0), 50.0);
		float amb = 0.2 + pos.y;
		color = diff*vec3(1, 1, 1) + diff2*vec3(1, 0, 0) + diff3*vec3(1, 0, 1) + spec*vec3(1, 1, 1) + amb*vec3(0.2, 0.2, 0.2);
		color /= dist;
	}
	out_color = vec4(color, 1.0);
}
);

void init_raymarch()
{
	raymarch_shader = initShader( vertex_source, (const char*)scene1_shader);
	// generate and bind the vao
	glGenVertexArrays(1, &raymarch_vao);
}

void draw_raymarch(float time, int program, int xres, int yres){
	glBindProgramPipeline(raymarch_shader.pid);
	float fparams[4] = { xres,yres, time, 0.0 };
	glProgramUniform4fv(raymarch_shader.fsid, 1, 1, fparams);
	// bind the vao
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glBindVertexArray(raymarch_vao);
	// draw
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glBindVertexArray(0);
	glBindProgramPipeline(0);
	glDisable(GL_BLEND);
}