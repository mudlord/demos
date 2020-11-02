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
#include "../../main.h"
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
	SelectObject( info->hDC, CreateSolidBrush(0x00000000) );
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
#include <CommCtrl.h>

typedef struct {
	int scrWidth;
	int scrHeight;
	int scrBPP;
	int nWindowed;
	int nVsync;

	HINSTANCE hInstance;
} fwzSettings;

#define MAXNUMRES 4096
fwzSettings setup;

typedef struct {
	int w, h;
} RES;
int nRes = 0;
RES ress[MAXNUMRES];
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
BOOL CALLBACK DlgFunc1(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam){
	switch (uMsg) {
	case WM_INITDIALOG: {
		char s[500];
		int i = 0;

		DEVMODE d;
		ZeroMemory(&d, sizeof(d));
		d.dmSize = sizeof(d);
		EnumDisplaySettings(NULL, 0, &d);
		while (1) {
			BOOL h = EnumDisplaySettings(NULL, i++, &d);
			if (!h || nRes > MAXNUMRES) break;
		

			/*** You can use this following line to avoid "rotated" pictures on e.g. laptops ***/
			if (d.dmDisplayOrientation != DMDO_DEFAULT) continue;

			if (d.dmBitsPerPel != setup.scrBPP) continue;
			if (!nRes || ress[nRes - 1].w != d.dmPelsWidth || ress[nRes - 1].h != d.dmPelsHeight) {
				ress[nRes].w = d.dmPelsWidth;
				ress[nRes].h = d.dmPelsHeight;
				nRes++;
				_snprintf(s, 500, "%d * %d", d.dmPelsWidth, d.dmPelsHeight);
				SendDlgItemMessage(hWnd, IDC_RESOLUTION, CB_ADDSTRING, 0, (LPARAM)s);
			}
		}

		int s2 = nRes - 1;
		for (i = 0; i < nRes; i++)
			if (ress[i].w == setup.scrWidth && ress[i].h == setup.scrHeight)
				s2 = i;
		SendDlgItemMessage(hWnd, IDC_RESOLUTION, CB_SETCURSEL, s2, 0);

		if (!setup.nWindowed) {
			SendDlgItemMessage(hWnd, IDC_FULLSCREEN, BM_SETCHECK, 1, 1);
			EnableWindow(GetDlgItem(hWnd, IDC_ONTOP), 0);
		}

	} break;
	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDOK: {
			setup.scrWidth = ress[SendDlgItemMessage(hWnd, IDC_RESOLUTION, CB_GETCURSEL, 0, 0)].w;
			setup.scrHeight = ress[SendDlgItemMessage(hWnd, IDC_RESOLUTION, CB_GETCURSEL, 0, 0)].h;
			setup.nWindowed = !SendDlgItemMessage(hWnd, IDC_FULLSCREEN, BM_GETCHECK, 0, 0);
			EndDialog(hWnd, TRUE);
		} break;
		case IDCANCEL: {
			EndDialog(hWnd, FALSE);
		} break;
		case IDC_FULLSCREEN: {
			/* cake. */
			if (SendDlgItemMessage(hWnd, IDC_FULLSCREEN, BM_GETCHECK, 0, 0)) {
				SendDlgItemMessage(hWnd, IDC_ONTOP, BM_SETCHECK, 0, 0);
				EnableWindow(GetDlgItem(hWnd, IDC_ONTOP), 0);
			}
			else {
				EnableWindow(GetDlgItem(hWnd, IDC_ONTOP), 1);
			}
		} break;
		} break;
	}
	return (WM_INITDIALOG == uMsg) ? TRUE : FALSE;
}


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
	info->full = !setup.nWindowed;
#endif
	//wininfo.full = 0;


    #ifdef ALLOWWINDOWED
    if( info->full )
    #endif
    {
		DEVMODE screenSettings;								// Device Mode
		memset(&screenSettings,0,sizeof(screenSettings));	// Makes Sure Memory's Cleared
		screenSettings.dmSize=sizeof(screenSettings);		// Size Of The Devmode Structure
		screenSettings.dmPelsWidth	= setup.scrWidth;				// Selected Screen Width
		screenSettings.dmPelsHeight	= setup.scrHeight;				// Selected Screen Height
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
    rec.right  = setup.scrWidth;
    rec.bottom = setup.scrHeight;

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


	int attribs[] =
	{
		WGL_CONTEXT_MAJOR_VERSION_ARB, 4,
		WGL_CONTEXT_MINOR_VERSION_ARB, 3,
		WGL_CONTEXT_FLAGS_ARB, WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB,
		0, 0
	};



    if( !(info->hRC=wglCreateContextAttribsARB(info->hDC,0, attribs)) )
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

extern "C" void __cdecl
	_wassert (const wchar_t *_Message, const wchar_t *_File, unsigned _Line)
{

}
extern long GetAudioTime();

static void DrawTime(WININFO *info, float t, int row)
{
	static int      frame = 0;
	static float    to = 0.0;
	static int      fps = 0;
	char            str[64];
	int             s, m, c, seconds;

	if (t<0.0f) return;
	if (info->full) return;

	frame++;
	if ((t - to)>1.0f)
	{
		fps = frame;
		to = t;
		frame = 0;
	}

	if (!(frame & 3))
	{
		seconds = floor(t);
		m = floor(t / 60.0f);
		s = floor(t - 60.0f*(float)m);
		c = (int)floor(t*100.0f) % 100;
		sprintf(str, "%02d:%02d:%02d  [%d fps] : [%d seconds]", m, s, c, fps, seconds);
		SetWindowText(info->hWnd, str);
	}
}


void entrypoint( void )
{
    MSG         msg;
    int         done=0;

	setup.hInstance = GetModuleHandle(0);
	setup.scrBPP = 32;
	setup.scrWidth = 1280;
	setup.scrHeight = 720;
	setup.nWindowed = 1;
	int return1 = DialogBox(setup.hInstance, MAKEINTRESOURCE(IDD_SETUP),NULL, DlgFunc1);
	if (return1 != IDOK){
		return;
	}
	window_xr = setup.scrWidth;
	window_yr = setup.scrHeight;
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

	

    IntroProgressDelegate pd = { &wininfo, loadbar };
    if( !intro_init( setup.scrWidth, setup.scrHeight, 0, &pd ) )
    {
        window_end( &wininfo );
        MessageBox( 0,msg_error,0,MB_OK|MB_ICONEXCLAMATION );
        ExitProcess( 0 );
    }

	Timer::Start();
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
		static long to = 0;
		if (!to) to = Timer::GetTime();
		float t = 0.001f * (float)(Timer::GetTime() - to);
		SwapBuffers( wininfo.hDC );
		DrawTime(&wininfo, t, 0);
    }

    intro_end();

    window_end( &wininfo );

    msys_end();
    ExitProcess( 0 );
}

