#include "bass.h"
#include "sys/msys.h"
#include "intro.h"

DWORD bass_channel;

typedef struct {
	BYTE *Address;
	DWORD Length;
} MUSIC, *LPMUSIC;
MUSIC Music_Table[1];

struct songdata
{
	char * filename;
	char* name;
};

#pragma pack(1)

int m_mouseY;
int m_mouseX;

songdata tunes[] = {
	{"test1.xm","test1"},
	{"test2.xm","test2"},
	{"test3.xm","test3"},
};
const int numtunes = sizeof(tunes)/sizeof(songdata);
int song_playing;


sprite play_spr;
sprite prev_spr;
sprite next_spr;

sprite scope_spr;
GLuint scope_texture;
int scope_w =  368;
int scope_h = 127;
#define SPECWIDTH  368
#define SPECHEIGHT 127
RGB *scopebuffer;
RGB  scope_palette[256];
int scopetexsize;


GLuint play_sprite;
GLuint stop_sprite;
GLuint prev_sprite;
GLuint next_sprite;
GLuint playover_sprite;
GLuint stopover_sprite;
GLuint prevover_sprite;
GLuint nextover_sprite;
bool reg_click;

void scope_init()
{
	{ // setup palette
		RGB *pal=scope_palette;
		int a;
		memset(scope_palette,0,sizeof(scope_palette));
		for (a=1;a<128;a++) {
			pal[a].g=256-2*a;
			pal[a].r=2*a;
		}
		for (a=0;a<32;a++) {
			pal[128+a].b=8*a;
			pal[128+32+a].b=255;
			pal[128+32+a].r=8*a;
			pal[128+64+a].r=255;
			pal[128+64+a].b=8*(31-a);
			pal[128+64+a].g=8*a;
			pal[128+96+a].r=255;
			pal[128+96+a].g=255;
			pal[128+96+a].b=8*a;
		}
	}

	glGenTextures(1, &scope_texture);
	glBindTexture(GL_TEXTURE_2D, scope_texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, 3, scope_w, scope_h, 0, GL_RGB, GL_UNSIGNED_BYTE,NULL);
	glBindTexture(GL_TEXTURE_2D,0);

	scope_spr.xsize = 102;
	scope_spr.ysize = 70;
	scope_spr.x = 321;
	scope_spr.y = 219;
	scope_spr.texture = scope_texture;
	scopetexsize=163*109*3;
	scopebuffer=(RGB*)malloc(scopetexsize);
}

void scope_do()
{
	memset(scopebuffer,0,scopetexsize*sizeof(*scopebuffer));
	int x,y,y1;
	#define BANDS 28
	float fft[1024];
	BASS_ChannelGetData(bass_channel,fft,BASS_DATA_FFT2048); // get the FFT data
	int b0=0;
	for (x=0;x<BANDS;x++) {
		float peak=0;
		int b1=pow(2,x*10.0/(BANDS-1));
		if (b1>1023) b1=1023;
		if (b1<=b0) b1=b0+1; // make sure it uses at least 1 FFT bin
		for (;b0<b1;b0++)
			if (peak<fft[1+b0]) peak=fft[1+b0];
		y=sqrt(peak)*3*SPECHEIGHT-4; // scale it (sqrt to make low values more visible)
		if (y>SPECHEIGHT) y=SPECHEIGHT; // cap it
		while (--y>=0)
			for (y1=0;y1<SPECWIDTH/BANDS-2;y1++)
				scopebuffer[(SPECHEIGHT-1-y)*SPECWIDTH+x*(SPECWIDTH/BANDS)+y1]=scope_palette[y+1]; // draw bar
	}

	glBindTexture(GL_TEXTURE_2D, scope_texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, 3, scope_w, scope_h, 0, GL_RGB, GL_UNSIGNED_BYTE,scopebuffer);
	glBindTexture(GL_TEXTURE_2D,0);
	draw_sprite(scope_spr, XRES, YRES, false);
}

void music_playfile(int song)
{
	if (BASS_ChannelIsActive(bass_channel))BASS_ChannelStop(bass_channel);
	bass_channel=BASS_StreamCreateFile(FALSE,tunes[song].filename,0,0,BASS_STREAM_AUTOFREE|BASS_SAMPLE_LOOP);
	if (!bass_channel)bass_channel=BASS_MusicLoad(FALSE,tunes[song].filename,0,0,BASS_MUSIC_AUTOFREE | BASS_SAMPLE_LOOP|BASS_MUSIC_RAMPS,0);
	BASS_ChannelPlay(bass_channel,TRUE);
}

float GetAmp(DWORD channel, float start, float end)
{
	float fft[512],amp=0,freq;
	int b,bin1,bin2;
	BASS_ChannelGetAttribute(channel,BASS_ATTRIB_FREQ,&freq); // get sample rate
	BASS_ChannelGetData(channel,fft,BASS_DATA_FFT1024); // get FFT data
	bin1=(int)(1024*start/(float)freq+0.5-((int)freq>>31));
	bin2=(int)(1024*end/(float)freq+0.5-((int)freq>>31)); // nearest bin to ending freq
	for (b=bin1;b<=bin2;b++) if (fft[b]>amp) amp=fft[b]; // check the bins
	return amp;
}

void music_init()
{
	if (!BASS_Init(-1,44100,0,0,NULL))return;
	song_playing = 0;
 
	int m_mouseY= 0;
	int m_mouseX = 0;
	reg_click = false;

	play_sprite = loadDDSTextureMemory(play_dat,play_len,false);
	stop_sprite = loadDDSTextureMemory(stop_dat,stop_len,false);
	prev_sprite = loadDDSTextureMemory(prev_dat,prev_len,false);
    next_sprite = loadDDSTextureMemory(next_dat,next_len,false);
	playover_sprite = loadDDSTextureMemory(playover_dat,playover_len,false);
	stopover_sprite = loadDDSTextureMemory(stopover_dat,stopover_len,false);
	prevover_sprite = loadDDSTextureMemory(prevover_dat,prevover_len,false);
	nextover_sprite = loadDDSTextureMemory(nextover_dat,nextover_len,false);

	float tSize = 50;
	play_spr.xsize = tSize;
	play_spr.ysize = tSize;
	play_spr.x = 321;
	play_spr.y = 127;
	play_spr.texture = play_sprite;

	prev_spr.xsize = tSize;
	prev_spr.ysize = tSize;
	prev_spr.x = 235;
	prev_spr.y = 127;
	prev_spr.texture = prev_sprite;

	next_spr.xsize = tSize;
	next_spr.ysize = tSize;
	next_spr.x = 405;
	next_spr.y = 127;
	next_spr.texture = next_sprite;

	scope_init();
//  music_playfile(song_playing);
}



void music_do()
{
	POINT pt;
	GetCursorPos(&pt);
	ScreenToClient(GetForegroundWindow(), &pt);
	m_mouseX = pt.x;
	m_mouseY = pt.y;
	// Ensure the mouse location doesn't exceed the screen width or height.
	if(m_mouseX < 0)  { m_mouseX = 0; }
	if(m_mouseY < 0)  { m_mouseY = 0; }

	if(m_mouseX > 1024)  { m_mouseX = 1024; }
	if(m_mouseY > 768) { m_mouseY = 768; }
	bool left_button = ((GetAsyncKeyState(VK_LBUTTON) & 0x8000) == 0x8000 );
	//do regions
	//prev
	if ((m_mouseX > 339 && m_mouseX < 409) && (m_mouseY > 523 && m_mouseY < 599))
	{
		if (song_playing >= 0 )
		{
			if (left_button && !reg_click)
			{
				if (song_playing > 0)
				{
					music_playfile(song_playing-1);
					--song_playing;
					reg_click=true;
				}	
			}
			if (!left_button)
			{
				reg_click = false;
			}
		}
		prev_spr.texture = prevover_sprite;
		draw_sprite(prev_spr,XRES,YRES);
	}
	else
	{
		prev_spr.texture = prev_sprite;
		draw_sprite(prev_spr,XRES,YRES);
	}

	//play/pause
	if ((m_mouseX > 473 && m_mouseX < 546) && (m_mouseY > 523 && m_mouseY < 599))
	{
		if (BASS_ChannelIsActive(bass_channel) == BASS_ACTIVE_PAUSED)
		{
			play_spr.texture = playover_sprite;
			draw_sprite(play_spr,XRES,YRES);
			if (left_button && !reg_click)
			{
				BASS_ChannelPlay(bass_channel,false);
				reg_click=true;
			}
			if (!left_button)
			{
				reg_click = false;
			}
		}
		else
		{
			play_spr.texture = stopover_sprite;
			draw_sprite(play_spr,XRES,YRES);
			if (left_button && !reg_click)
			{
				BASS_ChannelPause(bass_channel);
				reg_click=true;
			}
			if (!left_button)
			{
				reg_click = false;
			}
		}
		
	}
	else
	{
		if (BASS_ChannelIsActive(bass_channel) == BASS_ACTIVE_PAUSED)
		{
		play_spr.texture = play_sprite;
		draw_sprite(play_spr,XRES,YRES);
		}
		else
		{
			play_spr.texture = stop_sprite;
			draw_sprite(play_spr,XRES,YRES);
		}
	}


	//next
	if ((m_mouseX > 611 && m_mouseX < 684) && (m_mouseY > 523 && m_mouseY < 599))
	{
		if (song_playing < (numtunes-1) )
		{
			if (left_button && !reg_click)
			{
				if (song_playing != (numtunes-1))
				{

					music_playfile(song_playing+1);
				    ++song_playing;
					reg_click=true;
				}	
			}
			if (!left_button)
			{
				reg_click = false;
			}
		
		}
		next_spr.texture = nextover_sprite;
		draw_sprite(next_spr,XRES,YRES);
	}
	else
	{
		next_spr.texture = next_sprite;
		draw_sprite(next_spr,XRES,YRES);
	}
	

	
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDisable(GL_DEPTH_TEST);
	BeginOrtho2D(XRES, YRES);
	fnt->Print(0, YRES-30, "mouse pos X: %d",m_mouseX);
	fnt->Print(0, YRES-50, "mouse pos Y: %d",m_mouseY);
	fnt->Print(0, YRES-70, "song id: %s",tunes[song_playing].name);
	EndProjection();
	glEnable(GL_DEPTH_TEST);
	glDisable(GL_TEXTURE_2D);
	glDisable(GL_BLEND);

	scope_do();

}

void music_free()
{

}
