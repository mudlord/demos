//--------------------------------------------------------------------------//
// iq . 2003/2008 . code for 64 kb intros by RGBA                           //
//--------------------------------------------------------------------------//



#define WIN32_LEAN_AND_MEAN
#define WIN32_EXTRA_LEAN
#include <windows.h>
#include <mmsystem.h>
#include <string.h>
#include <stdio.h>
#include "../../intro.h"
#include "../msys.h"
#include "../events.h"
#include <math.h>


extern "C" {
_declspec(dllexport) DWORD NvOptimusEnablement = 0x00000001;
}

extern "C"
{
  __declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;
	
}

#define ALLOWWINDOWED
//----------------------------------------------------------------------------

typedef struct
{
    //---------------
    HINSTANCE   hInstance;
    HDC         hDC;
    HGLRC       hRC;
    HWND        hWnd;
    //---------------
    int         full;
    //---------------
    MSYS_EVENTINFO   events;
}WININFO;

static const PIXELFORMATDESCRIPTOR pfd =
{
    sizeof(PIXELFORMATDESCRIPTOR),
    1,
    PFD_DRAW_TO_WINDOW|PFD_SUPPORT_OPENGL|PFD_DOUBLEBUFFER,
    PFD_TYPE_RGBA,
    32,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0,
    32,             // zbuffer
    0,              // stencil!
    0,
    PFD_MAIN_PLANE,
    0, 0, 0, 0
};


static const char fnt_wait[]    = "arial";
static const char msg_wait[]   = "wait while loading...";
static const char msg_error[] = "intro_init()!\n\n"\
                             "  no memory?\n"\
                             "  no music?\n"\
                             "  no shaders?";
static const char wndclass[] = "mud_intro";

static WININFO     wininfo;


// n = [0..200]
static void loadbar( void *data,int n )
{
	WININFO *info = (WININFO*)data;
	const int xo = (( 28*window_xr)>>8);
	const int y1 = ((200*window_yr)>>8);
	const int yo = y1-8;

	// draw background
	SelectObject( info->hDC, CreateSolidBrush(0x0045302c) );
	Rectangle( info->hDC, 0, 0, window_xr, window_yr );

	// draw text
	SetBkMode( info->hDC, TRANSPARENT );
	SetTextColor( info->hDC, 0x00ffffff );
	SelectObject( info->hDC, CreateFont( 44,0,0,0,0,0,0,0,0,0,0,ANTIALIASED_QUALITY,0,"arial") );
	TextOut( info->hDC, (window_xr-318)>>1, (window_yr-38)>>1, "wait while loading...", 21 );

	// draw bar
	SelectObject( info->hDC, CreateSolidBrush(0x00705030) );
	Rectangle( info->hDC, xo, yo, (228*window_xr)>>8, y1 );
	SelectObject( info->hDC, CreateSolidBrush(0x00f0d0b0) );
	Rectangle( info->hDC, xo, yo, ((28+n)*window_xr)>>8, y1 );
}

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------



MSYS_EVENTINFO *getEvents( void )
{
    return &wininfo.events;
}

int getKeyPress( int key )
{
    int res;

    res = wininfo.events.keyb.press[key];
    wininfo.events.keyb.press[key] = 0;
    return res;
}


static void procMouse( void )
{
    POINT   p;
    int     i;

    wininfo.events.keyb.state[KEY_LEFT]     = GetAsyncKeyState( VK_LEFT );
	wininfo.events.keyb.state[KEY_RIGHT]    = GetAsyncKeyState( VK_RIGHT );
	wininfo.events.keyb.state[KEY_UP]       = GetAsyncKeyState( VK_UP );
    wininfo.events.keyb.state[KEY_PGUP]     = GetAsyncKeyState( VK_PRIOR );
    wininfo.events.keyb.state[KEY_PGDOWN]   = GetAsyncKeyState( VK_NEXT );
	wininfo.events.keyb.state[KEY_DOWN]     = GetAsyncKeyState( VK_DOWN );
	wininfo.events.keyb.state[KEY_SPACE]    = GetAsyncKeyState( VK_SPACE );
	wininfo.events.keyb.state[KEY_RSHIFT]   = GetAsyncKeyState( VK_RSHIFT );
	wininfo.events.keyb.state[KEY_RCONTROL] = GetAsyncKeyState( VK_RCONTROL );
	wininfo.events.keyb.state[KEY_LSHIFT]   = GetAsyncKeyState( VK_LSHIFT );
	wininfo.events.keyb.state[KEY_LCONTROL] = GetAsyncKeyState( VK_LCONTROL );
	wininfo.events.keyb.state[KEY_1]        = GetAsyncKeyState( '1' );
	wininfo.events.keyb.state[KEY_2]        = GetAsyncKeyState( '2' );
	wininfo.events.keyb.state[KEY_3]        = GetAsyncKeyState( '3' );
	wininfo.events.keyb.state[KEY_4]        = GetAsyncKeyState( '4' );
	wininfo.events.keyb.state[KEY_5]        = GetAsyncKeyState( '5' );
	wininfo.events.keyb.state[KEY_6]        = GetAsyncKeyState( '6' );
	wininfo.events.keyb.state[KEY_7]        = GetAsyncKeyState( '7' );
	wininfo.events.keyb.state[KEY_8]        = GetAsyncKeyState( '8' );
	wininfo.events.keyb.state[KEY_9]        = GetAsyncKeyState( '9' );
	wininfo.events.keyb.state[KEY_0]        = GetAsyncKeyState( '0' );
    for( i=KEY_A; i<=KEY_Z; i++ )
	    wininfo.events.keyb.state[i] = GetAsyncKeyState( 'A'+i-KEY_A );

    //-------
    GetCursorPos( &p );

	wininfo.events.mouse.ox = wininfo.events.mouse.x;
	wininfo.events.mouse.oy = wininfo.events.mouse.y;
	wininfo.events.mouse.x = p.x;
	wininfo.events.mouse.y = p.y;
	wininfo.events.mouse.dx =  wininfo.events.mouse.x - wininfo.events.mouse.ox;
	wininfo.events.mouse.dy =  wininfo.events.mouse.y - wininfo.events.mouse.oy;

	wininfo.events.mouse.obuttons[0] = wininfo.events.mouse.buttons[0];
	wininfo.events.mouse.obuttons[1] = wininfo.events.mouse.buttons[1];
	wininfo.events.mouse.buttons[0] = GetAsyncKeyState(VK_LBUTTON);
	wininfo.events.mouse.buttons[1] = GetAsyncKeyState(VK_RBUTTON);

	wininfo.events.mouse.dbuttons[0] = wininfo.events.mouse.buttons[0] - wininfo.events.mouse.obuttons[0];
	wininfo.events.mouse.dbuttons[1] = wininfo.events.mouse.buttons[1] - wininfo.events.mouse.obuttons[1];
}

//----------------------------------------------------------------------------

static LRESULT CALLBACK WndProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
    int i;

	// salvapantallas
	if( uMsg==WM_SYSCOMMAND && (wParam==SC_SCREENSAVE || wParam==SC_MONITORPOWER) )
		return( 0 );

	// boton x o pulsacion de escape
	if( uMsg==WM_CLOSE || uMsg==WM_DESTROY || (uMsg==WM_KEYDOWN && wParam==VK_ESCAPE) )
	{
		PostQuitMessage(0);
        return( 0 );
	}

    if( uMsg==WM_CHAR )
    {
        int conv = 0;
        switch( wParam )
        {
            case VK_LEFT:     conv = KEY_LEFT;     break;
            case VK_RIGHT:    conv = KEY_RIGHT;    break;
            case VK_UP:       conv = KEY_UP;       break;
            case VK_PRIOR:    conv = KEY_PGUP;     break;
            case VK_NEXT:     conv = KEY_PGDOWN;   break;
            case VK_DOWN:     conv = KEY_DOWN;     break;
            case VK_SPACE:    conv = KEY_SPACE;    break;
            case VK_RSHIFT:   conv = KEY_RSHIFT;   break;
            case VK_RCONTROL: conv = KEY_RCONTROL; break;
            case VK_LSHIFT:   conv = KEY_LSHIFT;   break;
            case VK_LCONTROL: conv = KEY_LCONTROL; break;
        }
        
        for( i=KEY_A; i<=KEY_Z; i++ )
        {
            if( wParam==(WPARAM)('A'+i-KEY_A) )
                conv = i;
            if( wParam==(WPARAM)('a'+i-KEY_A) )
                conv = i;
        }

        wininfo.events.keyb.press[conv] = 1;

        if( wParam==VK_ESCAPE )
        {
            PostQuitMessage(0);
            return( 0 );
        }
    }

    return( DefWindowProc(hWnd,uMsg,wParam,lParam) );
}

static void window_end( WININFO *info )
{
    if( info->hRC )
    {
        wglMakeCurrent( 0, 0 );
        wglDeleteContext( info->hRC );
    }

    if( info->hDC  ) ReleaseDC( info->hWnd, info->hDC );
    if( info->hWnd ) DestroyWindow( info->hWnd );

    UnregisterClass( wndclass, info->hInstance );

    if( info->full )
    {
        ChangeDisplaySettings( 0, 0 );
		while( ShowCursor( 1 )<0 ); // show cursor
    }
}

static int window_init( WININFO *info )
{
	unsigned int	PixelFormat;
	DWORD			dwExStyle, dwStyle;
	RECT			rec;
	WNDCLASSA		wc;

	info->hInstance = GetModuleHandle( 0 );

	memset( &wc, 0, sizeof(WNDCLASSA) );

	wc.style         = CS_OWNDC;
	wc.lpfnWndProc   = WndProc;
	wc.hInstance     = info->hInstance;
	wc.lpszClassName = wndclass;

	if( !RegisterClass((WNDCLASSA*)&wc) )
		return( 0 );

#ifdef ALLOWWINDOWED
	if( info->full )
#endif
	{
		DEVMODE screenSettings;								// Device Mode
		memset(&screenSettings,0,sizeof(screenSettings));	// Makes Sure Memory's Cleared
		screenSettings.dmSize=sizeof(screenSettings);		// Size Of The Devmode Structure
		screenSettings.dmPelsWidth	= window_xr;				// Selected Screen Width
		screenSettings.dmPelsHeight	= window_yr;				// Selected Screen Height
		screenSettings.dmBitsPerPel	= 32;					// Selected Bits Per Pixel
		screenSettings.dmFields=DM_BITSPERPEL|DM_PELSWIDTH|DM_PELSHEIGHT;

		if( ChangeDisplaySettings(&screenSettings,CDS_FULLSCREEN)!=DISP_CHANGE_SUCCESSFUL)
			return( 0 );
		dwExStyle = WS_EX_APPWINDOW;
		dwStyle   = WS_VISIBLE | WS_POPUP | WS_CLIPSIBLINGS | WS_CLIPCHILDREN;
		ShowCursor( 0 );
	}
#ifdef ALLOWWINDOWED
	else
	{
		dwExStyle = WS_EX_APPWINDOW;// | WS_EX_WINDOWEDGE;
		dwStyle   = WS_VISIBLE | WS_CAPTION | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_SYSMENU;
	}
#endif
	rec.left   = 0;
	rec.top    = 0;
	rec.right  = window_xr;
	rec.bottom = window_yr;

#ifdef ALLOWWINDOWED
	AdjustWindowRect( &rec, dwStyle, 0 );
	info->hWnd = CreateWindowEx( dwExStyle, wc.lpszClassName, wc.lpszClassName, dwStyle,
		(GetSystemMetrics(SM_CXSCREEN)-rec.right+rec.left)>>1,
		(GetSystemMetrics(SM_CYSCREEN)-rec.bottom+rec.top)>>1,
		rec.right-rec.left, rec.bottom-rec.top, 0, 0, info->hInstance, 0 );
#else
	info->hWnd = CreateWindowEx( dwExStyle, wc.lpszClassName, wc.lpszClassName, dwStyle, 0, 0, 
		rec.right-rec.left, rec.bottom-rec.top, 0, 0, info->hInstance, 0 );
#endif
	window_handle = info->hWnd;
	if( !info->hWnd )
		return( 0 );

	if( !(info->hDC=GetDC(info->hWnd)) )
		return( 0 );

	if( !(PixelFormat=ChoosePixelFormat(info->hDC,&pfd)) )
		return( 0 );

	if( !SetPixelFormat(info->hDC,PixelFormat,&pfd) )
		return( 0 );


	typedef HGLRC(APIENTRY * PFNWGLCREATECONTEXTATTRIBSARBPROC)(HDC hDC, HGLRC hShareContext, const int *attribList);
	static PFNWGLCREATECONTEXTATTRIBSARBPROC pfnCreateContextAttribsARB = 0;
	GLint attribs[] =
	{
		// Here we ask for OpenGL 4.5
		0x2091, 4,
		0x2092, 3,
		// Uncomment this for forward compatibility mode
		//0x2094, 0x0002,
		// Uncomment this for Compatibility profile
		// 0x9126, 0x9126,
		// We are using Core profile here
		0x9126, 0x00000001,
		0
	};

	HGLRC hTempContext;
	if (!(hTempContext = wglCreateContext(info->hDC)))
		return 0;

	if (!wglMakeCurrent(info->hDC, hTempContext))
	{
		wglDeleteContext(hTempContext);
		return 0;
	}
	pfnCreateContextAttribsARB = reinterpret_cast<PFNWGLCREATECONTEXTATTRIBSARBPROC>(wglGetProcAddress("wglCreateContextAttribsARB"));
	HGLRC CompHRC;
	if (!(CompHRC = pfnCreateContextAttribsARB(info->hDC, 0, attribs)))return 0;
	if (!wglMakeCurrent(info->hDC, CompHRC))return 0;
	info->hRC = CompHRC;
	wglDeleteContext(hTempContext);

	if( !wglMakeCurrent(info->hDC,info->hRC) )
		return( 0 );

	SetForegroundWindow(info->hWnd);
	SetFocus(info->hWnd);

	return( 1 );
}




//----------------------------------------------------------------------------

static void DrawTime( WININFO *info, float t, int row )
{
    static int      frame=0;
    static float    to=0.0;
    static int      fps=0;
    char            str[64];
    int             s, m, c, seconds;

    if( t<0.0f) return;
    if( info->full ) return;

    frame++;
    if( (t-to)>1.0f )
    {
        fps = frame;
        to = t;
        frame = 0;
    }

    if( !(frame&3) )
    {
		seconds = floor(t);
        m = floor( t/60.0f );
        s = floor( t-60.0f*(float)m );
        c = (int)floor( t*100.0f ) % 100;
        sprintf( str, "%02d:%02d:%02d  [%d fps] : [%d seconds] [%0x row]", m, s, c, fps,seconds,row );
        SetWindowText( info->hWnd, str );
    }
}

void calcula( void );



int WINAPI WinMain( HINSTANCE instance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow )
{
    MSG         msg;
    int         done=0;

	wininfo.full =  0;

    wininfo.hInstance = GetModuleHandle( 0 );
    if( !window_init(&wininfo) )
    {
        window_end( &wininfo );
        MessageBox( 0, msg_error,0,MB_OK|MB_ICONEXCLAMATION );
        return( 0 );
    }
    if( !msys_init(0)) 
    {
        window_end( &wininfo );
        MessageBox( 0, msg_error,0,MB_OK|MB_ICONEXCLAMATION );
        return( 0 );
    }
    IntroProgressDelegate pd = { &wininfo, loadbar };
    if( !intro_init( window_xr, window_yr, 1, &pd ) )
    {
        window_end( &wininfo );
        MessageBox( 0, msg_error, 0, MB_OK|MB_ICONEXCLAMATION );
        return( 0 );
    }
	
	TIMECAPS tc;
	timeGetDevCaps(&tc, sizeof(TIMECAPS));
	UINT gwTimerRes = min(max(tc.wPeriodMin, 1), tc.wPeriodMax);
	timeBeginPeriod (gwTimerRes);

	DWORD framecount = 0;
	float rateticks = 1000 / fps_cap;
	DWORD baseticks = timeGetTime();
	DWORD lastticks = baseticks;
	DWORD rate = fps_cap;

    while( !done )
    {
        if( PeekMessage(&msg,0,0,0,PM_REMOVE) )
        {
            if( msg.message==WM_QUIT ) done=1;
		    TranslateMessage( &msg );
            DispatchMessage( &msg );
        }
        else
        {
            procMouse();
            done = intro_do();

			DWORD current_ticks = timeGetTime();
			DWORD target_ticks = baseticks + (DWORD)((float)framecount * rateticks);
			framecount++;
			if (current_ticks <= target_ticks) {
				DWORD the_delay = target_ticks - current_ticks;
				Sleep(the_delay);
			}
			else {
				framecount = 0;
				baseticks = timeGetTime();
			}

            static long to = 0; 
			if( !to ) to=GetTime(); 
            float t = 0.001f * (float)(GetTime() - to);
            SwapBuffers( wininfo.hDC );
            DrawTime( &wininfo, t,0 );
        }
    }

    intro_end();
    window_end( &wininfo );
    return( 0 );
}



