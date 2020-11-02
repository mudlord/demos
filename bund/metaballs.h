#include "sys/msys.h"
#include <time.h>
#include <stdint.h>
#include "ttable.h"
#include "metaball_tex.h"

#define PI 3.141592654

#define MB_DETAIL		30
#define MB_CLEAR
//#define MB_METABALLS	1.0
#define MB_METACUBES	1.0

typedef struct {	// Type vertex
	float  x, y, z;
	float nx,ny,nz;
} vertex_type;

typedef struct {	// Type face
	int16_t v1,v2,v3;
} face_type;

vertex_type *vertex;
int16_t			nb_vertices;
face_type	*face;
int16_t			nb_faces;
int16_t         *vertices_grid;
float		*pot_grid;

float			mbx1,mbx2,mby1,mby2,mbz1,mbz2;
int16_t				mbnx,mbny,mbnz;
int16_t			mbnx_x_mbny,mbnx_x_mbny_x_mbnz;
float			mbk;
float			mbt;
unsigned int	mbtexture;
float radius,phi,theta;
int16_t edge[12*4] = { 0,0,0,0, 0,0,1,0, 0,0,0,1, 0,0,1,1,
	1,0,0,0, 1,1,0,0, 1,0,0,1, 1,1,0,1,
	2,0,0,0, 2,1,0,0, 2,0,1,0, 2,1,1,0  };

GLuint metaball_texture;
GLuint metaball_rendtex;
FBOELEM metaball_fbo;
int metaball_shader;
clock_t			mbstart_time;
clock_t			mbcurrent_time;

void compute_f()
{
	int16_t i,j,k;
	float x,y,z;
	float deltax,deltay,deltaz;
	float tx1,ty1,tz1;
	float tx2,ty2,tz2;
	float tx3,ty3,tz3;
	float tx4,ty4,tz4;
	int16_t index;

	tx1 = sin(mbt*2.1) * 2.5;
	ty1 = sin(mbt*1.3) * 2.5;
	tz1 = sin(mbt*3.1) * 2.5;
	tx2 = sin(mbt*2.5) * 2.0;
	ty2 = sin(mbt*2.7) * 2.0;
	tz2 = sin(mbt*1.4) * 2.0;
	tx3 = sin(mbt*3.3) * 1.0;
	ty3 = sin(mbt*1.1) * 1.0;
	tz3 = sin(mbt*1.7) * 1.0;
	tx4 = cos(mbt*5.2) * 1.0;
	ty4 = cos(mbt*3.7) * 1.5;
	tz4 = cos(mbt*4.1) * 1.5;

	deltax = (mbx2-mbx1) / mbnx;
	deltay = (mby2-mby1) / mbny;
	deltaz = (mbz2-mbz1) / mbnz;
	z = mbz1;
	for (k=0; k<=mbnz; k++) {
		y = mby1;
		for (j=0; j<=mbny; j++) {
			x = mbx1;
			for (i=0; i<=mbnx; i++) {
				index = i+j*(mbnx+1)+k*(mbnx+1)*(mbny+1);
#ifdef MB_CLEAR
				pot_grid[index] = 0.0;
#endif
#ifdef MB_METABALLS
				pot_grid[index] +=
					MB_METABALLS*MB_METABALLS/((x-tx1)*(x-tx1) + (y+ty1)*(y+ty1) + (z-tz1)*(z-tz1)) +
					MB_METABALLS*MB_METABALLS/((x-tx2)*(x-tx2) + (y+ty2)*(y+ty2) + (z-tz2)*(z-tz2)) +
					MB_METABALLS*MB_METABALLS/((x-tx3)*(x-tx3) + (y+ty3)*(y+ty3) + (z-tz3)*(z-tz3)) +
					MB_METABALLS*MB_METABALLS/((x-tx4)*(x-tx4) + (y+ty4)*(y+ty4) + (z-tz4)*(z-tz4));
#endif
#ifdef MB_METACUBES
				pot_grid[index] +=
					1.0/((x-tx1)*(x-tx1)*(x-tx1)*(x-tx1) + (y+ty1)*(y+ty1)*(y+ty1)*(y+ty1) + (z-tz1)*(z-tz1)*(z-tz1)*(z-tz1)) +
					1.0/((x-tx2)*(x-tx2)*(x-tx2)*(x-tx2) + (y+ty2)*(y+ty2)*(y+ty2)*(y+ty2) + (z-tz2)*(z-tz2)*(z-tz2)*(z-tz2)) +
					1.0/((x-tx3)*(x-tx3)*(x-tx3)*(x-tx3) + (y+ty3)*(y+ty3)*(y+ty3)*(y+ty3) + (z-tz3)*(z-tz3)*(z-tz3)*(z-tz3)) +
					1.0/((x-tx4)*(x-tx4)*(x-tx4)*(x-tx4) + (y+ty4)*(y+ty4)*(y+ty4)*(y+ty4) + (z-tz4)*(z-tz4)*(z-tz4)*(z-tz4));
#endif
#ifdef MB_USER
				pot_grid[index] +=
					z + sin(sqrt(x*x+y*y)*4.0+mbt*8.0) / 16.0;
#endif
				//				pot_grid[i+j*(mbnx+1)+k*(mbnx_x_mbny+mbny+mbnx+1)] = ((x+tmp)*(x+tmp)+y*y+z*z)*((x-tmp)*(x-tmp)+y*y+z*z)*((z-tmp)*(z-tmp)+y*y+x*x)*((z+tmp)*(z+tmp)+y*y+x*x)*20.0;
				x += deltax;
			}
			y+= deltay;
		}
		z += deltaz;
	}
}


void init_marching_cubes(float x1,float x2,float y1,float y2,float z1,float z2,int16_t m,int16_t n,int16_t p,float pot)
{
	int i,j,k;
	if ( !(vertices_grid = (int16_t *) malloc(sizeof(int16_t)*3*m*n*p)) ) {

	}
	if ( !(vertex = (vertex_type *) malloc(sizeof(vertex_type)*3*m*n*p)) ) {

	}
	if ( !(face = (face_type *) malloc(sizeof(face_type)*4*m*n*p)) ) {

	}
	if ( !(pot_grid = (float *) malloc(sizeof(float)*4*(m+1)*(n+1)*(p+1))) ) {

	}

	mbx1 = x1; mbx2 = x2; mbnx = m;
	mby1 = y1; mby2 = y2; mbny = n;
	mbz1 = z1; mbz2 = z2; mbnz = p;
	mbnx_x_mbny = mbnx * mbny;
	mbnx_x_mbny_x_mbnz = mbnx * mbny * mbnz;
	mbk  = pot;
	mbt  = 0.0;
	mbstart_time = clock();

	for (k=0; k<=mbnz; k++) for (j=0; j<=mbny; j++) for (i=0; i<=mbnx; i++) pot_grid[i+j*(mbnx+1)+k*(mbnx+1)*(mbny+1)] = 0.0;

}

#define f(i,j,k) pot_grid[(i)+(j)*(mbnx+1)+(k)*(mbnx_x_mbny+mbnx+mbny+1)]
void marching_cubes()
{
	float deltax,deltay,deltaz;
	float x,y,z;
	float ax,ay,az,bx,by,bz,nx,ny,nz,n;
	float f1,f2;
	int16_t i,j,k,l;
	int16_t v1,v2,v3;
	float coef;
	int16_t index;
	int16_t eindex;		// Edge index

	nb_vertices = 0;
	nb_faces = 0;
	compute_f();

	deltax = (mbx2-mbx1) / mbnx;
	deltay = (mby2-mby1) / mbny;
	deltaz = (mbz2-mbz1) / mbnz;


	z = mbz1;
	for (k=0; k<mbnz; k++) {
		y = mby1;
		for (j=0; j<mbny; j++) {
			x = mbx1;
			for (i=0; i<mbnx; i++) {
				//-------------------------------------------------------
				index = i+j*(mbnx+1)+k*(mbnx_x_mbny+mbnx+mbny+1);
				f1 = pot_grid[index];
				f2 = pot_grid[index+1];
				if ( ((f1 >= mbk) && (f2 < mbk)) || ((f1 < mbk) && (f2 >= mbk)) ) {

					vertices_grid[i+j*mbnx+k*mbnx_x_mbny+0] = nb_vertices;
					vertex[nb_vertices].nx = 0.0;
					vertex[nb_vertices].ny = 0.0;
					vertex[nb_vertices].nz = 0.0;
					coef = (mbk-f1) / (f2-f1);
					vertex[nb_vertices].x = x + deltax*coef;
					vertex[nb_vertices].y = y;
					vertex[nb_vertices].z = z;
					nb_vertices++;
				}
				f1 = pot_grid[index];
				f2 = pot_grid[index+mbnx+1];
				if ( ((f1 >= mbk) && (f2 < mbk)) || ((f1 < mbk) && (f2 >= mbk)) ) {

					vertices_grid[i+j*mbnx+k*mbnx_x_mbny+mbnx_x_mbny_x_mbnz] = nb_vertices;
					vertex[nb_vertices].nx = 0.0;
					vertex[nb_vertices].ny = 0.0;
					vertex[nb_vertices].nz = 0.0;
					coef = (mbk-f1) / (f2-f1);
					vertex[nb_vertices].x = x;
					vertex[nb_vertices].y = y + deltay*coef;
					vertex[nb_vertices].z = z;
					nb_vertices++;
				}

				f1 = pot_grid[index];
				f2 = pot_grid[index+mbnx_x_mbny+mbnx+mbny+1];
				if ( ((f1 >= mbk) && (f2 < mbk)) || ((f1 < mbk) && (f2 >= mbk)) ) {

				    vertices_grid[i+j*mbnx+k*mbnx_x_mbny+mbnx_x_mbny_x_mbnz+mbnx_x_mbny_x_mbnz] = nb_vertices;
					vertex[nb_vertices].nx = 0.0;
					vertex[nb_vertices].ny = 0.0;
					vertex[nb_vertices].nz = 0.0;
					coef = (mbk-f1) / (f2-f1);
					vertex[nb_vertices].x = x;
					vertex[nb_vertices].y = y;
					vertex[nb_vertices].z = z + deltaz*coef;
					nb_vertices++;
				}
				//-------------------------------------------------------
				x += deltax;
			}
			y += deltay;
		}
		z += deltaz;
	}


	z = mbz1;
	for (k=0; k<mbnz-1; k++) {
		y = mby1;
		for (j=0; j<mbny-1; j++) {
			x = mbx1;
			for (i=0; i<mbnx-1; i++) {
				//-------------------------------------------------------

				index  = ((f(i  ,j  ,k  ) >= mbk)?0x01:0x00);
				index |= ((f(i+1,j  ,k  ) >= mbk)?0x02:0x00);
				index |= ((f(i  ,j+1,k  ) >= mbk)?0x04:0x00);
				index |= ((f(i+1,j+1,k  ) >= mbk)?0x08:0x00);
				index |= ((f(i  ,j  ,k+1) >= mbk)?0x10:0x00);
				index |= ((f(i+1,j  ,k+1) >= mbk)?0x20:0x00);
				index |= ((f(i  ,j+1,k+1) >= mbk)?0x40:0x00);
				index |= ((f(i+1,j+1,k+1) >= mbk)?0x80:0x00);
				index *= 12;

				for (l=0; l<4; l++) {
					if (triangle[index+l*3] != triangle[index+l*3+1]) {

						eindex = triangle[index+l*3]*4;
						v1 = face[nb_faces].v1 = vertices_grid[i+edge[eindex+1]+(j+edge[eindex+2])*mbnx+(k+edge[eindex+3])*mbnx_x_mbny+edge[eindex]*mbnx_x_mbny_x_mbnz];
						eindex = triangle[index+l*3+1]*4;
						v2 = face[nb_faces].v2 = vertices_grid[i+edge[eindex+1]+(j+edge[eindex+2])*mbnx+(k+edge[eindex+3])*mbnx_x_mbny+edge[eindex]*mbnx_x_mbny_x_mbnz];
						eindex = triangle[index+l*3+2]*4;
						v3 = face[nb_faces].v3 = vertices_grid[i+edge[eindex+1]+(j+edge[eindex+2])*mbnx+(k+edge[eindex+3])*mbnx_x_mbny+edge[eindex]*mbnx_x_mbny_x_mbnz];


						ax = vertex[v2].x - vertex[v1].x;
						ay = vertex[v2].y - vertex[v1].y;
						az = vertex[v2].z - vertex[v1].z;
						bx = vertex[v3].x - vertex[v1].x;
						by = vertex[v3].y - vertex[v1].y;
						bz = vertex[v3].z - vertex[v1].z;
						nx = ay*bz - az*by;
						ny = az*bx - ax*bz;
						nz = ax*by - ay*bx;
						n = sqrt(nx*nx+ny*ny+nz*nz);
						if (n != 0.0) {
							n = 1.0/n;
							nx *= n; ny *= n; nz *= n;
						}
						vertex[v1].nx += nx; vertex[v1].ny += ny; vertex[v1].nz += nz;
						vertex[v2].nx += nx; vertex[v2].ny += ny; vertex[v2].nz += nz;
						vertex[v3].nx += nx; vertex[v3].ny += ny; vertex[v3].nz += nz;
						nb_faces++;
					}
				}
				//-------------------------------------------------------
				x += deltax;
			}
			y += deltay;
		}
		z += deltaz;
	}

	for (i=0; i<nb_vertices; i++) {
		n = sqrt(vertex[i].nx*vertex[i].nx + vertex[i].ny*vertex[i].ny + vertex[i].nz*vertex[i].nz );
		if (n != 0.0) {
			n = 1.0/n;
			vertex[i].nx *= n; vertex[i].ny *= n; vertex[i].nz *= n;
		}
	}
}

const char godrays_frag[] = ""
	"uniform sampler2D tex;"
	"const int g=100;"
	"void main()"
	"{"
	"float t=.0034f,v=1.f,f=.15f,x=5.65f;"
	"vec2 i=vec2(0.5f,0.5f),r=vec2(gl_TexCoord[0].rg-i.rg),o=gl_TexCoord[0].rg;"
	"r*=1./float(g)*f;"
	"float c=1.;"
	"for(int m=0;m<g;m++)"
	"{"
	"o-=r;"
	"vec4 s=texture2D(tex,o);"
	"if (s.a > 0.1)s.a = 0.4;"
	"s*=c*x;"
	"gl_FragColor+=s;"
	"c*=v;"
	"}"
	"gl_FragColor*=t;"
	"}";

const char vertex_metaball[] = ""
	"varying vec4 vPos;"
	"void main()"
	"{"
	"gl_Position=gl_ModelViewProjectionMatrix*gl_Vertex,gl_TexCoord[0]=gl_MultiTexCoord0,vPos=gl_Position;"
	"}";

const char fragment[] = ""
	"uniform sampler2D tex;"
	"uniform sampler2D tex2;"
	"void main()"
	"{"
	"vec2 texcoords_tex1 = vec2(gl_TexCoord[0].rg);"
	"vec2 texcoords_tex2 = vec2(gl_TexCoord[1].rg);"
	"vec4 color=texture2D(tex,texcoords_tex1);"
	"if (color.a > 0.1)color.a = 1.0;"
	"gl_FragColor=color;"
	"}";



sprite metaball_fbospr;

void metaballs_init()
{
	init_marching_cubes(-4.0,4.0,-4.0,4.0,-4.0,4.0,MB_DETAIL,MB_DETAIL,MB_DETAIL,1.0);
	metaball_texture = loadTexGenTexMemory(metaball_tex,metaball_tex_len,256,256);
	metaball_fbo = init_fbo(XRES,YRES);
	initShader(  &metaball_shader, (const char*)vertex_metaball, (const char*)godrays_frag);

}

void draw_fonttexture(GLuint texture,int width, int height )
{
	glEnable(GL_TEXTURE_2D);
	glDisable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
	//glAlphaFunc(GL_GREATER, 0.1);
	BeginOrtho2D(width,height,true);
	glBindTexture( GL_TEXTURE_2D,  texture);
	DrawFullScreenQuad(width,height);
	glBindTexture( GL_TEXTURE_2D,  0);
	glDisable(GL_BLEND);
	glEnable(GL_DEPTH_TEST);
	EndProjection();
}

void metaballs_render()
{
	start_fbo(metaball_fbo.fbo,XRES,YRES);
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	Resize(XRES,YRES);

	float rot = clock()>>6;
	BeginPerspective(45.0, (GLdouble)XRES / (GLdouble)YRES) ;
	glTranslatef(0.0,0.0,-10.0);
	glRotatef(rot,1.0f,0.0f,0.0f);
	glRotatef(rot,0.0f,1.0f,0.0f);
	glRotatef(rot,0.0f,0.0f,1.0f);
	mbcurrent_time = clock();
	mbt = (float) (mbcurrent_time-mbstart_time) * 0.0015;
	marching_cubes();
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, metaball_texture);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
	glBegin(GL_TRIANGLES);
	for (int16_t i=0; i<nb_faces; i++) {
		glTexCoord2f((vertex[face[i].v1].nx+1.0)/2.0,(vertex[face[i].v1].ny+1.0)/2.0);
		glVertex3f(vertex[face[i].v1].x,vertex[face[i].v1].y,vertex[face[i].v1].z);
		glTexCoord2f((vertex[face[i].v2].nx+1.0)/2.0,(vertex[face[i].v2].ny+1.0)/2.0);
		glVertex3f(vertex[face[i].v2].x,vertex[face[i].v2].y,vertex[face[i].v2].z);
		glTexCoord2f((vertex[face[i].v3].nx+1.0)/2.0,(vertex[face[i].v3].ny+1.0)/2.0);
		glVertex3f(vertex[face[i].v3].x,vertex[face[i].v3].y,vertex[face[i].v3].z);
	}
	glEnd();
	glBindTexture(GL_TEXTURE_2D, 0);
	glDisable(GL_TEXTURE_2D);
	EndProjection();

	end_fbo();





	//glBindTexture(GL_TEXTURE_2D, metaball_rendtex);
	//glCopyTexImage2D(GL_TEXTURE_2D,0,GL_RGB,0,0,XRES,YRES,0);
	//glBindTexture(GL_TEXTURE_2D, 0);


}

void metaballs_draw()
{
    int shadertex =oglGetUniformLocation(metaball_shader, "tex");
	oglUniform1i(shadertex, 0);
	oglUseProgram(metaball_shader );
	draw_fonttexture(metaball_fbo.texture,XRES,YRES);
	oglUseProgram( NULL );
}
