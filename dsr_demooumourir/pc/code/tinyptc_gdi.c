/*
** TinyPTC by Gaffer
** GDI port by Zoon, cleanified by 8bitbubsy
*/

#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include "tinyptc_gdi.h"

static HDC wndHdc;
static HWND wnd;
static BITMAPINFO bmpHeader;
static WORD surfaceWidth;
static WORD surfaceHeight;
static WORD scaledWidth;
static WORD scaledHeight;
static PVOID cachedBuffer;
static BOOL scrSaverEnabled;

static LONG_PTR CALLBACK wndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
        case WM_PAINT:
        {
            if (cachedBuffer)
            {
                StretchDIBits(wndHdc, 0, 0, scaledWidth, scaledHeight, 0, 0, surfaceWidth, 
                    surfaceHeight, cachedBuffer, &bmpHeader, DIB_RGB_COLORS, SRCCOPY);

                ValidateRect(wnd, NULL);
            }
        }
        return 0;

        case WM_KEYDOWN:
        {
            if ((wParam & 0xFF) == 0x1B)
                SendMessageA(wnd, WM_CLOSE, (WPARAM)NULL, (LPARAM)NULL);
        }
        return 0;

        case WM_CLOSE:
        {
            ptcClose();
            ExitProcess(0);
        }
        return 0;

        default:
            return DefWindowProcA(hWnd, message, wParam, lParam);
    }
}

int ptcOpen(const char *title, short width, short height)
{
    WNDCLASSA wc;
    RECT rect;

    cachedBuffer = NULL;
    scrSaverEnabled = FALSE;

	scaledWidth = width *2;
	scaledHeight = height *2;

    memset(&wc, 0, sizeof (WNDCLASSA));
    wc.style = CS_OWNDC | CS_VREDRAW | CS_HREDRAW;
    wc.lpfnWndProc = wndProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = 0;
    wc.hIcon = 0;
    wc.hCursor = LoadCursorA(NULL, IDC_ARROW);
    wc.hbrBackground = 0;
    wc.lpszMenuName = 0;
    wc.lpszClassName = title;

    RegisterClassA(&wc);

    rect.left = 0;
    rect.top = 0;
    rect.right = scaledWidth;
    rect.bottom = scaledHeight;

    AdjustWindowRect(&rect, WS_POPUP | WS_SYSMENU | WS_CAPTION, FALSE);

    rect.right -= rect.left;
    rect.bottom -= rect.top;

    surfaceWidth = width;
    surfaceHeight = height;
	

    wnd = CreateWindowExA(0, title, title, WS_OVERLAPPEDWINDOW & ~WS_MAXIMIZEBOX & ~WS_THICKFRAME,
        (GetSystemMetrics(SM_CXSCREEN) - rect.right) >> 1,
        (GetSystemMetrics(SM_CYSCREEN) - rect.bottom) >> 1,
        rect.right, rect.bottom, 0, 0, 0, NULL);

    ShowWindow(wnd, SW_NORMAL);

    memset(&bmpHeader, 0, sizeof (BITMAPINFOHEADER));
    bmpHeader.bmiHeader.biSize = sizeof (BITMAPINFOHEADER);
    bmpHeader.bmiHeader.biPlanes = 1;
    bmpHeader.bmiHeader.biBitCount = 32;
    bmpHeader.bmiHeader.biCompression = BI_RGB;
    bmpHeader.bmiHeader.biWidth = surfaceWidth;
    bmpHeader.bmiHeader.biHeight = -surfaceHeight;

    wndHdc = GetDC(wnd);

    SystemParametersInfoA(SPI_GETSCREENSAVEACTIVE, 0, &scrSaverEnabled, 0);

    if (scrSaverEnabled == TRUE) // Turn off screen saver if it was on
        SystemParametersInfoA(SPI_SETSCREENSAVEACTIVE, 0, NULL, 0);

    return 1;
}

int ptcUpdate(void *buffer)
{
    MSG message;
    cachedBuffer = buffer;

    InvalidateRect(wnd, NULL, TRUE);
    SendMessageA(wnd, WM_PAINT, (WPARAM)NULL, (LPARAM)NULL);

    while (PeekMessageA(&message, wnd, 0, 0, PM_REMOVE))
    {
        TranslateMessage(&message);
        DispatchMessageA(&message);
    }

    return 1;
}

void ptcClose(void)
{
    cachedBuffer = NULL;

    ReleaseDC(wnd, wndHdc);
    DestroyWindow(wnd);

    if (scrSaverEnabled == TRUE) // Turn screen saver back on if it was on
        SystemParametersInfoA(SPI_SETSCREENSAVEACTIVE, 1, NULL, 0);
}
