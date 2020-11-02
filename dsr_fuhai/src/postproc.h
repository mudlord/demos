#include "sys/msys.h"
#include "intro.h"

typedef struct
{
	float x, y, z;
	int xsize, ysize;
	shader_id program;
	GLuint vbo;
	GLuint vao;
}tv_effect;

FBOELEM postproc_fbo; //1280/640, 720 /360
tv_effect tv;
const char postpoc[] = ""
"\n#version 430\n"
"in vec2 ftexcoord;"
"layout(location = 0) out vec4 FragColor;"
"layout(location = 1) uniform sampler2D mytexture;"
"layout(location = 2) uniform vec4 time_resolution; "
"layout(location = 3) uniform vec4 frame;"
"layout(location = 4) uniform vec4 strips;"
"layout(location = 5) uniform vec4 tv_artifacts;"
"layout(location = 6) uniform int godrays;"
"layout(location = 7) uniform int sceneid;"
"float f(vec2 n)"
"{"
"float t=12.9898,r=78.233,v=43758.5,s=dot(n.rg,vec2(t,r)),f=mod(s,3.14);"
"return fract(sin(f)*v);"
"}"
"float t(vec3 n)"
"{"
"float t=12.9898,s=78.233,r=58.5065,v=43758.5,f=dot(n.rgb,vec3(t,s,r)),m=mod(f,3.14);"
"return fract(sin(m)*v);"
"}"
"float v(float t)"
"{"
"return fract(sin(mod(dot(t,12.9898),3.14))*43758.5);"
"}"
"vec4 f(vec2 t,sampler2D v)"
"{"
"float r=floor(t.g*max(1.,strips.r))/max(1.,strips.r),s=f(vec2(0.,-r)),g=f(vec2(0.,r))*strips.g;"
"vec2 m=vec2(mod(t.r+g*.25,1.),t.g+min(1.,1.+mod(t.r+g*.25,1.))-1.);"
"vec4 b=texture2D(v,m);"
"return b;"
"}"
"float f(float t,float v,float f)"
"{"
"float b=step(v,t)-step(f,t),r=(t-v)/(f-v)*b;"
"return(1.-r)*b;"
"}"
"void main()"
"{"
"vec2 n=gl_FragCoord.rg/time_resolution.gb;"
"n.g=1.-n.g;"
"vec2 s=-1.+2.*n,m=n,r=m;"
"mat3 g=mat3(.299,-.147,.615,.587,-.289,-.515,.114,.436,-.1),b=mat3(1.,1.,1.,0.,-.395,2.032,1.14,-.581,0.);"
"float i=1.;"
"i-=v(m.r*.1+m.g*50.+time_resolution.r)*.5;"
"if(tv_artifacts.g==2.)"
"{"
"r.g+=v(time_resolution.r*12.)*.004;"
"r.r+=v(m.g*39.+time_resolution.r*20.)*.01044-.012;"
"float c=sin(time_resolution.r*10.+m.g*9.),d=v(time_resolution.r*235.);"
"r.r+=c*clamp(d-.91,0.,2.)*2.5;"
"float F=sin(time_resolution.r*5.+m.g*12.),p=v(time_resolution.r*235.);"
"r.r+=F*clamp(p-.94,0.,1.)*.5;"
"float e=v(time_resolution.r*511.);"
"r.g+=clamp(e-.98,-.1,1.)*.01;"
"r.r+=v(dot(m,vec2(10.,56.))+time_resolution.r*121.)*.013;"
"}"
"vec3 c=vec3(0.);"
"float F=.3,e=-.002;"
"if(tv_artifacts.g==1.)"
"F=.7,F+=sin(time_resolution.r*2.3)*.2,F+=sin(time_resolution.r*5.52)*.1,F+=sin(time_resolution.r*23.)*.1,F+=v(m.g*59.+time_resolution.r*40.)*.4,F*=.5,e=-.005+v(m.g*45.+time_resolution.r*23.)*.003;"
"for(int d=10;d>=0;d-=1)"
"{"
"float p=float(d)/10.;"
"if(p<0.)"
"p=0.;"
"float o=p*-.05*F+e,l=p*.1*F+e;"
"vec3 a=(vec3(1.)-pow(vec3(p),vec3(.2,1.,1.)))*.2;"
"vec2 u=r+vec2(o,0.),x=r+vec2(l,0.);"
"c+=g*f(u,mytexture).rgb*a;"
"c+=g*f(x,mytexture).rgb*a;"
"}"
"c.r=c.r*.2+(g*f(r,mytexture).rgb).r*.8;"
"if(tv_artifacts.g>.5)"
"{"
"float d=v(time_resolution.r*666.);"
"if(d>.91)"
"c.gb=vec2(0.);"
"float p=0.,a=max(0.,v(time_resolution.r*440.+n.g*440.)-.995)/.015;"
"if(a>.5)"
"c.gb=vec2(0.);"
"p=(.5+sin(n.r*3.+time_resolution.r*10.)*.5)*a;"
"c.r=mix(c.r,v(time_resolution.r*11.+r.r*13.+r.g*12.),p);"
"}"
"vec4 d=vec4(0.);"
"d.rgb=b*c*i;"
"if(tv_artifacts.g>.5)"
"{"
"float p=r.g*time_resolution.g*time_resolution.g/time_resolution.g;"
"vec3 a=mix(vec3(1.,.7,1.),vec3(.7,1.,.7),floor(mod(p,2.)));"
"d.rgb*=a;"
"}"
"vec3 p=vec3(gl_FragCoord.rg,time_resolution.r);"
"float a=t(p);"
"if(godrays==1)"
"{"
"vec2 u=gl_FragCoord.rg/time_resolution.gb,o=u;"
"if(time_resolution.r<49.)"
"o=u+vec2(clamp(cos(time_resolution.r*.17),-.4,.5),clamp(sin(time_resolution.r*.09),-.3,.4))-.5;"
"if(time_resolution.r<57.)"
"o=u+vec2(clamp(cos(time_resolution.r*.1),-.3,.5),0.)-.5;"
"else"
" if(time_resolution.r<70.)"
"o=u+vec2(sin(time_resolution.r*.3),.5)-.5;"
"o*=vec2(1./float(50)*.926);"
"u+=o*a;"
"float l=1.;"
"vec4 x=texture2D(mytexture,u.rg)*.305104;"
"for(int y=0;y<50;y++)"
"{"
"u-=o;"
"vec4 D=texture2D(mytexture,u)*.305104;"
"D*=l*.58767;"
"x+=D;"
"l*=.96815;"
"}"
"d+=x*.31;"
"}"
"vec4 o=vec4(a,a,a,1.);"
"d.rgb=mix(d.rgb,o.rgb,tv_artifacts.a);"
"float u=(1.-s.r*s.r)*(1.-s.g*s.g),l=clamp(frame.b*(pow(u,frame.g)-frame.r),0.,1.);"
"d.rgb*=l;"
"d.rgb-=tv_artifacts.b;"
"if(tv_artifacts.b>.95)"
"d.rgb=vec3(0.);"
"FragColor=vec4(d.rgb,1.);"
"}";

void init_postproc()
{
	memset(&tv, 0, sizeof(tv_effect));
	postproc_fbo = init_fbo(window_xr, window_yr);
	tv.xsize = window_xr;
	tv.ysize = window_yr;
	tv.x = window_xr / 2;
	tv.y = window_yr / 2;

	const char vertex_source[] =
		"#version 430\n"
		"layout(location = 0) in vec4 v_coord;\n"
		"layout(location = 1)in vec2 v_texcoord;\n"
		"out gl_PerVertex"
		"{"
		"	vec4 gl_Position;"
		"};"
		"out vec2 ftexcoord;\n"
		"layout(location = 2)uniform mat4 mvp;\n"
		"void main() {\n"
		"   ftexcoord = v_texcoord;\n"
		"   gl_Position = mvp * v_coord;\n"
		"}\n";



	typedef struct
	{
		float   x; //vertices
		float   y;
		float   z;
		float   u; //tex coords
		float   v;
	} sprite_vbodat;


	sprite_vbodat sprite_vertices[] = {
		//X			//Y			//Z	//U  //V
		0, 0, 0, 0.0, 0.0,
		tv.xsize, 0, 0, 1.0, 0.0,
		0, tv.ysize, 0, 0.0, 1.0,
		tv.xsize, tv.ysize, 0, 1.0, 1.0,
	};
	
	glGenVertexArrays(1, &tv.vao);
	glBindVertexArray(tv.vao);

	glGenBuffers(1, &tv.vbo);
	glBindBuffer(GL_ARRAY_BUFFER,tv.vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(sprite_vbodat) * 4, sprite_vertices, GL_STATIC_DRAW);
	glBindVertexArray(0);

	tv.program = initShader(vertex_source, (const char*)postpoc);

}

float frameLimit = .35;
float frameShape = .4;
float frameSharpness = 3.;
float strp_cnt = 32.;
float strp_trnsinst = 0.10;
float tv_artifacts = 1.0;
float toblack = 0.0;
float noisemix = .065;
int godrays = 0;
void draw_postproc(int xres, int yres, float scenetime, int texture,bool flip_y)
{
	glBindProgramPipeline(tv.program.pid);

	gbMat4 projection;
	gbMat4 m_transform;
	gbMat4 mvp;
	float  tX = tv.xsize / 2.0f;
	float  tY = tv.ysize / 2.0f;
	gb_mat4_ortho2d(&projection, 0.0, (float)xres, (float)yres, 0.0);
	gb_mat4_translate(&m_transform, gb_vec3((float)tv.x - tX, (float)tv.y - tY, 0.0));
	gb_mat4_mul(&mvp, &projection, &m_transform);
	glProgramUniformMatrix4fv(tv.program.vsid, 2, 1, GL_FALSE, (float*)gb_float44_m(&mvp));
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture);
	glProgramUniform1i(tv.program.fsid, 1, 0);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	

	float time_res[4] = { scenetime, window_xr, window_yr, 0. };
	glProgramUniform4fv(tv.program.fsid, 2, 1, time_res);
	float frame_ctrls[4] = {frameLimit,frameShape,frameSharpness,0.};
	glProgramUniform4fv(tv.program.fsid, 3, 1, frame_ctrls);
	float strip_ctrls[4] = { strp_cnt, strp_trnsinst,0., 0. };
	glProgramUniform4fv(tv.program.fsid, 4, 1, strip_ctrls);
	float tv_artifacts1[4] = { flip_y, tv_artifacts, toblack, noisemix };
	glProgramUniform4fv(tv.program.fsid, 5, 1, tv_artifacts1);
	glProgramUniform1i(tv.program.fsid, 6, godrays);
	glProgramUniform1i(tv.program.fsid, 7, sceneid);


	glBindVertexArray(tv.vao);

	typedef struct
	{
		float   x; //vertices
		float   y;
		float   z;
		float   u; //tex coords
		float   v;
	} sprite_vbodat;


	sprite_vbodat sprite_vertices[] = {
		//X			//Y			//Z	//U  //V
		0, 0, 0, 0.0, 0.0,
		tv.xsize, 0, 0, 1.0, 0.0,
		0, tv.ysize, 0, 0.0, 1.0,
		tv.xsize, tv.ysize, 0, 1.0, 1.0,
	};

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glBindBuffer(GL_ARRAY_BUFFER, tv.vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(sprite_vbodat) * 4, sprite_vertices, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(sprite_vbodat), (void*)offsetof(sprite_vbodat, x));
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(sprite_vbodat), (void*)offsetof(sprite_vbodat, u));
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);
	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glDisable(GL_BLEND);
	glBindProgramPipeline(0);
}