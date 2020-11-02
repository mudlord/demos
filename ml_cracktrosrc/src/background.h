#include "sys/msys.h"
#include "intro.h"
#include <math.h>
#include "WaveFunc.h"
#include "sphtex.h"

size_t blank_len = 82;
unsigned char blank[82]=
{
	0x00,0x00,0x02,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x01,0x00,0x10,0x00,0x20,0x08,0x00,0x00,0x00,0xFF,
	0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,0x00,0x00,0x00,
	0xFF,0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,0x00,0x00,
	0x00,0xFF,0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,0x00,
	0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,
	0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,0x00,0x00,0x00,
	0xFF,0x00,0x00,0x00,0xFF,
};

static int shader;
static int water_shader;

const char vert[] = ""
	"varying vec4 vPos;"
	"void main()"
	"{"
	"gl_Position=gl_ModelViewProjectionMatrix*gl_Vertex,gl_TexCoord[0]=gl_MultiTexCoord0,vPos=gl_Position;"
	"}";

const char shader1frag[] = ""
	"uniform float time;"
	"uniform vec2 resolution;"
	"vec2 t(vec2 t)"
	"{"
	"return t=vec2(dot(t,vec2(127.1,311.7)),dot(t,vec2(269.5,183.3))),fract(sin(t)*43758.5);"
	"}"
	"float r(vec2 v)"
	"{"
	"vec2 r=floor(v),e=fract(v);"
	"float f=8.;"
	"for(float u=-1.;u<=1.;u++)"
	"for(float m=-1.;m<=1.;m++)"
	"{"
	"vec2 g=vec2(m,u),d=t(r+g);"
	"d=.5+.5*sin(time+6.2831*d);"
	"vec2 s=g+d-e;"
	"float o=dot(s,s);"
	"f=min(f,o);"
	"}"
	"return f;"
	"}"
	"void main()"
	"{"
	"vec2 t=gl_FragCoord.rg/resolution.rg;"
	"float f=r(8.*t);"
	"vec3 v=(f+.1)*vec3(.9,.4,1.5);"
	"gl_FragColor=vec4(v,1.);"
	"}";

const char water_frag[] = ""
 "uniform sampler2D tex;"
 "uniform float time;"
 "uniform vec2 resolution;"
 "const float f=3.14159,r=.2,t=.3,n=.3,e=3.;"
 "const int i=4;"
 "const float s=4.;"
 "const int v=3;"
 "const float c=20.,g=400.,o=.7;"
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
   "gl_FragColor=texture2D(tex,u)*s;"
 "}";

int  g_nPrecision = 32;
void renderSphere( float cx, float cy, float cz, float r, int p, float time )
{
    const float PI     = 3.14159265358979f;
    const float TWOPI  = 6.28318530717958f;
    const float PIDIV2 = 1.57079632679489f;

    float theta1 = 0.0;
    float theta2 = 0.0;
    float theta3 = 0.0;

    float ex = 0.0f;
    float ey = 0.0f;
    float ez = 0.0f;

    float px = 0.0f;
    float py = 0.0f;
    float pz = 0.0f;

	WaveFunc wave;
    wave.func = FUNC_SIN;   // sine wave function
    wave.amp = 0.18f;       // amplitude
    wave.freq = 1.0f;       // cycles/sec
    wave.phase = 0;         // horizontal shift
    wave.offset = 0;        // vertical shift
    float waveLength = 2.5f;
	float height;

    // Disallow a negative number for radius.
    if( r < 0 )
        r = -r;

    // Disallow a negative number for precision.
    if( p < 0 )
        p = -p;

    // If the sphere is too small, just render a OpenGL point instead.
    if( p < 4 || r <= 0 ) 
    {
        glBegin( GL_POINTS );
        glVertex3f( cx, cy, cz );
        glEnd();
        return;
    }

    for( int i = 0; i < p/2; ++i )
    {
        theta1 = i * TWOPI / p - PIDIV2;
        theta2 = (i + 1) * TWOPI / p - PIDIV2;

        glBegin( GL_TRIANGLE_STRIP );
        {
            for( int j = 0; j <= p; ++j )
            {
                theta3 = j * TWOPI / p;

                ex = cosf(theta2) * cosf(theta3);
                ey = sinf(theta2);
                ez = cosf(theta2) * sinf(theta3);
                px = cx + r * ex;
                py = cy + r * ey;
                pz = cz + r * ez;

                glNormal3f( ex, ey, ez );
                glTexCoord2f( -(j/(float)p) , 2*(i+1)/(float)p );

			    wave.phase = (px + py + pz) / waveLength;
				height = wave.update(time);

                glVertex3f( px + height, py + height, pz + height);

                ex = cosf(theta1) * cosf(theta3);
                ey = sinf(theta1);
                ez = cosf(theta1) * sinf(theta3);
                px = cx + r * ex;
                py = cy + r * ey;
                pz = cz + r * ez;

				wave.phase = (px + py + pz) / waveLength;
				height = wave.update(time);

                glNormal3f( ex, ey, ez );
                glTexCoord2f( -(j/(float)p), 2*i/(float)p );
                glVertex3f( px + height, py + height, pz + height);
            }
        }
        glEnd();
    }
}



GLuint sphtex1;
GLuint blanktex;
void bg_init()
{
	
	GLuint blanktex = loadTGATextureMemory(blank,blank_len,true);
	sphtex1 = loadTexGenTexMemory(sphtex,sphtex_len,512,512);
	initShader( &shader, vert, (const char*)shader1frag  );
	initShader( &water_shader, vert, (const char*)water_frag  );
}

void bg_do(float sceneTime)
{
	float resolution[2] = {XRES,YRES};
	float time[1] = {sceneTime};
	oglUseProgram(shader);
	oglUniform2fv(oglGetUniformLocation(shader, "resolution" ),1,resolution);
	oglUniform1fv( oglGetUniformLocation( shader, "time" ),  1, time );
    glEnable(GL_TEXTURE_2D);
	BeginOrtho2D(XRES,YRES,true);
	glBindTexture( GL_TEXTURE_2D,  blanktex);
	glTexParameteri (GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri (GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	DrawFullScreenQuad(XRES,YRES);
	glBindTexture( GL_TEXTURE_2D,  0);
	EndProjection();
	oglUseProgram(0 );

    glMatrixMode( GL_MODELVIEW );
    glLoadIdentity();
    glTranslatef( 0.0f, 0.0f, -5.0f );

	//  static float alpha = 0;
	// glRotatef( -alpha, 1.0f, 0.0f, 0.0f );
	// glRotatef( -alpha, 0.0f, 1.0f, 0.0f );


	glDisable(GL_DEPTH_TEST);
    // Render a sphere with texture coordinates
	int shadertex =oglGetUniformLocation(water_shader, "tex");
	oglUseProgram(water_shader);
	oglUniform1i(shadertex, 0);
	resolution[0] = 512;
	resolution[1] = 512;
	oglUniform2fv(oglGetUniformLocation(water_shader, "resolution" ),1,resolution);
	oglUniform1fv( oglGetUniformLocation( water_shader, "time" ),  1, time );
	glBindTexture(GL_TEXTURE_2D,sphtex1);
    renderSphere( 0.0f, 0.0f, 0.0f, 1.5f, g_nPrecision,sceneTime );
	glBindTexture(GL_TEXTURE_2D,0);
	glDisable(GL_DEPTH_TEST);
	oglUseProgram(0 );
	// alpha = alpha + 0.1;
}