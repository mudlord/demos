shader_id RAYMARCH

#version 430
layout(location = 0) out vec4 out_color;
layout(location = 1) uniform vec4 parameters;
layout(location = 2) uniform float time;
layout(location = 3) uniform sampler2D tex;
in vec2 ftexcoord;
uniform float crap;//-1. 30.0 1.1
float xres = parameters.x;
float yres = parameters.y;

vec3 rotatex(in vec3 p, float ang) { return vec3(p.x, p.y*cos(ang) - p.z*sin(ang), p.y*sin(ang) + p.z*cos(ang)); }
vec3 rotatey(in vec3 p, float ang) { return vec3(p.x*cos(ang) - p.z*sin(ang), p.y, p.x*sin(ang) + p.z*cos(ang)); }
vec3 rotatez(in vec3 p, float ang) { return vec3(p.x*cos(ang) - p.y*sin(ang), p.x*sin(ang) + p.y*cos(ang), p.z); }

float SuperFormula(float phi, float a, float b, float m, float n1, float n2, float n3)
{
	return pow((pow(abs(cos(m*phi/4.0)/a),n2) + pow(abs(sin(m*phi/4.0)/b), n3)), -(1.0/n1));
}

vec2 scene(vec3 p)
{
	p = rotatex(p, 0.18*time);
	p = rotatez(p, 0.20*time);
	p = rotatey(p, 0.22*time);
    float d=length(p);
	float sn=p.z/d;
	vec4 w =vec4(12.,6.,6.,16.);
	float r1=SuperFormula(atan(p.y,p.x),1.0+0.0025*sin(time),1.0,w.x,w.y,w.z,w.w);
	float r2=SuperFormula(asin(sn),1.0,1.0,w.x,w.y,w.z,w.w);
	d-=r2*sqrt(r1*r1*(1.0-sn*sn)+sn*sn);
	return vec2(d,1.0);
}
vec2 castRay( in vec3 ro, in vec3 rd )
{
    float t = 0.0;
    float m = -1.0;
    for( int i=0; i<64; i++ )
    {
	    vec2 res = scene( ro+rd*t );
      t += res.x;
	    m = res.y;
      if (res.x <= 0.001)return vec2( t, m );
    }
    return vec2( t, m );
}
vec3 get_normal(vec3 p)
{
	vec3 eps = vec3(0.11, 0.0, 0.0);
	float nx = scene(p + eps.xyy).x - scene(p - eps.xyy).x;
	float ny = scene(p + eps.yxy).x - scene(p - eps.yxy).x;
	float nz = scene(p + eps.yyx).x - scene(p - eps.yyx).x;
	return normalize(vec3(nx, ny, nz));
}

float hex(vec2 p) {
  p.x *= 0.57735*2.0;
	p.y += mod(floor(p.x), 2.0)*0.5;
	p = abs((mod(p, 1.0) - 0.5));
	return abs(max(p.x*1.5 + p.y, p.y*2.0) - 1.0);
}
vec2 noise(vec2 t)
{
t=vec2(dot(t,vec2(127.1,311.7)),dot(t,vec2(269.5,183.3)));
t=fract(sin(t)*43758.5);
return t;
}

float voronoi(vec2 v)
	{
	vec2 r=floor(v);
	vec2 e=fract(v);
	float f=8.;
	for(float u=-1.;u<=1.;u++)
	for(float m=-1.;m<=1.;m++)
	{
	vec2 g=vec2(m,u);
	vec2 d=noise(r+g);
	d=.1+.5*sin(time+4.2831*d);
	vec2 s=g+d-e;
	float o=dot(s,s);
	f=min(f,o);
	}
	return f;
}

float getBorder( in vec2 p )
{
    float dis = voronoi( p );
    
    return 1.0 - smoothstep( 0.0, .26, dis );
}

void main(void)
{
	vec2 p = 2.0 * gl_FragCoord.xy / parameters.xy - 1.0;
	p.x *= parameters.x / parameters.y;
	vec3 ro = vec3(-0.5, 0., 4.);
	vec3 rd = normalize(vec3(p, -1.4));
	vec3 pos = ro;
	float dist = 0.0;
	vec2 result_raymarch = castRay(ro,rd);
	dist = result_raymarch.x;

		float f=1-hex(p*8.);
    float voron = getBorder(1.8*p)*sin(f);
    vec3 color2=(f*voron)*vec4(43./255.,73./255.,112./255.,0.0).rgb;
	vec3 color = color2.rgb;
	if (dist < 20.)
	{
	    vec3 pos = ro + dist*rd;
		vec3 n = get_normal(pos);
		vec3 r = reflect(normalize(pos - ro), n);
		vec3 h = -normalize(n + pos - ro);
		vec3 gold = vec3(170./255.,127./255.,57./255.);
		float diff = 1.0*clamp(dot(n, normalize(vec3(1, 1, 1))), 0.0, 1.0);
		float diff2 = 0.2*clamp(dot(n, normalize(vec3(0.7, -1, 0.5))), 0.0, 1.0);
		float diff3 = 0.1*clamp(dot(n, normalize(vec3(-0.7, -0.4, 0.7))), 0.0, 1.0);
		float spec = pow(clamp(dot(h, normalize(vec3(1, 223./255., 170./255.))), 0.0, 1.0), 50.0);
		float amb = 2.5;
		color = diff*vec3(1, 1, 1) + diff2*gold + diff3*gold + spec*vec3(1, 0, 0) + amb*gold;
		color /= dist;
	}
	out_color = vec4(color, 1.0);
}


shader_id POST1

#version 430
layout(location = 0) out vec4 out_color;
layout(location = 1) uniform vec4 parameters;
layout(location = 2) uniform float time;
layout(location = 3) uniform sampler2D tex;
in vec2 ftexcoord;
float xres = parameters.x;
float yres = parameters.y;

float rand(float x)
{
    vec2 co = vec2(x,x);
    float a = 12.9898;
    float b = 78.233;
    float c = 43758.5453;
    float dt= dot(co.xy ,vec2(a,b));
    float sn= mod(dt,3.14);
    return fract(sin(sn) * c);
}

vec2 crt(vec2 coord, float bend)
{
	// put in symmetrical coords
	coord = (coord - 0.5) * 2.0;
	coord *= 0.5;	
	// deform coords
	coord.x *= 1.0 + pow((abs(coord.y) / bend), 2.0);
	coord.y *= 1.0 + pow((abs(coord.x) / bend), 2.0);
	// transform back to 0.0 - 1.0 space
	coord  = (coord / 1.0) + 0.5;
	return coord;
}

void main(void)
{
    vec2 uv = gl_FragCoord.xy / parameters.xy;
	vec2 p=crt(uv, 2.0);
    vec2 uv_q = crt(uv, 2.0);
	vec2 uv_n = uv_q;
    mat3 rgbtoyuv = mat3(0.299, -0.147,  0.615, 0.587, -0.289, -0.515, 0.114, 0.436, -0.100);
	mat3 yuvtorgb = mat3(1.000, 1.000, 1.000, 0.000, -0.395, 2.032, 1.140, -0.581, 0.000);
    float shade = 1.0;
    shade -= rand((uv.x*time) * 0.1 + (uv.y*time) * 50.0 + time) * 0.5;

    vec3 yuv = vec3(0.0);
	float fix = 0.3;
	float lumadelay = -0.002;
	for (int x = 10; x >= 0; x -= 1)
  {
    float xx = float(x) / 10.0;
    if(xx < 0.0) xx = 0.0 ;
    float x1 = (xx * -0.05)* fix + lumadelay;
    float x2 = (xx * 0.1)* fix + lumadelay;
    vec3 mult = (vec3(1.0) - pow(vec3(xx), vec3(0.2, 1.0, 1.0))) * 0.2;
    vec2 uv1 = uv_n + vec2(x1,0.0);
    vec2 uv2 = uv_n + vec2(x2,0.0);
    yuv += (rgbtoyuv * texture(tex,uv1).rgb) * mult;
	yuv += (rgbtoyuv * texture(tex,uv2).rgb) * mult;
  }
  yuv.r = yuv.r * 0.2 + (rgbtoyuv *  texture(tex,uv_n).rgb).r * 0.8;
   vec4 col = vec4(0.0);
    col.rgb = yuvtorgb * yuv * shade;
	float mod_factor = uv_n.y * parameters.y * parameters.y / parameters.y;
	vec3 dotMaskWeights = mix(vec3(1.0, 0.7, 1.0),vec3(0.7, 1.0, 0.7),floor(mod(mod_factor, 2.0)));
	col.rgb*= dotMaskWeights;
	float frameShape = 0.35;
	float frameLimit = .3;
	float frameSharpness =1.1;
	float f = (1.0 - p.x *p.x) * (1.0 - p.y *p.y);
	float frame = clamp(frameSharpness * (pow(f, frameShape) - frameLimit), 0.0, 1.0);
	col.rgb*=frame;
	out_color = vec4(col.rgb, 1.0);
}