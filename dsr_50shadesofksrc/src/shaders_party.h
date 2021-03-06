const char  scene1_shader[] =
"#version 330\n"
"in vec2 ftexcoord;\n"
"layout(location = 0) out vec4 FragColor;\n"
"uniform float fGlobalTime;"
"uniform vec2 v2Resolution;"
"vec2 v=v2Resolution;"
"uniform sampler2D texTex1,texTex2;"
"uniform float camx,camy,camz,sc1_mengertweak1,sc1_mengertweak2,sc1_mengertweak3;"
"float i=fGlobalTime,c=3.14153;\n"
"#define INF 100000.0\n"
"float t(vec3 v,vec3 g)"
"{"
"vec3 i=abs(v)-g;"
"return min(max(i.r,max(i.g,i.b)),0.)+length(max(i,0.));"
"}"
"float t(in vec3 v)"
"{"
"float i=t(v.rgb,vec3(INF,1.,1.)),c=t(v.gbr,vec3(1.,INF,1.)),g=t(v.brg,vec3(1.,1.,INF));"
"return min(i,min(c,g));"
"}"
"float p(vec3 v)"
"{"
"float i=t(v,vec3(1.)),f=1.;"
"for(int r=0;r<6;r++)"
"{"
"vec3 c=mod(v*f,sc1_mengertweak3)-sc1_mengertweak1;"
"f*=sc1_mengertweak2;"
"vec3 g=1.-2.*c;"
"float n=t(g)/f;"
"i=max(i,-n);"
"}"
"return i;"
"}"
"float p(vec3 v,float g)"
"{"
"return p(v/g)*g;"
"}"
"vec2 n(in vec3 v)"
"{"
"float i=p(v-vec3(0.,.5,0.),1.4);"
"vec2 g=vec2(i,2.);"
"return g;"
"}"
"vec2 n(in vec3 v,in vec3 g)"
"{"
"float i=0.,f=1.;"










"for(int r=0;r<65;r++)"








"{"
"vec2 c=n(v+g*i);"
"i+=c.r;"
"f=c.g;"
"}"
"return vec2(i,f);"
"}"
"float n(in vec3 v,in vec3 g,in float i,in float f)"
"{"
"float r=1.,c=i;"
"for(int m=0;m<8;m++)"
"{"
"float d=n(v+g*c).r;"
"r=min(r,8.*d/c);"
"c+=clamp(d,.02,.1);"
"if(d<.001||c>f)"
"break;"
"}"
"return clamp(r,0.,1.);"
"}"
"vec3 e(in vec3 v)"
"{"
"vec3 i=vec3(.001,0.,0.),c=vec3(n(v+i.rgg).r-n(v-i.rgg).r,n(v+i.grg).r-n(v-i.grg).r,n(v+i.ggr).r-n(v-i.ggr).r);"
"return normalize(c);"
"}"
"float e(in vec3 v,in vec3 g)"
"{"
"float i=0.,f=1.;"
"for(int r=0;r<5;r++)"
"{"
"float c=.01+.12*float(r)/4.;"
"vec3 d=g*c+v;"
"float m=n(d).r;"
"i+=-(m-c)*f;"
"f*=.95;"
"}"
"return clamp(1.-3.*i,0.,1.);"
"}"
"vec2 f(vec2 v)"
"{"
"return v=vec2(dot(v,vec2(127.1,311.7)),dot(v,vec2(269.5,183.3))),fract(sin(v)*43758.5);"
"}"
"float r(vec2 v)"
"{"
"vec2 c=floor(v),n=fract(v);"
"float r=8.;"
"for(float g=-1.;g<=1.;g++)"
"for(float m=-1.;m<=1.;m++)"
"{"
"vec2 t=vec2(m,g),d=f(c+t);"
"d=.1+.5*sin(i+4.2831*d);"
"vec2 s=t+d-n;"
"float I=dot(s,s);"
"r=min(r,I);"
"}"
"return r;"
"}"
"float m(in vec2 v)"
"{"
"float g=r(v);"
"return 1.-smoothstep(0.,.75,g);"
"}"
"vec3 f(in vec3 v,in vec3 i)"
"{"
"vec3 f=vec3(0.,0.,0.);"
"vec2 c=n(v,i);"
"float g=c.r,d=c.g;"
"if(d>-.5)"
"{"
"vec3 r=v+g*i,s=e(r),I=reflect(i,s);"
"float t=clamp(dot(s,normalize(vec3(1,1,1))),0.,1.),l=.2*clamp(dot(s,normalize(vec3(.7,-1,.5))),0.,1.),p=.1*clamp(dot(s,normalize(vec3(-.7,-.4,.7))),0.,1.),o=pow(clamp(dot(I,normalize(vec3(1,1,1))),0.,1.),50.);"
"vec3 b=texture(texTex1,I.rg).rgb,a=texture(texTex2,I.rg*2.).rgb;"
"float u=m(16.*I.rg);"
"vec4 x=(u+1.)*vec4(.1,.3,.5,0.);"
"if(d==2.)"
"f=t*vec3(.1,.3,.5)+l*vec3(.1,.3,.5)+b+a+p*vec3(.1,.3,.5)*x.rgb+o*vec3(1,1,1)+x.rgb,f/=c.r*.44;"
"float k=e(r,s);"
"vec3 z=normalize(vec3(-.6,.7,-.5));"
"float w=clamp(.5+.5*I.g,0.,1.),T=clamp(dot(I,normalize(vec3(-z.r,0.,-z.b))),0.,1.)*clamp(1.-r.g,0.,1.),F=smoothstep(-.1,.1,I.g),N=pow(clamp(1.+dot(I,i),0.,1.),2.);"
"vec3 h=vec3(0.);"
"h+=.3*w*vec3(.5,.7,1.)*k;"
"h+=.4*F*vec3(.5,.7,1.)*k;"
"h+=.3*T*vec3(.25,.25,.25)*k;"
"h+=.4*N*vec3(1.,1.,1.)*k;"
"h+=.02;"
"f=f*h;"
"}"
"f*=c.r*.3;"
"return vec3(clamp(f,0.,1.));"
"}"
"mat3 e(in vec3 v,in vec3 i,float g)"
"{"
"vec3 c=normalize(i-v),f=vec3(sin(g),cos(g),0.),r=normalize(cross(c,f)),s=normalize(cross(r,c));"
"return mat3(r,s,c);"
"}"
"void main()"
"{"
"vec2 r=2.*gl_FragCoord.rg/v-1.;"
"r.r*=v.r/v.g;"
"float c=15.+i;"
"vec3 g=vec3(camx,camy,camz),s=vec3(-.5,-.2,.3);"
"mat3 m=e(g,s,0.);"
"vec3 n=m*normalize(vec3(r.rg,2.5)),d=f(g,n);"
"FragColor=vec4(d,1.);"
"}";


const char  scene3_shader[] =
"#version 330 core\n"
"layout(location = 0) out vec4 FragColor;\n"
"uniform float fGlobalTime;"
"uniform vec2 v2Resolution;"
"vec2 v=v2Resolution;"
"uniform sampler2D texTex1,texTex2;"
"uniform float sc3_rotatex,sc3_rotatey,sc3_rotatez,sc3_tweak1,sc3_tweak2,sc3_tweak3,sc3_tweak4,sc3_tweak5,sc3_mengerscale,sc3_lightscale,sc3_fogscale;"
"float g=fGlobalTime,s=3.14153;"
"vec3 n(in vec3 v,float g)"
"{"
"return vec3(v.r,v.g*cos(g)-v.b*sin(g),v.g*sin(g)+v.b*cos(g));"
"}"
"vec3 t(in vec3 v,float g)"
"{"
"return vec3(v.r*cos(g)-v.b*sin(g),v.g,v.r*sin(g)+v.b*cos(g));"
"}"
"vec3 p(in vec3 v,float g)"
"{"
"return vec3(v.r*cos(g)-v.g*sin(g),v.r*sin(g)+v.g*cos(g),v.b);"
"}\n"
"#define INF 100000.0\n"
"float e(float g,float v)"
"{"
"return max(-v,g);"
"}"
"vec2 x(vec2 v,vec2 g)"
"{"
"return v.r<g.r?v:g;"
"}"
"vec3 w(vec3 v,vec3 g)"
"{"
"return mod(v,g)-.5*g;"
"}"
"vec3 e(vec3 v)"
"{"
"float g=cos(10.*v.g+10.),s=sin(10.*v.g+10.);"
"mat2 m=mat2(g,-s,s,g);"
"return vec3(m*v.rb,v.g);"
"}"
"float n(vec3 v)"
"{"
"return v.g;"
"}"
"float m(vec3 g,vec3 v)"
"{"
"vec3 s=abs(g)-v;"
"return min(max(s.r,max(s.g,s.b)),0.)+length(max(s,0.));"
"}"
"float m(in vec3 v)"
"{"
"float g=m(v.rgb,vec3(INF,1.,1.)),s=m(v.gbr,vec3(1.,INF,1.)),e=m(v.brg,vec3(1.,1.,INF));"
"return min(g,min(s,e));"
"}"
"float p(vec3 g)"
"{"
"float v=m(g,vec3(1.)),s=sc3_tweak1;"
"for(int f=0;f<4;f++)"
"{"
"vec3 c=mod(g*s,2.)-.5;"
"s*=sc3_tweak2;"
"vec3 e=sc3_tweak3-sc3_tweak4*c;"
"float r=m(e)/s;"
"v=max(v,-r);"
"}"
"return v;"
"}"
"float f(vec3 v,float g)"
"{"
"return p(v/g)*g;"
"}"
"float f(vec3 v)"
"{"
"if(sc3_rotatex>0.)"
"v=n(v,sc3_rotatex*g-3.);"
"if(sc3_rotatez>0.)"
"v=p(v,sc3_rotatez*g-2.);"
"if(sc3_rotatey>0.)"
"v=t(v,sc3_rotatey*g-2.);"
"if(sc3_tweak5>0.)"
"v.b-=.14*g;"
"vec3 s=mod(v,2.)-1.;"
"s=abs(s);"
"float m=f(s-vec3(0.,-.7,0.),sc3_mengerscale);"
"vec2 c=vec2(m,2.);"
"return c.r;"
"}"
"vec3 t(vec3 g)"
"{"
"vec3 v=vec3(.001,0.,0.);"
"float s=f(g+v.rgg)-f(g-v.rgg),e=f(g+v.grg)-f(g-v.grg),d=f(g+v.ggr)-f(g-v.ggr);"
"return normalize(vec3(s,e,d));"
"}"
"float c(in vec3 v,in vec3 g)"
"{"
"float s=0.,r=1.;"
"for(int m=0;m<5;m++)"
"{"
"float c=.01+.12*float(m)/4.;"
"vec3 e=g*c+v;"
"float i=f(e);"
"s+=-(i-c)*r;"
"r*=.95;"
"}"
"return clamp(1.-3.*s,0.,1.);"
"}"
"vec2 c(vec2 g)"
"{"
"return g=vec2(dot(g,vec2(127.1,311.7)),dot(g,vec2(269.5,183.3))),fract(sin(g)*43758.5);"
"}"
"float w(vec2 v)"
"{"
"vec2 s=floor(v),e=fract(v);"
"float f=8.;"
"for(float r=-1.;r<=1.;r++)"
"for(float m=-1.;m<=1.;m++)"
"{"
"vec2 n=vec2(m,r),d=c(s+n);"
"d=.1+.5*sin(g+4.2831*d);"
"vec2 i=n+d-e;"
"float b=dot(i,i);"
"f=min(f,b);"
"}"
"return f;"
"}"
"float x(in vec2 g)"
"{"
"float v=w(g);"
"return 1.-smoothstep(0.,.75,v);"
"}"
"vec3 r(vec2 v)"
"{"
"float c=3.,r=0.;"
"vec2 f=v*c-c/2.;"
"r+=sin(f.r+g);"
"r+=sin((f.g+g)/2.);"
"r+=sin((f.r+f.g+g)/4.);"
"f+=c/2.*vec2(sin(g/3.),cos(g/2.));"
"r+=sin(sqrt(f.r*f.r+f.g*f.g+1.)+g);"
"r=r*2.;"
"float m=sin(s*r),e=sin(s*r+2.*s/4.),d=sin(s*r+4.*s/3.);"
"vec3 i=vec3(m,e,d);"
"return vec3(i*.5+.5);"
"}"
"void main()"
"{"
"vec2 s=2.*gl_FragCoord.rg/v-1.;"
"s.r*=v.r/v.g;"
"vec2 e=gl_FragCoord.rg/v.rg;"
"vec3 m=vec3(0.,0.,1.),d=normalize(vec3(s.r,s.g,-1.6)),i=vec3(0.),b=m;"
"float n=0.;"
"for(int u=0;u<85;u++)"
"{"
"float p=f(b);"
"b+=d*p;"
"n+=p;"
"}"
"vec3 p=t(b),I=reflect(normalize(b-m),p);"
"float a=clamp(dot(p,normalize(vec3(1,1,1))),0.,1.),l=.2*clamp(dot(p,normalize(vec3(.7,-1,.5))),0.,1.),u=pow(clamp(dot(I,normalize(vec3(1,1,1))),0.,1.),50.);"
"vec3 F=normalize(vec3(sin(g-.5)*.5,sin(g-.5)*.5,-1)),z=normalize(vec3(sin(g-.5)*.5,-1,sin(g-.5)*.5)),y=normalize(vec3(-1,sin(g-.5)*.5,sin(g-.5)*.5)),w=normalize(vec3(sin(g-.5)*.5,sin(g-.5)*.5,sin(g-.5)*.5)),o=normalize(vec3(sin(g-.5)*.5,sin(g-.7)*.3,sin(g-.5)*.5));"
"float k=pow(max(.5,dot(reflect(F,p),d)),sc3_lightscale),h=pow(max(.5,dot(reflect(z,p),d)),sc3_lightscale),T=pow(max(.5,dot(reflect(y,p),d)),sc3_lightscale),N=pow(max(.5,dot(reflect(w,p),d)),sc3_lightscale),C=pow(max(.5,dot(reflect(w,p),d)),sc3_lightscale);"
"vec3 R=texture(texTex1,I.rg*.5).rgb,G=texture(texTex2,I.rg).rgb;"
"float D=x(16.*e);"
"vec4 q=(D+1.)*vec4(.1,.3,.5,0.);"
"i=a*vec3(.1,.3,.5)+l*vec3(.1,.3,.5)+R+G*q.rgb*r(I.rg)+T*r(I.rg)+h*vec3(1,1,0)+u*r(I.rg)*1.5+k*r(I.rg)+N*r(I.rg)+C*r(I.rg);"
"float Z=c(b,p);"
"vec3 Y=normalize(vec3(-.6,.7,-.5));"
"float X=clamp(.5+.5*I.g,0.,1.),W=clamp(dot(I,normalize(vec3(-Y.r,0.,-Y.b))),0.,1.)*clamp(1.-b.g,0.,1.),V=smoothstep(-.1,.1,I.g),U=pow(clamp(1.+dot(I,d),0.,1.),2.);"
"vec3 S=vec3(0.);"
"S+=.3*X*vec3(.5,.7,1.)*Z;"
"S+=.4*V*vec3(.5,.7,1.)*Z;"
"S+=.3*W*vec3(.25,.25,.25)*Z;"
"S+=.4*U*vec3(1.,1.,1.)*Z;"
"S+=.12;"
"i=i*S;"
"float Q=clamp(pow(sc3_fogscale/length(b-m),2.),0.,1.);"
"vec3 P=normalize(vec3(1.,.9,.3));"
"float O=dot(d,P),M=pow(max(O,0.),4.1);"
"vec3 L=mix(vec3(.1,.3,.5),vec3(.1,.3,.5),M);"
"i.rgb=mix(L,i.rgb,Q);"
"FragColor=vec4(i,1.);"
"}";






const char  scene4_shader[] =
"#version 330 core\n"
"layout(location = 0) out vec4 FragColor;\n"
"uniform float fGlobalTime;"
"uniform vec2 v2Resolution;"
"vec2 v=v2Resolution;"
"uniform sampler2D texTex1,texTex2;"
"float g=fGlobalTime,i=3.14153;\n"
"#define INF 100000.0\n"
"float f=-.7,r=.15,n=1.,s=.33;"
"uniform float sc3_lightscale;"
"vec3 p(in vec3 v,float g)"
"{"
"return vec3(v.r,v.g*cos(g)-v.b*sin(g),v.g*sin(g)+v.b*cos(g));"
"}"
"vec3 t(in vec3 v,float g)"
"{"
"return vec3(v.r*cos(g)-v.b*sin(g),v.g,v.r*sin(g)+v.b*cos(g));"
"}"
"vec3 e(in vec3 v,float g)"
"{"
"return vec3(v.r*cos(g)-v.g*sin(g),v.r*sin(g)+v.g*cos(g),v.b);"
"}"
"float m(float v,float g)"
"{"
"return max(-g,v);"
"}"
"vec2 x(vec2 v,vec2 g)"
"{"
"return v.r<g.r?v:g;"
"}"
"vec3 h(vec3 v,vec3 g)"
"{"
"return mod(v,g)-.5*g;"
"}"
"vec3 e(vec3 v)"
"{"
"float g=cos(10.*v.g+10.),r=sin(10.*v.g+10.);"
"mat2 i=mat2(g,-r,r,g);"
"return vec3(i*v.rb,v.g);"
"}"
"float h(vec3 v)"
"{"
"return v.g;"
"}"
"float c(vec3 v,vec3 g)"
"{"
"vec3 i=abs(v)-g;"
"return min(max(i.r,max(i.g,i.b)),0.)+length(max(i,0.));"
"}"
"float c(in vec3 v)"
"{"
"float g=c(v.rgb,vec3(INF,1.,1.)),i=c(v.gbr,vec3(1.,INF,1.)),f=c(v.brg,vec3(1.,1.,INF));"
"return min(g,min(i,f));"
"}"
"float m(vec3 v)"
"{"
"float i=c(v,vec3(1.)),r=1.;"
"for(float f=0;f<5;f++)"
"{"
"vec3 m=mod(v*r,2)-s;"
"r*=2.;"
"vec3 b=1.-1.5*m;"
"b+=sin(g)*.47;"
"float e=c(b)/r;"
"i=max(i,-e);"
"}"
"return i;"
"}"
"float w(vec3 v,float g)"
"{"
"return m(v/g)*g;"
"}"
"float d(vec3 v,vec3 g)"
"{"
"vec3 i=abs(v)-g;"
"return min(max(i.r,max(i.g,i.b)),0.)+length(max(i,0.))+clamp(sin((v.r+v.g+v.b)*6.)*.02,0.,1.);"
"}"
"float d(in vec3 v)"
"{"
"float g=d(v.rgb,vec3(INF,1.,1.)),i=d(v.gbr,vec3(1.,INF,1.)),f=d(v.brg,vec3(1.,1.,INF));"
"return min(g,min(i,f));"
"}"
"float l(vec3 v,float g)"
"{"
"return d(v/g)*g;"
"}"
"vec2 l(in vec3 v)"
"{"
"vec3 i=mod(v,2.)-1.,b=v;"
"float s=length(max(abs(b-vec3(0.,r,2.))-vec3(.25),0.))-.01+clamp(sin((b.r+b.g+b.b)*23.)*.02,0.,1.),d=length(max(abs(b)-.5,0.))-.01+clamp(sin((b.r+b.g+b.b)*20.)*.03,0.,1.),n=length(b-vec3(0.,.5,0.))-.6,c=m(d,n);"
"b=p(b,.6*g-2.);"
"b=e(b,.4*g-2.);"
"b=t(b,.22*g);"
"float I=length(max(abs(b)-.6,0.))-.01+clamp(sin((b.r+b.g+b.g)*10.)*.03,0.,1.),F=length(b)-.4,a=m(F,I),o=c<a?c:a,u=w(i-vec3(0.,f,0.),1.),z=l(i-vec3(0.,f,0.),.1);"
"vec2 k=x(vec2(h(v),1.),vec2(u,2.));"
"k=x(k,vec2(o,2.));"
"k=x(k,vec2(s,1.));"
"return k;"
"}"
"vec2 a(in vec3 v,in vec3 g)"
"{"
"float i=0.,f=-1.;"
"for(int r=0;r<50;r++)"
"{"
"vec2 m=l(v+g*i);"
"i+=m.r;"
"f=m.g;"
"if(m.r<=.001)"
"return vec2(i,f);"
"}"
"return vec2(i,f);"
"}"
"float a(in vec3 v,in vec3 g,in float m,in float b)"
"{"
"float i=1.,f=m;"
"for(int r=0;r<8;r++)"
"{"
"float s=l(v+g*f).r;"
"i=min(i,8.*s/f);"
"f+=clamp(s,.02,.1);"
"if(s<.001||f>b)"
"break;"
"}"
"return clamp(i,0.,1.);"
"}"
"vec3 a(in vec3 v)"
"{"
"vec3 g=vec3(.01,0.,0.),i=vec3(l(v+g.rgg).r-l(v-g.rgg).r,l(v+g.grg).r-l(v-g.grg).r,l(v+g.ggr).r-l(v-g.ggr).r);"
"return normalize(i);"
"}"
"float I(in vec3 v,in vec3 g)"
"{"
"float r=0.,f=1.;"
"for(int i=0;i<5;i++)"
"{"
"float b=.01+.12*float(i)/4.;"
"vec3 m=g*b+v;"
"float s=l(m).r;"
"r+=-(s-b)*f;"
"f*=.95;"
"}"
"return clamp(1.-3.*r,0.,1.);"
"}"
"vec2 I(vec2 v)"
"{"
"return v=vec2(dot(v,vec2(127.1,311.7)),dot(v,vec2(269.5,183.3))),fract(sin(v)*43758.5);"
"}"
"float p(vec2 v)"
"{"
"vec2 i=floor(v),f=fract(v);"
"float r=8.;"
"for(float b=-1.;b<=1.;b++)"
"for(float m=-1.;m<=1.;m++)"
"{"
"vec2 c=vec2(m,b),s=I(i+c);"
"s=.1+.5*sin(g+4.2831*s);"
"vec2 d=c+s-f;"
"float n=dot(d,d);"
"r=min(r,n);"
"}"
"return r;"
"}"
"float t(in vec2 v)"
"{"
"float g=p(v);"
"return 1.-smoothstep(0.,.75,g);"
"}"
"vec3 w(vec2 v)"
"{"
"float s=3.,f=0.;"
"vec2 b=v*s-s/2.;"
"f+=sin(b.r+g);"
"f+=sin((b.g+g)/2.);"
"f+=sin((b.r+b.g+g)/4.);"
"b+=s/2.*vec2(sin(g/3.),cos(g/2.));"
"f+=sin(sqrt(b.r*b.r+b.g*b.g+1.)+g);"
"f=f*2.;"
"float m=sin(i*f),r=sin(i*f+2.*i/4.),n=sin(i*f+4.*i/3.);"
"vec3 d=vec3(m,r,n);"
"return vec3(d*.5+.5);"
"}"
"vec3 o(in vec3 v,in vec3 i)"
"{"
"vec3 f=vec3(0.,0.,0.);"
"vec2 m=a(v,i);"
"float b=m.r,s=m.g;"
"if(s>-.5)"
"{"
"vec3 r=v+b*i,d=a(r),c=reflect(i,d);"
"float n=clamp(dot(d,normalize(vec3(1,1,1))),0.,1.),p=.2*clamp(dot(d,normalize(vec3(.7,-1,.5))),0.,1.),k=.1*clamp(dot(d,normalize(vec3(-.7,-.4,.7))),0.,1.),l=pow(clamp(dot(c,normalize(vec3(1,1,1))),0.,1.),50.);"
"vec3 F=normalize(vec3(sin(g-.5)*.7,sin(g-.5)*.5,1)),o=normalize(vec3(-1,-1,1)),u=normalize(vec3(-1,-1,1));"
"float e=pow(max(.5,dot(reflect(F,d),i)),sc3_lightscale),z=pow(max(.5,dot(reflect(o,d),i)),sc3_lightscale),x=pow(max(.5,dot(reflect(u,d),i)),sc3_lightscale);"
"vec3 h=x*w(i.rg)+z*w(c.rg)+l*w(c.rg)+e*w(r.rg),N=texture(texTex1,c.rg).rgb,T=texture(texTex2,c.rg*2.).rgb;"
"float R=t(16.*c.rg);"
"vec4 G=(R+1.)*vec4(.1,.3,.5,0.);"
"if(s==2.)"
"f=n*vec3(.1,.3,.5)+p*vec3(.1,.3,.5)+N+T+k*vec3(.1,.3,.5)*G.rgb+h,f*=m.r;"
"if(s==1.)"
"{"
"float C=mod(floor(5.*r.b)+floor(5.*r.r),2.);"
"f=p+N+T+C+k*vec3(.1,.3,.5)*G.rgb+l*vec3(1,1,1)+G.rgb;"
"f/=m.r*.54;"
"}"
"float C=I(r,d);"
"vec3 D=normalize(vec3(-.6,.7,-.5));"
"float q=clamp(.5+.5*c.g,0.,1.),Z=clamp(dot(c,D),0.,1.),Y=clamp(dot(c,normalize(vec3(-D.r,0.,-D.b))),0.,1.)*clamp(1.-r.g,0.,1.),X=smoothstep(-.1,.1,c.g),W=pow(clamp(1.+dot(c,i),0.,1.),2.);"
"Z*=a(r,D,.02,2.5);"
"X*=a(r,c,.02,2.5);"
"vec3 V=vec3(0.);"
"V+=.3*q*w(c.rg)*C;"
"V+=.4*X*vec3(.5,.7,1.)*C;"
"V+=.3*Y*vec3(.25,.25,.25)*C;"
"V+=.4*W*vec3(1.,1.,1.)*C;"
"V+=.02;"
"f=f*V;"
"float U=clamp(pow(2.4/length(r-v),2.),0.,1.);"
"vec3 S=normalize(vec3(1.,.9,.3));"
"float Q=dot(i,S),P=pow(max(Q,.9),1.1);"
"vec3 O=mix(vec3(.1,.3,.5),vec3(.1,.3,.5),P);"
"f.rgb=mix(O,f.rgb,U);"
"}"
"f/=m.r*.7;"
"return vec3(clamp(f,0.,1.));"
"}"
"mat3 I(in vec3 v,in vec3 i,float g)"
"{"
"vec3 f=normalize(i-v),b=vec3(sin(g),cos(g),0.),r=normalize(cross(f,b)),d=normalize(cross(r,f));"
"return mat3(r,d,f);"
"}"
"void main()"
"{"
"vec2 i=2.*gl_FragCoord.rg/v-1.;"
"i.r*=v.r/v.g;"
"float f=15.+g;"
"vec3 r=vec3(-.25+3.2*cos(.1*g+6.),1.5,.5+2.2*sin(.1*g+6.)),d=vec3(-.5,-.4,.5);"
"mat3 m=I(r,d,0.);"
"vec3 b=m*normalize(vec3(i.rg,2.5)),s=o(r,b);"
"s=pow(s,vec3(1.4545));"
"FragColor=vec4(s,1.);"
"}";

const char scene5_shader[] = ""
"#version 330 core\n"
"layout(location = 0) out vec4 FragColor;\n"
"uniform float fGlobalTime;"
"uniform vec2 v2Resolution;"
"vec2 v=v2Resolution;"
"uniform sampler2D texTex1,texTex2;"
"float g=fGlobalTime,i=3.14153;\n"
"#define INF 100000.0\n"
"uniform float sc3_lightscale;"
"float n(float v,float g)"
"{"
"return max(-g,v);"
"}"
"vec2 t(vec2 v,vec2 g)"
"{"
"return v.r<g.r?v:g;"
"}"
"vec3 p(vec3 v,vec3 g)"
"{"
"return mod(v,g)-.5*g;"
"}"
"vec3 n(vec3 v)"
"{"
"float g=cos(10.*v.g+10.),i=sin(10.*v.g+10.);"
"mat2 m=mat2(g,-i,i,g);"
"return vec3(m*v.rb,v.g);"
"}"
"float p(vec3 v)"
"{"
"return v.g;"
"}"
"float s(vec3 v,vec3 g)"
"{"
"vec3 i=abs(v)-g;"
"return min(max(i.r,max(i.g,i.b)),0.)+length(max(i,0.));"
"}"
"float s(in vec3 v)"
"{"
"float i=s(v.rgb,vec3(INF,1.,1.)),g=s(v.gbr,vec3(1.,INF,1.)),b=s(v.brg,vec3(1.,1.,INF));"
"return min(i,min(g,b));"
"}"
"float t(vec3 v)"
"{"
"float i=s(v,vec3(1.)),g=2.;"
"for(int f=0;f<3;f++)"
"{"
"vec3 m=mod(v*g,2.)-.35;"
"g*=4.6;"
"vec3 c=1.-2.*m;"
"float b=s(c)/g;"
"i=max(i,-b);"
"}"
"return i;"
"}"
"float e(vec3 v,float g)"
"{"
"return t(v/g)*g;"
"}"
"float m(vec3 v,vec3 g)"
"{"
"vec3 i=abs(v)-g;"
"return min(max(i.r,max(i.g,i.b)),0.)+length(max(i,.1))+clamp(sin(v.b*4.*3.)*.04,0.,1.);"
"}"
"float e(in vec3 v)"
"{"
"float i=m(v.rgb,vec3(INF,1.,1.)),g=m(v.gbr,vec3(1.,INF,1.)),b=m(v.brg,vec3(1.,1.,INF));"
"return min(i,min(g,b));"
"}"
"float x(vec3 v,float g)"
"{"
"return e(v/g)*g;"
"}"
"vec2 m(in vec3 v)"
"{"
"float g=-.7;"
"vec3 i=mod(v,2.)-1.,b=abs(v);"
"float m=length(max(abs(i-vec3(0.,-.7,0.))-vec3(.25),0.))-.02+clamp(sin((i.r+i.g+i.b)*23.)*.02,0.,1.),s=e(i-vec3(0.,-.7,0.),.4),r=x(i-vec3(0.,-.7,0.),.1);"
"vec2 f=t(vec2(p(v),1.),vec2(r,2.));"
"f=t(f,vec2(s,2.));"
"return f;"
"}"
"vec2 f(in vec3 v,in vec3 g)"
"{"
"float i=0.,f=-1.;"
"for(int r=0;r<50;r++)"
"{"
"vec2 s=m(v+g*i);"
"i+=s.r;"
"f=s.g;"
"}"
"return vec2(i,f);"
"}"
"float e(in vec3 v,in vec3 g,in float s,in float f)"
"{"
"float i=1.,r=s;"
"for(int b=0;b<8;b++)"
"{"
"float c=m(v+g*r).r;"
"i=min(i,8.*c/r);"
"r+=clamp(c,.02,.1);"
"if(c<.001||r>f)"
"break;"
"}"
"return clamp(i,0.,1.);"
"}"
"vec3 f(in vec3 v)"
"{"
"vec3 i=vec3(.001,0.,0.),g=vec3(m(v+i.rgg).r-m(v-i.rgg).r,m(v+i.grg).r-m(v-i.grg).r,m(v+i.ggr).r-m(v-i.ggr).r);"
"return normalize(g);"
"}"
"float c(in vec3 v,in vec3 g)"
"{"
"float i=0.,f=1.;"
"for(int r=0;r<5;r++)"
"{"
"float b=.01+.12*float(r)/4.;"
"vec3 c=g*b+v;"
"float s=m(c).r;"
"i+=-(s-b)*f;"
"f*=.95;"
"}"
"return clamp(1.-3.*i,0.,1.);"
"}"
"vec2 c(vec2 v)"
"{"
"return v=vec2(dot(v,vec2(127.1,311.7)),dot(v,vec2(269.5,183.3))),fract(sin(v)*43758.5);"
"}"
"float x(vec2 v)"
"{"
"vec2 i=floor(v),f=fract(v);"
"float r=8.;"
"for(float b=-1.;b<=1.;b++)"
"for(float m=-1.;m<=1.;m++)"
"{"
"vec2 s=vec2(m,b),n=c(i+s);"
"n=.1+.5*sin(g+4.2831*n);"
"vec2 d=s+n-f;"
"float e=dot(d,d);"
"r=min(r,e);"
"}"
"return r;"
"}"
"float r(in vec2 v)"
"{"
"float g=x(v);"
"return 1.-smoothstep(0.,.75,g);"
"}"
"vec3 w(vec2 v)"
"{"
"float s=3.,f=0.;"
"vec2 r=v*s-s/2.;"
"f+=sin(r.r+g);"
"f+=sin((r.g+g)/2.);"
"f+=sin((r.r+r.g+g)/4.);"
"r+=s/2.*vec2(sin(g/3.),cos(g/2.));"
"f+=sin(sqrt(r.r*r.r+r.g*r.g+1.)+g);"
"f=f*2.;"
"float m=sin(i*f),b=sin(i*f+2.*i/4.),n=sin(i*f+4.*i/3.);"
"vec3 c=vec3(m,b,n);"
"return vec3(c*.5+.5);"
"}"
"vec3 r(in vec3 v,in vec3 i)"
"{"
"vec3 m=vec3(0.,0.,0.);"
"vec2 s=f(v,i);"
"float b=s.r,d=s.g;"
"if(d>-.5)"
"{"
"vec3 n=v+b*i,F=f(n),I=reflect(i,F);"
"float p=clamp(dot(F,normalize(vec3(1,1,1))),0.,1.),t=.2*clamp(dot(F,normalize(vec3(.7,-1,.5))),0.,1.),l=.1*clamp(dot(F,normalize(vec3(-.7,-.4,.7))),0.,1.);"
"vec3 x=texture(texTex1,I.rg).rgb,o=texture(texTex2,I.rg*2.).rgb;"
"float a=r(16.*I.rg);"
"vec4 u=(a+1.)*vec4(.1,.3,.5,0.);"
"vec3 z=normalize(vec3(sin(g-.5)*.5,sin(g-.5)*.5,1)),h=normalize(vec3(1,1,1)),N=normalize(vec3(-1,1,1));"
"float T=pow(max(.5,dot(reflect(z,F),i)),sc3_lightscale),R=pow(max(.5,dot(reflect(h,F),i)),sc3_lightscale),G=pow(max(.5,dot(reflect(N,F),i)),sc3_lightscale),C=pow(clamp(dot(I,normalize(vec3(1,1,1))),0.,1.),sc3_lightscale);"
"vec3 k=G*vec3(1,0,0)+R*w(i.rg)+C*w(I.rg)+T*w(n.rg);"
"if(d==2.)"
"m=p*vec3(.1,.3,.5)+t*vec3(.1,.3,.5)+x+o+l*vec3(.1,.3,.5)*u.rgb+k,m*=s.r*.8;"
"if(d==1.)"
"{"
"float D=mod(floor(5.*n.b)+floor(5.*n.r),2.);"
"m=t+x+o+D+l*vec3(.1,.3,.5)*u.rgb+C*w(i.rg)+k;"
"m/=s.r*.74;"
"}"
"float D=c(n,F);"
"vec3 q=normalize(vec3(-.6,.7,-.5));"
"float Z=clamp(.5+.5*I.g,0.,1.),Y=clamp(dot(I,q),0.,1.),X=clamp(dot(I,normalize(vec3(-q.r,0.,-q.b))),0.,1.)*clamp(1.-n.g,0.,1.),W=smoothstep(-.1,.1,I.g),V=pow(clamp(1.+dot(I,i),0.,1.),2.);"
"Y*=e(n,q,.02,2.5);"
"W*=e(n,I,.02,2.5);"
"vec3 U=vec3(0.);"
"U+=.3*Z*vec3(.5,.7,1.)*D;"
"U+=.4*W*vec3(.5,.7,1.)*D;"
"U+=.3*X*vec3(.25,.25,.25)*D;"
"U+=.4*V*vec3(1.,1.,1.)*D;"
"U+=.02;"
"m=m*U;"
"float S=clamp(pow(2.4/length(n-v),2.),0.,1.);"
"vec3 Q=normalize(vec3(1.,.9,.3));"
"float P=dot(i,Q),O=pow(max(P,.9),1.1);"
"vec3 M=mix(vec3(.1,.3,.5),vec3(.1,.3,.5),O);"
"m.rgb=mix(M,m.rgb,S);"
"}"
"m/=s.r*.7;"
"return vec3(clamp(m,0.,1.));"
"}"
"mat3 c(in vec3 v,in vec3 i,float g)"
"{"
"vec3 m=normalize(i-v),b=vec3(sin(g),cos(g),0.),r=normalize(cross(m,b)),f=normalize(cross(r,m));"
"return mat3(r,f,m);"
"}"
"void main()"
"{"
"vec2 i=2.*gl_FragCoord.rg/v-1.;"
"i.r*=v.r/v.g;"
"float m=15.+g;"
"vec3 f=vec3(-.2+3.2*cos(.1*g+6.),1.6,.6+2.*sin(.5*g+6.)),b=vec3(-.5,-.4,.5);"
"mat3 s=c(f,b,0.);"
"vec3 n=s*normalize(vec3(i.rg,2.5)),d=r(f,n);"
"d=pow(d,vec3(1.4545));"
"FragColor=vec4(d,1.);"
"}";