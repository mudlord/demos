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
#include "../../../resource.h"

extern "C" {
	_declspec(dllexport) DWORD NvOptimusEnablement = 0x00000001;
}
extern "C"
{
	__declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;
}

//#include "../../ufmod.h"

int main(int argc, char **argv) {
		return 0;
}

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
    0,0,156,0,0x001c0000,{0},0,0,0,0,0,{0},0,32,window_xr,window_yr,{0}, 0,           // Visuatl Studio 2005
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
"Or your video card not supporting OGL 4.3\n"
"Or something else :O";


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

#include "../msys_glext.h"




static int window_init( WININFO *info )
{
	unsigned int	PixelFormat;
    DWORD			dwExStyle, dwStyle;
    RECT			rec;
    WNDCLASSA		wc;

	info->hInstance = GetModuleHandle(0);
	

    memset( &wc, 0, sizeof(WNDCLASSA) );

    wc.style         = CS_OWNDC;
    wc.lpfnWndProc   = WndProc;
    wc.hInstance     = info->hInstance;
    wc.lpszClassName = wndclass;

    if( !RegisterClass((WNDCLASSA*)&wc) )
        return( 0 );

	
	


#ifdef ALLOWWINDOWED
	info->full = ( MessageBox( 0, "fullscreen?", wndclass, MB_YESNO|MB_ICONQUESTION)==IDYES );
#endif
	//wininfo.full = 0;


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

extern "C" void __cdecl
	_wassert (const wchar_t *_Message, const wchar_t *_File, unsigned _Line)
{

}
extern long GetAudioTime();

void entrypoint( void )
{
    MSG         msg;
    int         done=0;

    if( !window_init(&wininfo) )
    {
        window_end( &wininfo );
        MessageBox( 0,msg_error,0,MB_OK|MB_ICONEXCLAMATION );
        ExitProcess( 0 );
    }
    if( !msys_init((intptr_t)wininfo.hWnd) ) 
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
    if( !intro_init( window_xr, window_yr, 0, &pd ) )
    {
        window_end( &wininfo );
        MessageBox( 0,msg_error,0,MB_OK|MB_ICONEXCLAMATION );
        ExitProcess( 0 );
    }



	DWORD framecount = 0;
	float rateticks = 1000 / fps_cap;
	DWORD baseticks = timeGetTime();
	DWORD lastticks = baseticks;
	DWORD rate = fps_cap;
	
    while( !done )
    {
		while( PeekMessage(&msg,0,0,0,PM_REMOVE) )
		{
			if( msg.message==WM_QUIT ) { done=1; break; }
			DispatchMessage( &msg );
		}

		done |= intro_do();

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
		SwapBuffers( wininfo.hDC );
    }
	timeEndPeriod (gwTimerRes);

    intro_end();

    window_end( &wininfo );

    msys_end();
    ExitProcess( 0 );
}

