#include "common/context.hpp"

#include <windows.h>
#include <iostream>
#include <GL/GL.h>

static HGLRC context;

bool contextCreate()
{
	PIXELFORMATDESCRIPTOR pfd =
	{
		sizeof(PIXELFORMATDESCRIPTOR),
		1,
		PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL,
		PFD_TYPE_RGBA,
		32, // Colordepth of the framebuffer.
		0, 0, 0, 0, 0, 0,
		0,
		0,
		0,
		0, 0, 0, 0,
		24, // Number of bits for the depthbuffer
		8,  // Number of bits for the stencilbuffer
		0,  // Number of Aux buffers in the framebuffer.
		PFD_MAIN_PLANE,
		0,
		0, 0, 0 };

	auto wnd = GetDesktopWindow();
	auto dc = GetDC(wnd);

	auto pixelFormat = ChoosePixelFormat(dc, &pfd);
	SetPixelFormat(dc, pixelFormat, &pfd);

	context = wglCreateContext(dc);
	if (context == nullptr)
	{
		return false;
	}

	wglMakeCurrent(dc, context);

	return true;
}

void contextDestroy()
{
	wglDeleteContext(context);
}
