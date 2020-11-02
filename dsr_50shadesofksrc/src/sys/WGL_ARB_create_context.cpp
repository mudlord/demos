//-----------------------------------------------------------------------------
// Copyright (c) 2009 dhpoware. All Rights Reserved.
//
// Permission is hereby granted, free of charge, to any person obtaining a
// copy of this software and associated documentation files (the "Software"),
// to deal in the Software without restriction, including without limitation
// the rights to use, copy, modify, merge, publish, distribute, sublicense,
// and/or sell copies of the Software, and to permit persons to whom the
// Software is furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
// OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
// IN THE SOFTWARE.
//-----------------------------------------------------------------------------

#include <windows.h>
#include <GL/gl.h>
#include <cassert>

#include "WGL_ARB_create_context.h"

#define LOAD_ENTRYPOINT(name, var, type) \
    if (!var) \
    { \
        var = reinterpret_cast<type>(wglGetProcAddress(name)); \
    }

HGLRC wglCreateContextAttribsARB(HDC hDC, HGLRC hShareContext, const int *attribList)
{
    // An OpenGL 3.1 rendering context is created using the new
    // wglCreateContextAttribsARB() function. This new function was introduced
    // in OpenGL 3.0 to maintain backwards compatibility with existing OpenGL
    // 2.1 and older applications. To create an OpenGL 3.1 rendering context
    // first create an OpenGL 2.1 or older rendering context using the
    // wglCreateContext() function. Activate the context and then call the new
    // wglCreateContextAttribsARB() function to create an OpenGL 3.1 rendering
    // context. Once the context is created activate it to enable OpenGL 3.1
    // functionality.
    //
    // For further details see:
    // http://www.opengl.org/registry/specs/ARB/wgl_create_context.txt

    typedef HGLRC (APIENTRY * PFNWGLCREATECONTEXTATTRIBSARBPROC)(HDC hDC, HGLRC hShareContext, const int *attribList);
    static PFNWGLCREATECONTEXTATTRIBSARBPROC pfnCreateContextAttribsARB = 0;    
    
    HGLRC hContext = 0;
    HGLRC hCurrentContext = wglGetCurrentContext();
    
    if (!hCurrentContext)
    {
        if (!(hCurrentContext = wglCreateContext(hDC)))
            return 0;

        if (!wglMakeCurrent(hDC, hCurrentContext))
        {
            wglDeleteContext(hCurrentContext);
            return 0;
        }

        LOAD_ENTRYPOINT("wglCreateContextAttribsARB", pfnCreateContextAttribsARB, PFNWGLCREATECONTEXTATTRIBSARBPROC);
        
        if (pfnCreateContextAttribsARB)
            hContext = pfnCreateContextAttribsARB(hDC, hShareContext, attribList);

        wglMakeCurrent(hDC, 0);
        wglDeleteContext(hCurrentContext);
    }
    else
    {
        if (!wglMakeCurrent(hDC, hCurrentContext))
            return 0;

        LOAD_ENTRYPOINT("wglCreateContextAttribsARB", pfnCreateContextAttribsARB, PFNWGLCREATECONTEXTATTRIBSARBPROC);
        
        if (pfnCreateContextAttribsARB)
            hContext = pfnCreateContextAttribsARB(hDC, hShareContext, attribList);
    }

    return hContext;
}