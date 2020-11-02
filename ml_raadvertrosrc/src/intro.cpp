#include "sys/msys.h"
#include "intro.h"


#include "stb_image.c"
#include "scene1.h"
#include "fontwriter.h"

#include "ahx/hvl_replay.h"
#include "music.h"
#define BUFFNUM 8

 /* Device handle */




struct thrddata {
	void *data;
	int len;
	int subsong;
};
bool isplaying=false;
DWORD WINAPI AudioThread(LPVOID lpParameter)
{
	HANDLE eventh;
	HWAVEOUT     hWaveOut = (HWAVEOUT)INVALID_HANDLE_VALUE;
	WAVEFORMATEX wfx;
	LPSTR        audblock;
	char audiobuffer[BUFFNUM][((44100*2*2)/50)];
	struct hvl_tune *ht = NULL;
	thrddata* instance = (thrddata*)lpParameter;
	wfx.nSamplesPerSec  = 44100;
	wfx.wBitsPerSample  = 16;
	wfx.nChannels       = 2;

	wfx.cbSize          = 0;
	wfx.wFormatTag      = WAVE_FORMAT_PCM;
	wfx.nBlockAlign     = (wfx.wBitsPerSample >> 3) * wfx.nChannels;
	wfx.nAvgBytesPerSec = wfx.nBlockAlign * wfx.nSamplesPerSec;

	hvl_InitReplayer();
	ht = hvl_load_ahx((uint8*)instance->data,instance->len,2,44100);
		if( !ht ) return FALSE;

	eventh = CreateEvent(
		NULL,               // default security attributes
		TRUE,               // manual-reset event
		FALSE,              // initial state is nonsignaled
		TEXT("WriteEvent")  // object name
		);
	int nextbuf = 0;
	if( waveOutOpen( (HWAVEOUT*)&hWaveOut, WAVE_MAPPER, &wfx, (unsigned int)eventh, 0, CALLBACK_EVENT ) != MMSYSERR_NOERROR )
	{
		printf( "Unable to open waveout\n" );
		return FALSE;
	}
	int i;
	WAVEHDR header[BUFFNUM];
	for ( i=0; i<BUFFNUM; i++ ){
		memset( &header[i], 0, sizeof( WAVEHDR ) );
		header[i].dwBufferLength = ((44100*2*2)/50);
		header[i].lpData         = (LPSTR)audiobuffer[i];
	}

	for ( i=0; i<BUFFNUM-1; i++ ){
		hvl_DecodeFrame( ht, (int8*)audiobuffer[nextbuf], (int8*)audiobuffer[nextbuf]+2, 4 );
		waveOutPrepareHeader( hWaveOut, &header[nextbuf], sizeof( WAVEHDR ) );
		waveOutWrite( hWaveOut, &header[nextbuf], sizeof( WAVEHDR ) );
		nextbuf = (nextbuf+1)%BUFFNUM;
	}
	isplaying=true;
	while(isplaying)
	{
		hvl_DecodeFrame( ht, (int8*)audiobuffer[nextbuf], (int8*)audiobuffer[nextbuf]+2, 4 );
		waveOutPrepareHeader( hWaveOut, &header[nextbuf], sizeof( WAVEHDR ) );
		waveOutWrite( hWaveOut, &header[nextbuf], sizeof( WAVEHDR ) );
		nextbuf = (nextbuf+1)%BUFFNUM;
		while( waveOutUnprepareHeader( hWaveOut, &header[nextbuf], sizeof( WAVEHDR ) ) == WAVERR_STILLPLAYING ){
			WaitForSingleObject(eventh, INFINITE);
		}
		ResetEvent(eventh);
	}

	if( ht ) hvl_FreeTune( ht );
	if( hWaveOut != INVALID_HANDLE_VALUE )
	{
		for (int i = 0; i < BUFFNUM; ++i)
		{
			if (header[i].lpData)
			{
				waveOutUnprepareHeader(hWaveOut, &header[i], sizeof( WAVEHDR ) );
				header[i].dwFlags &= ~WHDR_PREPARED;
			}
		}
		waveOutReset(hWaveOut);
		waveOutClose(hWaveOut);
	}
}



void glenz_side(GLint r,GLint g,GLint b,GLfloat l, float point)
{
	GLfloat d=0.5f;
	GLfloat w[4]={1.0f,1.0f,1.0f,l};
	GLfloat c[4]={(GLfloat)(r),(GLfloat)(g),(GLfloat)(b),l};

	glBegin(GL_TRIANGLES);				
	glMaterialfv(GL_FRONT_AND_BACK,GL_AMBIENT_AND_DIFFUSE,w);
	glNormal3f(0.25f,-0.25f,-0.75f);
	glVertex3f(-d, d, d);
	glVertex3f( d, d, d);
	glVertex3f(0.0,0.0,(GLfloat)(d*point));		
	glMaterialfv(GL_FRONT_AND_BACK,GL_AMBIENT_AND_DIFFUSE,c);
	glNormal3f(-0.25f,-0.25f,-0.75f);
	glVertex3f( d, d, d);
	glVertex3f( d,-d, d);
	glVertex3f(0.0,0.0,(GLfloat)(d*point));		
	glMaterialfv(GL_FRONT_AND_BACK,GL_AMBIENT_AND_DIFFUSE,w);
	glNormal3f(-0.25f,0.25f,-0.75f);
	glVertex3f( d,-d, d);
	glVertex3f(-d,-d, d);
	glVertex3f(0.0,0.0,(GLfloat)(d*point));		
	glMaterialfv(GL_FRONT_AND_BACK,GL_AMBIENT_AND_DIFFUSE,c);
	glNormal3f(0.25f,0.25f,-0.75f);
	glVertex3f(-d,-d, d);
	glVertex3f(-d, d, d);
	glVertex3f(0.0,0.0,(GLfloat)(d*point));
	glEnd();
}

typedef struct{
	float x,y,z;
}rotate;

typedef struct{
	float r,g,b,a;
}colors;

void gen_glenz(float r,float g,float b,float l, rotate rot, float point = 1.75)
{
	//glLoadIdentity();
	glRotatef(rot.x,1.0f,0.0f,0.0f);
	glRotatef(rot.y,0.0f,1.0f,0.0f);
	glRotatef(rot.z,0.0f,0.0f,1.0f);

	glenz_side(r,g,b,l,point);
	glRotatef(90,0.0f,0.0f,1.0f);
	glRotatef(90,1.0f,0.0f,0.0f);
	glenz_side(r,g,b,l,point);
	glRotatef(180,0.0f,1.0f,0.0f);
	glenz_side(r,g,b,l,point);
	glRotatef(90,0.0f,0.0f,1.0f);
	glRotatef(270,0.0f,1.0f,0.0f);
	glenz_side(r,g,b,l,point);
	glRotatef(90,0.0f,0.0f,1.0f);
	glRotatef(90,0.0f,1.0f,0.0f);
	glenz_side(r,g,b,l,point);
	glRotatef(180,0.0f,1.0f,0.0f);
	glenz_side(r,g,b,l,point);
}

void draw_glenz(double x, double y, double z,rotate rot, colors col,float point = 1.75)
{
	BeginPerspective(45.0, (GLdouble)XRES / (GLdouble)YRES) ;
	glTranslatef(x,y,z);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	float posLight0[4] = {-1.0f, 1.0f, 1.0f, 0.0f};
	float ambLight0[4] = {0.5f, 0.5f, 0.5f, 0.5f};
	glLightfv(GL_LIGHT0, GL_POSITION, posLight0);
	glLightfv(GL_LIGHT0, GL_AMBIENT, ambLight0);
	float specLight0[4] = {0.5f, 0.5f, 0.5f, 1.0f};
	glLightfv(GL_LIGHT0, GL_SPECULAR, specLight0);
	gen_glenz(col.r,col.g,col.b,col.a,rot,point);
	glDisable(GL_LIGHTING);
	glDisable(GL_LIGHT0);
	glDisable(GL_BLEND);
	EndProjection();
}


GLuint cube_creditstex;


thrddata* threaddata;
HANDLE g_handle;

int intro_init( int xr, int yr, int nomusic, IntroProgressDelegate *pd )
{
	Resize(XRES,YRES);

	srand(NULL);
    // progress report, (from 0 to 200)
    pd->func( pd->obj, 0 );
	scene1_init();
	srand(NULL);
	fontwriter_init();

	glEnable(GL_TEXTURE_2D);
	glGenTextures(1, &cube_creditstex);
	glBindTexture(GL_TEXTURE_2D,  cube_creditstex);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA8,0,XRES,YRES,GL_RGB,GL_UNSIGNED_BYTE, 0);
	glCopyTexImage2D(GL_TEXTURE_2D,0,GL_RGB,0,0,XRES,YRES,0);
	glBindTexture(GL_TEXTURE_2D,0);
	glDisable(GL_TEXTURE_2D);


	DWORD  g_id = NULL;     
	threaddata = (thrddata*)malloc( sizeof(thrddata));
	threaddata->data = (void*)music;
	threaddata->len = music_len;
	g_handle = CreateThread( NULL, 0, AudioThread,(LPVOID)threaddata,   NULL, &g_id );
    pd->func( pd->obj, 1.0 );
    return 1;
}

void intro_end()
{
	isplaying=false;
	WaitForSingleObject(g_handle,INFINITE);
	free(threaddata);

}

#define PI 3.14159265358// pi
#define PID PI/180.0 // pi ratio



void render_cube(float x, float y,float z)
{
	static GLfloat	xrot = 0;				// X Rotation ( NEW )
	static GLfloat	yrot= 0;				// Y Rotation ( NEW )
	static GLfloat	zrot= 0;				// Z Rotation ( NEW )
	glPushMatrix();
	glLoadIdentity();                           // Reset The Current Matrix
	glTranslatef(x,y,z);                      // Move Into The Screen 5 Units
	glRotatef(xrot,1.0f,0.0f,0.0f);                     // Rotate On The X Axis
	glRotatef(yrot,0.0f,1.0f,0.0f);                     // Rotate On The Y Axis
	glRotatef(zrot,0.0f,0.0f,1.0f);                     // Rotate On The Z Axis
	glEnable(GL_TEXTURE_2D);
	glBindTexture( GL_TEXTURE_2D,  cube_creditstex);

	glBegin(GL_QUADS);
	// Front Face
	glTexCoord2f(0.0f, 0.0f); glVertex3f(-1.0f, -1.0f,  1.0f);  // Bottom Left Of The Texture and Quad
	glTexCoord2f(1.0f, 0.0f); glVertex3f( 1.0f, -1.0f,  1.0f);  // Bottom Right Of The Texture and Quad
	glTexCoord2f(1.0f, 1.0f); glVertex3f( 1.0f,  1.0f,  1.0f);  // Top Right Of The Texture and Quad
	glTexCoord2f(0.0f, 1.0f); glVertex3f(-1.0f,  1.0f,  1.0f);  // Top Left Of The Texture and Quad
	// Back Face
	glTexCoord2f(1.0f, 0.0f); glVertex3f(-1.0f, -1.0f, -1.0f);  // Bottom Right Of The Texture and Quad
	glTexCoord2f(1.0f, 1.0f); glVertex3f(-1.0f,  1.0f, -1.0f);  // Top Right Of The Texture and Quad
	glTexCoord2f(0.0f, 1.0f); glVertex3f( 1.0f,  1.0f, -1.0f);  // Top Left Of The Texture and Quad
	glTexCoord2f(0.0f, 0.0f); glVertex3f( 1.0f, -1.0f, -1.0f);  // Bottom Left Of The Texture and Quad
	// Top Face
	glTexCoord2f(0.0f, 1.0f); glVertex3f(-1.0f,  1.0f, -1.0f);  // Top Left Of The Texture and Quad
	glTexCoord2f(0.0f, 0.0f); glVertex3f(-1.0f,  1.0f,  1.0f);  // Bottom Left Of The Texture and Quad
	glTexCoord2f(1.0f, 0.0f); glVertex3f( 1.0f,  1.0f,  1.0f);  // Bottom Right Of The Texture and Quad
	glTexCoord2f(1.0f, 1.0f); glVertex3f( 1.0f,  1.0f, -1.0f);  // Top Right Of The Texture and Quad
	// Bottom Face
	glTexCoord2f(1.0f, 1.0f); glVertex3f(-1.0f, -1.0f, -1.0f);  // Top Right Of The Texture and Quad
	glTexCoord2f(0.0f, 1.0f); glVertex3f( 1.0f, -1.0f, -1.0f);  // Top Left Of The Texture and Quad
	glTexCoord2f(0.0f, 0.0f); glVertex3f( 1.0f, -1.0f,  1.0f);  // Bottom Left Of The Texture and Quad
	glTexCoord2f(1.0f, 0.0f); glVertex3f(-1.0f, -1.0f,  1.0f);  // Bottom Right Of The Texture and Quad
	// Right face
	glTexCoord2f(1.0f, 0.0f); glVertex3f( 1.0f, -1.0f, -1.0f);  // Bottom Right Of The Texture and Quad
	glTexCoord2f(1.0f, 1.0f); glVertex3f( 1.0f,  1.0f, -1.0f);  // Top Right Of The Texture and Quad
	glTexCoord2f(0.0f, 1.0f); glVertex3f( 1.0f,  1.0f,  1.0f);  // Top Left Of The Texture and Quad
	glTexCoord2f(0.0f, 0.0f); glVertex3f( 1.0f, -1.0f,  1.0f);  // Bottom Left Of The Texture and Quad
	// Left Face
	glTexCoord2f(0.0f, 0.0f); glVertex3f(-1.0f, -1.0f, -1.0f);  // Bottom Left Of The Texture and Quad
	glTexCoord2f(1.0f, 0.0f); glVertex3f(-1.0f, -1.0f,  1.0f);  // Bottom Right Of The Texture and Quad
	glTexCoord2f(1.0f, 1.0f); glVertex3f(-1.0f,  1.0f,  1.0f);  // Top Right Of The Texture and Quad
	glTexCoord2f(0.0f, 1.0f); glVertex3f(-1.0f,  1.0f, -1.0f);  // Top Left Of The Texture and Quad
	glEnd();
	glDisable(GL_TEXTURE_2D);
	glPopMatrix();
	xrot+=0.3f;                             // X Axis Rotation
	yrot+=0.2f;                             // Y Axis Rotation
	zrot+=0.4f;                             // Z Axis Rotation
}

int intro_do( void )
{
	static long lastTime = timeGetTime();
	long currTime = timeGetTime();
	static double delta = 0.0;
	long diff= currTime - lastTime;
	//delta = delta*0.5 + (diff/20.0)*0.5;
	delta = diff;
	lastTime = currTime;
	static float sceneTime = 0;
	sceneTime += (float)delta/1000.f;
	static float texttime = 0;
	texttime += (float)delta/1000.f;

	//shader rendering
	glClearColor(0.0, 0.0,0.0, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glScissor(1	,int(0.075f*YRES),XRES-2,int(0.85f*YRES));
	glEnable(GL_SCISSOR_TEST); 
	//shader timeline
	glClearColor(1.0, 1.0,0.0, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	scene1_render(sceneTime,delta);

	static rotate rot3 = {0.00,0.0,0.0};
	const colors col2 = {70/255.0,31/255.0,125/255.0,1.0};
	rot3.x += delta/6;
	rot3.y += delta/5.4;
	static GLfloat	r_y;	
	static GLfloat c_y=-3.85f;
	GLfloat c_z=-13.0f;
	GLfloat j_y=3.85f;// jump
	GLfloat j_r=0.0f;
	r_y=(GLfloat)(0.01*(sceneTime-lastTime));
	GLint g_n=15;	

	for(int i=0;i<g_n;i++)
	{
		GLfloat p_x=(GLfloat)((8.0+3.0*cos(r_y*4.0*PID))*cos((360/g_n*i+r_y*2.0)*PID));
		GLfloat p_y=(GLfloat)(c_y+j_y*cos(j_r)+1.0*cos((360/g_n*i+r_y*4.0)*PID));	// -c_j*cos(c_r)
		GLfloat p_z=(GLfloat)(c_z+(8.0+3.0*cos(r_y*4.0*PID))*sin((360/g_n*i+r_y*2.0)*PID));
		draw_glenz(p_x, p_y, p_z,rot3,col2,1.0);
	}
		
	glDisable(GL_SCISSOR_TEST);
	logo_draw();
	draw_twister(delta);

	render_cube(0.0,0.0,-10.0);

	fontwrite(delta);

glBindTexture( GL_TEXTURE_2D,  cube_creditstex);
	glCopyTexSubImage2D(GL_TEXTURE_2D,0,0,0,0,0,XRES,YRES);
	glBindTexture(GL_TEXTURE_2D,0);






	return 0;

}