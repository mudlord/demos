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

const char *packed_sh =
"\n#version 430\n"
"layout(location = 0) out vec4 out_color;"
"layout(location = 1) uniform vec4 parameters;"
"layout(location = 2) uniform sampler2D tex;"
 "in vec2 ftexcoord;"
 "float v=parameters.r,f=parameters.g,r=parameters.b;"
 "vec3 n(in vec3 v,float i)"
 "{"
   "return vec3(v.r,v.g*cos(i)-v.b*sin(i),v.g*sin(i)+v.b*cos(i));"
 "}"
 "vec3 t(in vec3 v,float i)"
 "{"
   "return vec3(v.r*cos(i)-v.b*sin(i),v.g,v.r*sin(i)+v.b*cos(i));"
 "}"
 "vec3 p(in vec3 v,float i)"
 "{"
   "return vec3(v.r*cos(i)-v.g*sin(i),v.r*sin(i)+v.g*cos(i),v.b);"
 "}"
 "float n(float v,float i,float r,float g,float b,float f,float s)"
 "{"
   "return pow(pow(abs(cos(g*v/4.)/i),f)+pow(abs(sin(g*v/4.)/r),s),-(1./b));"
 "}"
 "vec2 t(vec3 v)"
 "{"
   "v=n(v,.18*parameters.b);"
   "v=p(v,.2*parameters.b);"
   "v=t(v,.22*parameters.b);"
   "float z=.0025;"
   "if(parameters.a==1.)z=.28;"
   "vec4 w =vec4(12.,6.,6.,16.);"
   "float f=length(v),g=v.b/f,d=atan(v.g,v.r),b=asin(g),c=n(d,1.+z*sin(r),1.,w.r,w.g,w.b,w.a),p=n(b,1.,1.,w.r,w.g,w.b,w.a);"
   "f-=p*sqrt(c*c*(1.-g*g)+g*g);"
   "return vec2(f,1.);"
 "}"
 "vec2 e(in vec3 v,in vec3 i)"
 "{"
   "float f=0.,r=-1.;"
   "for(int s=0;s<64;s++)"
     "{"
       "vec2 p=t(v+i*f);"
       "f+=p.r;"
       "r=p.g;"
       "if(p.r<=.001)"
         "return vec2(f,r);"
     "}"
   "return vec2(f,r);"
 "}"
 "vec3 e(vec3 v)"
 "{"
   "vec3 s=vec3(.11,0.,0.);"
   "float f=t(v+s.rgg).r-t(v-s.rgg).r,r=t(v+s.grg).r-t(v-s.grg).r,i=t(v+s.ggr).r-t(v-s.ggr).r;"
   "return normalize(vec3(f,r,i));"
 "}"
 "float w(vec2 v)"
 "{"
   "return v.r*=1.1547,v.g+=mod(floor(v.r),2.)*.5,v=abs(mod(v,1.)-.5),abs(max(v.r*1.5+v.g,v.g*2.)-1.);"
 "}"
 "vec2 m(vec2 v)"
 "{"
   "return v=vec2(dot(v,vec2(127.1,311.7)),dot(v,vec2(269.5,183.3))),v=fract(sin(v)*43758.5),v;"
 "}"
 "float d(vec2 v)"
 "{"
   "vec2 f=floor(v),s=fract(v);"
   "float i=8.;"
   "for(float g=-1.;g<=1.;g++)"
     "for(float d=-1.;d<=1.;d++)"
       "{"
         "vec2 p=vec2(d,g),c=m(f+p);"
         "c=.1+.5*sin(r+4.2831*c);"
         "vec2 b=p+c-s;"
         "float n=dot(b,b);"
         "i=min(i,n);"
       "}"
   "return i;"
 "}"
 "float a(in vec2 v)"
 "{"
   "float i=d(v);"
   "return 1.-smoothstep(0.,.26,i);"
 "}"
 "void main()"
 "{"
   "vec2 v=2.*gl_FragCoord.rg/parameters.rg-1.;"
   "v.r*=parameters.r/parameters.g;"
   "vec3 i=vec3(-.5,0.,4.),f=normalize(vec3(v,-1.4)),s=i;"
   "float r=0.;"
   "vec2 p=e(i,f);"
   "r=p.r;"
   "float g=1-w(v*8.),c=a(1.8*v)*sin(g);"

   "vec3 m=g*c*vec4(.168627,.286275,.439216,0.).rgb,d=m.rgb;"
   "if(r<20.)"
     "{"
       "vec3 b=i+r*f,n=e(b),t=reflect(normalize(b-i),n),o=-normalize(n+b-i),x=vec3(.666667,.498039,.223529);"
       "float l=clamp(dot(n,normalize(vec3(1,1,1))),0.,1.),u=.2*clamp(dot(n,normalize(vec3(.7,-1,.5))),0.,1.),z=.1*clamp(dot(n,normalize(vec3(-.7,-.4,.7))),0.,1.),h=pow(clamp(dot(o,normalize(vec3(1,.87451,.666667))),0.,1.),50.),F=2.5;"
       "d=l*vec3(1,1,1)+u*x+z*x+h*vec3(1,0,0)+F*x;"
       "d/=r;"
     "}"
   "out_color=vec4(d,1.);"
 "}";

void init_raymarch()
{
	raymarch_shader = initShader( vertex_source, (const char*)packed_sh);
	// generate and bind the vao
	glGenVertexArrays(1, &raymarch_vao);
}

void draw_raymarch(float time, int program, int xres, int yres,int sync){
	glBindProgramPipeline(raymarch_shader.pid);
	float fparams[4] = { xres,yres, time, sync };
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