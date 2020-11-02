//--------------------------------------------------------------------------//
// iq / rgba  .  tiny codes  .  2008                                        //
//--------------------------------------------------------------------------//

#define GLSL(src) #src

static const char *vsh = \
"#version 430\n"
"out gl_PerVertex"
"{"
"	vec4 gl_Position;"
"};"
"layout (location=0) in vec2 inVer;"
"void main()"
"{"
"gl_Position=vec4(inVer,0.0,1.0);"
"}";

static const char *vsh_texture = \
"#version 430\n"
"out gl_PerVertex"
"{"
"	vec4 gl_Position;"
"};"
"layout (location=0) in vec2 inVer;"
"out vec2 ftexcoord;\n"
"const vec2 madd = vec2(0.5, 0.5);"
"void main()"
"{"
"ftexcoord = inVer*madd +madd;"
"gl_Position=vec4(inVer,0.0,1.0);"
"}";

const char* texture_shader = GLSL(
\n#version 430\n
layout(location = 0) uniform vec4 fpar[4];
layout(location = 4) uniform sampler2D tex;
layout(location = 0) out vec4 out_color;
in vec2 ftexcoord;

void main(void)
{
	vec4 col = texture(tex, ftexcoord);
	float gray = dot(col.rgb, vec3(0.299, 0.587, 0.114));
	out_color = vec4(gray, gray, gray, col.a);
}
);

const char* scene1_shader = GLSL(
\n#version 430\n
layout (location=0) uniform vec4 fpar[4];
layout(location = 0) out vec4 out_color;
vec3 rotatex(in vec3 raymarch_p, float ang) { return vec3(raymarch_p.x, raymarch_p.y*cos(ang) - raymarch_p.z*sin(ang), raymarch_p.y*sin(ang) + raymarch_p.z*cos(ang)); }

vec3 rotatey(in vec3 raymarch_p, float ang) { return vec3(raymarch_p.x*cos(ang) - raymarch_p.z*sin(ang), raymarch_p.y, raymarch_p.x*sin(ang) + raymarch_p.z*cos(ang)); }

vec3 rotatez(in vec3 raymarch_p, float ang) { return vec3(raymarch_p.x*cos(ang) - raymarch_p.y*sin(ang), raymarch_p.x*sin(ang) + raymarch_p.y*cos(ang), raymarch_p.z); }

float rand(float x)
{
	vec2 co = vec2(x, x);
	float a = 12.9898;
	float b = 78.233;
	float c = 43758.5453;
	float dt = dot(co.xy, vec2(a, b));
	float sn = mod(dt, 3.14);
	return fract(sin(sn) * c);
}

float scene(vec3 raymarch_p)
{
	raymarch_p = rotatex(raymarch_p, 0.18* fpar[0].z);
	raymarch_p = rotatez(raymarch_p, 0.20*fpar[0].z);
	raymarch_p = rotatey(raymarch_p, 0.22*fpar[0].z);

	float d0 = length(max(abs(raymarch_p) - 0.5, 0.0)) - 0.01 + clamp(sin((raymarch_p.x + raymarch_p.y + raymarch_p.z)*20.0)*0.03, 0.0, 1.0);
	float d1 = length(raymarch_p) - 0.5;
	return sin(max(d0, -d1));
	//return length(p) - 0.85;   // test sphere
}

vec3 get_normal(vec3 raymarch_p)
{
	vec3 eps = vec3(0.01, 0.0, 0.0);
	float nx = scene(raymarch_p + eps.xyy) - scene(raymarch_p - eps.xyy);
	float ny = scene(raymarch_p + eps.yxy) - scene(raymarch_p - eps.yxy);
	float nz = scene(raymarch_p + eps.yyx) - scene(raymarch_p - eps.yyx);
	return normalize(vec3(nx, ny, nz));
}

void main(void)
{
	vec2 raymarch_p = 2.0 * gl_FragCoord.xy / fpar[0].xy - 1.0;
	//post effect pixel position
	vec2 posteffect_p = raymarch_p;
	//fix for raymarch ray aspect ratio
	raymarch_p.x *= fpar[0].x/fpar[0].y;

	vec3 ro = vec3(0.0, 0.0, 1.7);
	vec3 rd = normalize(vec3(raymarch_p.x, raymarch_p.y, -1.4));
	vec3 color = (1.0 - vec3(length(raymarch_p*0.5)))*0.2;

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
	//do post processing here
	float frameLimit = .65335;
	float frameShape = .4;
	float frameSharpness =3.;

	float f = (1.0 - posteffect_p.x *posteffect_p.x) * (1.0 - posteffect_p.y *posteffect_p.y);
	float frame = clamp(frameSharpness * (pow(f, frameShape) - frameLimit), 0.0, 1.0);
	color.rgb *= frame;
	out_color = vec4(color, 1.0);
}
);