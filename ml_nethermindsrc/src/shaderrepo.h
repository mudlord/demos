#define STRINGIFY(x) #x



const char vert[] = ""
	"void main()"
	"{"
	"gl_Position=gl_ModelViewProjectionMatrix*gl_Vertex,gl_TexCoord[0]=gl_MultiTexCoord0,gl_TexCoord[1]=gl_MultiTexCoord1;"
	"}";

const char  bg_vert[] = ""
	"varying vec4 vPos;"
	"void main()"
	"{"
	"gl_Position=gl_ModelViewProjectionMatrix*gl_Vertex;"
	"gl_TexCoord[0]=gl_MultiTexCoord0;"

	"vPos=gl_Position;"
	"}";

const char  logo_vert[] = ""
	"varying vec4 vPos;"
	"void main()"
	"{"
	"gl_Position=gl_ModelViewProjectionMatrix*gl_Vertex;"
	"gl_TexCoord[0]=gl_MultiTexCoord0;"

	"vPos=gl_Position;"
	"}";




const char tunnel_fragment[] = ""
	"uniform vec2 resolution,time;"
	"uniform sampler2D tex0,tex1;"
	"void main()"
	"{"
	"vec2 t=-1.+2.*gl_FragCoord.xy/resolution.xy,v,e;"
	"v.x=cos(time.x*1.5)*.5;"
	"v.y=sin(time.x*2.)*.5;"
	"e.x=cos(time.x*2.5)*.5;"
	"e.y=sin(time.x*3.)*.5;"
	"vec2 y,n;"
	"n.x=sin(time.x*.534);"
	"n.y=sin(time.x*.3146);"
	"float s=atan(t.y-n.y,t.x-n.x),x=sqrt(dot(t-n,t-n));"
	"y.x+=.534*time.x.x+1./x;"
	"y.y+=s;"
	"vec3 d=texture2D(tex1,y).xyz;"
	"d*=x;"
	"float r=dot(t-v,t-v)*8.,z=dot(t+e,t+e)*16.,c=d.x/r+d.y/z+d.z/2;"
	"vec2 m=vec2(r/sqrt(z),z/sqrt(r));"
	"vec3 f=texture2D(tex0,m.xy/3.).xyz;"
	"d.xyz+=c;"
	"f*=d;"
	"gl_FragColor=vec4(f,1.);"
	"}";

const char xbr_fragment[] = ""
	"uniform vec2 resolution,time;"
	"uniform sampler2D tex0;"
	"void main()"
	"{"
	"vec2 pos=gl_TexCoord[0].rg;"
	"vec4 color=texture2D(tex0,pos).rgba;"
	"color+= 0.01;"
	"gl_FragColor=vec4(color);"
	"}";

const char bg_fragment[] = ""
	"uniform vec2 resolution,time;"
	"uniform sampler2D tex0;"
	"void main()"
	"{"
	"vec2 pos=gl_TexCoord[0].rg;"
	"vec4 color=texture2D(tex0,pos).rgba;"
	"color+= 0.01;"
	"gl_FragColor=vec4(color);"
	"}";

const char logo_fragment[] = ""
	"uniform vec2 resolution,time;"
	"uniform sampler2D tex0;"
	"void main()"
	"{"
	"vec2 v=gl_TexCoord[0].rg;"
	"v.r+=sin(v.g*4*3.14159+time.x*1.5)/100;"
	"v.g+=sin(v.r*4*3.14159+time.x*1.5)/100;"
	"vec4 r=texture2D(tex0,v).rgba;"
	"gl_FragColor=vec4(r);"
	"}";

