//--------------------------------------------------------------------------//
// iq / rgba  .  tiny codes  .  2008                                        //
//--------------------------------------------------------------------------//

#define WIN32_LEAN_AND_MEAN
#define WIN32_EXTRA_LEAN
#include <windows.h>
#include <mmsystem.h>
#include "../intro.h"
#include "../config.h"



static const PIXELFORMATDESCRIPTOR pfd = {
    sizeof(PIXELFORMATDESCRIPTOR), 1, PFD_DRAW_TO_WINDOW|PFD_SUPPORT_OPENGL|PFD_DOUBLEBUFFER, PFD_TYPE_RGBA,
    32, 0, 0, 0, 0, 0, 0, 8, 0, 0, 0, 0, 0, 0, 32, 0, 0, PFD_MAIN_PLANE, 0, 0, 0, 0 };

#ifdef __cplusplus
extern "C" 
{
#endif
int  _fltused = 0;
#ifdef __cplusplus
}
#endif

//----------------------------------------------------------------------------

void entrypoint( void )
{
    // full screen
	DEVMODE screenSettings;
	memset(&screenSettings, 0, sizeof(screenSettings));	// Makes Sure Memory's Cleared
	screenSettings.dmSize = sizeof(screenSettings);		// Size Of The Devmode Structure
	screenSettings.dmPelsWidth = XRES;				// Selected Screen Width
	screenSettings.dmPelsHeight = YRES;				// Selected Screen Height
	screenSettings.dmBitsPerPel = 32;					// Selected Bits Per Pixel
	screenSettings.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT | DM_DISPLAYFREQUENCY;
	screenSettings.dmDisplayFrequency = 60;
    if( ChangeDisplaySettings(&screenSettings,CDS_FULLSCREEN)!=DISP_CHANGE_SUCCESSFUL) return;
    ShowCursor( 0 );
    // create window
    HWND hWnd = CreateWindow( "static",0,WS_POPUP|WS_VISIBLE,0,0,XRES,YRES,0,0,0,0);
    if( !hWnd ) return;
    HDC hDC = GetDC(hWnd);
    // initalize opengl
    if( !SetPixelFormat(hDC,ChoosePixelFormat(hDC,&pfd),&pfd) ) return;
    wglMakeCurrent(hDC,wglCreateContext(hDC));

	TIMECAPS tc;
	timeGetDevCaps(&tc, sizeof(TIMECAPS));
	UINT gwTimerRes = min(max(tc.wPeriodMin, 1), tc.wPeriodMax);
	timeBeginPeriod(gwTimerRes);



    // init intro
    if( !intro_init() ) return;



    // play intro
    long t;
    long to = timeGetTime();
    do 
    {
        t = timeGetTime() - to;
         int done =intro_do( t );
		 if (done)goto exit;

		 static DWORD lasttime = 0;
		 DWORD   time = timeGetTime();
		 unsigned    frameDuration = 1000 / 60;
		 while ((time - lasttime) < frameDuration) {
			 Sleep(0);
			 time = timeGetTime();
		 }
		 lasttime = time;
        wglSwapLayerBuffers( hDC, WGL_SWAP_MAIN_PLANE ); //SwapBuffers( hDC );
    }while ( !GetAsyncKeyState(VK_ESCAPE));
	exit:
    ExitProcess(0);
}
