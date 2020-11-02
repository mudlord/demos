// /CRINKLER /PRINT:LABELS /PRINT:IMPORTS /COMPMODE:SLOW /HASHSIZE:200 /ORDERTRIES:6000  /UNSAFEIMPORT

//--------------------------------------------------------------------------//
// iq . 2003/2008 . code for 64 kb intros by RGBA                           //
//--------------------------------------------------------------------------//


#ifdef A64BITS
#pragma pack(8) // VERY important, so WNDCLASS get's the correct padding and we don't crash the system
#endif

//#pragma check_stack(off)
//#pragma runtime_checks("", off)


#define ALLOWWINDOWED       // allow windowed mode

#define WIN32_LEAN_AND_MEAN
#define WIN32_EXTRA_LEAN
#include <windows.h>
#include <string.h>
#include "../../intro.h"
#include "../msys.h"
#include "../events.h"
//#include "../../ufmod.h"

//----------------------------------------------------------------------------

typedef struct
{
    HINSTANCE   hInstance;
    HWND        hWnd;
    HDC         hDC;
    HGLRC       hRC;
    int         full;
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



static DEVMODE screenSettings = { {0},
    0,0,156,0,0x001c0000,{0},0,0,0,0,0,{0},0,32,XRES,YRES,{0}, 0,           // Visuatl Studio 2005
    #if(WINVER >= 0x0400)
    0,0,0,0,0,0,
    #if (WINVER >= 0x0500) || (_WIN32_WINNT >= 0x0400)
    0,0
    #endif
    #endif
    };
static const char wndclass[] = "ml_intro";
static const char msg_error[] = 
"This demo failed to load.\n"
"Maybe you have no operating audio service running...\n"
"Or your video card not supporting OGL 1.4\n"
"Or something else :O";


static WININFO     wininfo;

// n = [0..200]
static void loadbar( void *data,float n )
{
	WININFO *info = (WININFO*)data;
	glClear(GL_COLOR_BUFFER_BIT); 
	glDisable(GL_DEPTH_TEST); 

	glClearColor(0.0f,0.0f,0.0f,1.0f);

	//loading bar
	BeginOrtho2D(0, 600, 800, 0); 
	glLineWidth(2);
	glBegin(GL_LINE_LOOP);
	glVertex2i( 95, 285);
	glVertex2i(705, 285);
	glVertex2i(705, 315);
	glVertex2i( 95, 315);
	glEnd();
	glLineWidth(1);

	glBegin(GL_QUADS);
	glVertex2i(100      , 290);
	glVertex2i(100+600*n, 290);
	glVertex2i(100+600*n, 310);
	glVertex2i(100      , 310);
	glEnd();
EndProjection();

	glEnable(GL_DEPTH_TEST); 
	SwapBuffers( wininfo.hDC );
}

//----------------------------------------------------------------------------

static LRESULT CALLBACK WndProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	if( uMsg==WM_SYSCOMMAND && (wParam==SC_SCREENSAVE || wParam==SC_MONITORPOWER) )
		return 0 ;

	if( uMsg==WM_CLOSE || (uMsg==WM_KEYDOWN && wParam==VK_ESCAPE) )
	{
		PostQuitMessage(0);
        return 0 ;
	}

    return DefWindowProc(hWnd,uMsg,wParam,lParam);
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

    #ifdef ALLOWWINDOWED
    if( info->full )
    #endif
    {
        ChangeDisplaySettings( 0, 0 );
		ShowCursor( 1 ); 
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
		screenSettings.dmPelsWidth	= XRES;				// Selected Screen Width
		screenSettings.dmPelsHeight	= YRES;				// Selected Screen Height
		screenSettings.dmBitsPerPel	= 32;					// Selected Bits Per Pixel
		screenSettings.dmDisplayFrequency = 60;
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
    rec.right  = XRES;
    rec.bottom = YRES;

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

    if( !(info->hRC=wglCreateContext(info->hDC)) )
        return( 0 );

    if( !wglMakeCurrent(info->hDC,info->hRC) )
        return( 0 );
    
    SetForegroundWindow(info->hWnd);
    SetFocus(info->hWnd);

    return( 1 );
}


//----------------------------------------------------------------------------
#if 0
extern "C" extern int __cdecl _heap_init (int);
extern "C" extern int __cdecl _mtinit ( void );
extern "C" _CRTIMP int __cdecl _CrtSetCheckCount(int);
extern "C" extern int __cdecl _ioinit (void);
extern "C" extern int __cdecl _cinit (int);

extern "C" 
{
	int _fltused = 0;
}

/*
extern "C" extern int _heap_init(int);
extern "C" extern void _ioinit(void);
extern "C" extern void _cinit(void);

extern "C" extern void _mtinit(void);
*/

#include <rtcapi.h>
extern "C" extern void _RTC_Initialize(void);


int __cdecl MyErrorFunc(int, const wchar_t *, int, const wchar_t *, const wchar_t *, ...)
{
MessageBox(0,"q",0,0);
    return 0;
}


/*
// C version:
_RTC_error_fnW __cdecl _CRT_RTC_INITW(void *res0, void **res1, int res2, int res3, int res4)
{
    return &MyErrorFunc; 
}
*/

// C++ version:
extern "C" _RTC_error_fnW __cdecl _CRT_RTC_INITW(void *res0, void **res1, int res2, int res3, int res4)
{
    return &MyErrorFunc;
}

#include <winbase.h>

 // RunTmChk.lib
#endif


void entrypoint( void )
{
    MSG         msg;
    int         done=0;

    #ifdef ALLOWWINDOWED
    wininfo.full = ( MessageBox( 0, "fullscreen?", wndclass, MB_YESNO|MB_ICONQUESTION)==IDYES );
    #endif
	//wininfo.full = 0;

    if( !window_init(&wininfo) )
    {
        window_end( &wininfo );
        MessageBox( 0,msg_error,0,MB_OK|MB_ICONEXCLAMATION );
        ExitProcess( 0 );
    }
    if( !msys_init((intptr)wininfo.hWnd) ) 
    {
        window_end( &wininfo );
        MessageBox( 0,msg_error,0,MB_OK|MB_ICONEXCLAMATION );
        ExitProcess( 0 );
    }

	TIMECAPS tc;
	timeGetDevCaps(&tc, sizeof(TIMECAPS));
	UINT gwTimerRes = min(max(tc.wPeriodMin, 1), tc.wPeriodMax);
	timeBeginPeriod (gwTimerRes);

    IntroProgressDelegate pd = { &wininfo, loadbar };
	
	owglswapinterval(1);
    if( !intro_init( XRES, YRES, 0, &pd ) )
    {
        window_end( &wininfo );
        MessageBox( 0,msg_error,0,MB_OK|MB_ICONEXCLAMATION );
        ExitProcess( 0 );
    }

	
    while( !done )
    {
		while( PeekMessage(&msg,0,0,0,PM_REMOVE) )
		{
			if( msg.message==WM_QUIT ) { done=1; break; }
			DispatchMessage( &msg );
		}
		
		static DWORD lasttime = 0;
		DWORD   time = timeGetTime();
		unsigned    frameDuration   = 1000 / 60;
		while( (time-lasttime) < frameDuration ) {
			Sleep( 0 );
			time = timeGetTime();
		}
		lasttime = time;


		done |= intro_do();

		SwapBuffers( wininfo.hDC );
    }
	timeEndPeriod (gwTimerRes);

    intro_end();

    window_end( &wininfo );

    msys_end();
    ExitProcess( 0 );
}

