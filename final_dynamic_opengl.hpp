/***
final_dynamic_opengl.hpp

-------------------------------------------------------------------------------
	About
-------------------------------------------------------------------------------

A open source single header file OpenGL-Loader C++ library.

This library is designed to load all the opengl functions for you so you can start right away with OpenGL up to version 4.6.
It even can create a rendering context for you, if needed.

Due to removing any kind of linking madness, all opengl functions are defined as static - so its private to this header file only!
Therefore you can use this library in one file only.

The only dependencies are built-in operating system libraries and a C++/11 compiler.

Required linking is bare minimum:
	Win32: Link to kernel32.lib
	Unix/Linux: Link to ld.so

-------------------------------------------------------------------------------
	Getting started
-------------------------------------------------------------------------------

- Drop this file into your main C++ project and include it in one place you do the rendering.
- Define FDYNGL_IMPLEMENTATION before including this header file in that translation unit.
- Load the library with fdyngl::LoadOpenGL(), while a opengl rendering context is already activated - or create a context using fdyngl::CreateOpenGLContext()
- Use all the opengl features you want
- Unload the library with fdyngl::UnloadOpenGL() when you are done
- Destroy the context when you created it using fdyngl::DestroyOpenGLContext()

-------------------------------------------------------------------------------
	Usage from a already activated rendering context
-------------------------------------------------------------------------------

#define FDYNGL_IMPLEMENTATION
#include <final_dynamic_opengl.hpp>

if (fdyngl::LoadOpenGL()) {
	// ... load shader, whatever you want to do
	fdyngl::UnloadOpenGL();
}

-------------------------------------------------------------------------------
	Usage (Without a rendering context, but with an existing window)
-------------------------------------------------------------------------------

#define FDYNGL_IMPLEMENTATION
#include <final_dynamic_opengl.hpp>

// Load opengl library without loading all the functions - functions are loaded separately later
if (fdyngl::LoadOpenGL(false)) {

	// Fill out window handle (This is platform dependent!)
	fdyngl::OpenGLContextCreationParameters contextCreationParams = {};
#	if defined(FDYNGL_PLATFORM_WIN32)
		contextCreationParams.windowHandle.win32.deviceContext = // ... pass your current device context here
		// or
		contextCreationParams.windowHandle.win32.windowHandle = // ... pass your current window handle here
#	endif

	// Create context and load opengl functions
	fdyngl::OpenGLContext glContext = {};
	if (fdyngl::CreateOpenGLContext(contextCreationParams, glContext)) {
		fdyngl::LoadOpenGLFunctions();

		// ... load shader, whatever you want to do

		fdyngl::DestroyOpenGLContext(glContext);
	}
	fdyngl::UnloadOpenGL();
}

-------------------------------------------------------------------------------
	License
-------------------------------------------------------------------------------

Final Dynamic OpenGL is released under the following license:

MIT License

Copyright (c) 2018 Torsten Spaete

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
***/

/*!
	\file final_dynamic_opengl.hpp
	\version v0.2.0.0 beta
	\author Torsten Spaete
	\brief Final Dynamic OpenGL (FDYNGL) - A open source C++ single file header OpenGL-Loader library.
*/

/*!
	\page page_changelog Changelog
	\tableofcontents

	## v0.2.0.0 beta:
	- Changed: Added parameter for controlling to load of the extensions to LoadOpenGL()
	- Fixed: Use stdint.h instead of inttypes.h
	- Fixed: WINGDIAPI detection was wrong
	- New: Added context creation support (CreateOpenGLContext, DestroyOpenGLContext)
	- New: Added LoadOpenGLFunctions()
	- New: Added PresentOpenGL()
	- New: Added GetLastError()
	- New: Written documentation

	## v0.1.0.0 beta:
	- Initial version
*/

/*!
	\page page_todo Todo
	\tableofcontents

	- [Win32] Modern context creation
	- [POSIX, GLX] Implement context creation

*/

// ****************************************************************************
//
// Header
//
// ****************************************************************************
#ifndef FDYNGL_INCLUDE_HPP
#define FDYNGL_INCLUDE_HPP

// C++/11 detection
#if !((defined(__cplusplus) && (__cplusplus >= 201103L)) || (defined(_MSC_VER) && _MSC_VER >= 1900))
#	error "You need a C++/11 compatible compiler for this library!"
#endif

//
// Platform detection
//
#if defined(_WIN32)
#	define FDYNGL_PLATFORM_WIN32
#elif defined(__linux__) || defined(__gnu_linux__) || defined(linux)
#	define FDYNGL_PLATFORM_LINUX
#	define FDYNGL_SUBPLATFORM POSIX
#	define FDYNGL_SUBPLATFORM_X11
#	define FDYNGL_SUBPLATFORM_GLX
#elif defined(__unix__) || defined(_POSIX_VERSION)
#	define FDYNGL_PLATFORM_UNIX
#	define FDYNGL_SUBPLATFORM POSIX
#	define FDYNGL_SUBPLATFORM_X11
#	define FDYNGL_SUBPLATFORM_GLX
#else
#	error "This platform/compiler is not supported!"
#endif

//
// Required api defines
//
#ifdef FDYNGL_PLATFORM_WIN32
#	ifdef APIENTRY
#		define FDYNGL_GLAPIENTRY APIENTRY
#		define FDYNGL_APIENTRY APIENTRY
#	else
#		if defined(__MINGW32__) || defined(__CYGWIN__) || (_MSC_VER >= 800) || defined(_STDCALL_SUPPORTED) || defined(__BORLANDC__)
#			define FDYNGL_APIENTRY __stdcall
#			ifndef FDYNGL_GLAPIENTRY
#				define FDYNGL_GLAPIENTRY __stdcall
#			endif
#			ifndef FDYNGL_APIENTRY
#				define FDYNGL_APIENTRY __stdcall
#			endif
#		else
#			define FDYNGL_APIENTRY
#		endif
#	endif
#	ifdef WINGDIAPI
#		define FDYNGL_WINGDIAPI WINGDIAPI
#	else
#		define FDYNGL_WINGDIAPI __declspec(dllimport)
#	endif
#	if defined(__MINGW32__) || defined(__CYGWIN__)
#		define FDYNGL_GLAPI extern
#	else
#		define FDYNGL_GLAPI FDYNGL_WINGDIAPI
#	endif
#else
#	define FDYNGL_GLAPI extern
#	define FDYNGL_APIENTRY
#endif

#if FDYNGL_AS_PRIVATE
	//! API functions exported as static
#	define fdyngl_api static
#else
	//! API functions exported as extern
#	define fdyngl_api extern
#endif

//
// Includes
//
#include <stddef.h>
#include <stdint.h>

#if defined(FDYNGL_PLATFORM_WIN32)
#	ifndef WIN32_LEAN_AND_MEAN
#			define WIN32_LEAN_AND_MEAN 1
#	endif
#	include <Windows.h>
#elif defined(FDYNGL_SUBPLATFORM_X11)
#	include<X11/X.h>
#	include<X11/Xlib.h>
#	include <GL/gl.h>
#	include <GL/glx.h>
#endif

//
// API
//

//! Core namespace
namespace fdyngl {
	//! OpenGL profile type
	enum class OpenGLProfileType {
		//! No or legacy profile
		LegacyProfile,
		//! Core profile
		CoreProfile,
		//! Compability profile
		CompabilityProfile,
	};

	//! OpenGL window handle
	struct OpenGLWindowHandle {
#if defined(FDYNGL_PLATFORM_WIN32)
		//! Win32 window
		struct {
			//! Window handle
			HWND windowHandle;
			//! Device context
			HDC deviceContext;
			//! Bool to indicate to release DC when done
			bool requireToReleaseDC;
		} win32;
#elif defined(FDYNGL_SUBPLATFORM_X11)
		//! X11 window
		struct {
			Display *display;
			Window *window;
		} x11;
#endif
	};

	//! OpenGL window handle
	struct OpenGLRenderingContext {
#if defined(FDYNGL_PLATFORM_WIN32)
		//! Win32 rendering context
		struct {
			//! Rendering context
			HGLRC renderingContext;
		} win32;
#elif defined(FDYNGL_SUBPLATFORM_GLX)
		// @TODO(final): Implement GLX context definition
		struct {
		} glx;
#endif
	};

	//! OpenGL rendering context
	struct OpenGLContext {
		//! Window handle container
		OpenGLWindowHandle windowHandle;
		//! Rendering context container
		OpenGLRenderingContext renderingContext;
		//! Is context valid
		bool isValid;
	};

	struct OpenGLContextCreationParameters {
		OpenGLWindowHandle windowHandle;
		uint32_t majorVersion;
		uint32_t minorVersion;
		OpenGLProfileType profile;
		bool forwardCompability;
	};

	inline OpenGLContextCreationParameters MakeDefaultOpenGLContextCreationParameters() {
		OpenGLContextCreationParameters result = {};
		result.majorVersion = 3;
		result.minorVersion = 3;
		result.profile = OpenGLProfileType::LegacyProfile;
		return(result);
	}

	//! Create a opengl context
	fdyngl_api bool CreateOpenGLContext(const OpenGLContextCreationParameters &contextCreationParams, OpenGLContext &outContext);

	//! Destroy the given opengl context
	fdyngl_api void DestroyOpenGLContext(OpenGLContext &context);

	//! Does all the things to get opengl up and running
	fdyngl_api bool LoadOpenGL(const bool loadFunctions = true);

	//! Releases all resources allocated for opengl
	fdyngl_api void UnloadOpenGL();

	//! Load all opengl functions
	fdyngl_api void LoadOpenGLFunctions();

	//! Presents the current frame for the given opengl context
	fdyngl_api void PresentOpenGL(const OpenGLContext &context);

	//! Returns last error string
	fdyngl_api const char *GetLastError();
};

#ifdef __cplusplus
extern "C" {
#endif

	//
	// OpenGL types and function prototypes
	// This is automatically generated by a tool, do not modify by hand!
	// Contains gl.h and glext.h stuff, reconverted into static const.
	//
#	ifndef GL_VERSION_1_1
#		define GL_VERSION_1_1 1
	static bool isGL_VERSION_1_1;
	typedef unsigned int GLenum;
	typedef unsigned int GLbitfield;
	typedef unsigned int GLuint;
	typedef int GLint;
	typedef int GLsizei;
	typedef unsigned char GLboolean;
	typedef signed char GLbyte;
	typedef short GLshort;
	typedef unsigned char GLubyte;
	typedef unsigned short GLushort;
	typedef unsigned long GLulong;
	typedef float GLfloat;
	typedef float GLclampf;
	typedef double GLdouble;
	typedef double GLclampd;
	typedef void GLvoid;
	static const uint32_t GL_ACCUM = 0x0100;
	static const uint32_t GL_LOAD = 0x0101;
	static const uint32_t GL_RETURN = 0x0102;
	static const uint32_t GL_MULT = 0x0103;
	static const uint32_t GL_ADD = 0x0104;
	static const uint32_t GL_NEVER = 0x0200;
	static const uint32_t GL_LESS = 0x0201;
	static const uint32_t GL_EQUAL = 0x0202;
	static const uint32_t GL_LEQUAL = 0x0203;
	static const uint32_t GL_GREATER = 0x0204;
	static const uint32_t GL_NOTEQUAL = 0x0205;
	static const uint32_t GL_GEQUAL = 0x0206;
	static const uint32_t GL_ALWAYS = 0x0207;
	static const uint32_t GL_CURRENT_BIT = 0x00000001;
	static const uint32_t GL_POINT_BIT = 0x00000002;
	static const uint32_t GL_LINE_BIT = 0x00000004;
	static const uint32_t GL_POLYGON_BIT = 0x00000008;
	static const uint32_t GL_POLYGON_STIPPLE_BIT = 0x00000010;
	static const uint32_t GL_PIXEL_MODE_BIT = 0x00000020;
	static const uint32_t GL_LIGHTING_BIT = 0x00000040;
	static const uint32_t GL_FOG_BIT = 0x00000080;
	static const uint32_t GL_DEPTH_BUFFER_BIT = 0x00000100;
	static const uint32_t GL_ACCUM_BUFFER_BIT = 0x00000200;
	static const uint32_t GL_STENCIL_BUFFER_BIT = 0x00000400;
	static const uint32_t GL_VIEWPORT_BIT = 0x00000800;
	static const uint32_t GL_TRANSFORM_BIT = 0x00001000;
	static const uint32_t GL_ENABLE_BIT = 0x00002000;
	static const uint32_t GL_COLOR_BUFFER_BIT = 0x00004000;
	static const uint32_t GL_HINT_BIT = 0x00008000;
	static const uint32_t GL_EVAL_BIT = 0x00010000;
	static const uint32_t GL_LIST_BIT = 0x00020000;
	static const uint32_t GL_TEXTURE_BIT = 0x00040000;
	static const uint32_t GL_SCISSOR_BIT = 0x00080000;
	static const uint32_t GL_ALL_ATTRIB_BITS = 0x000fffff;
	static const uint32_t GL_POINTS = 0x0000;
	static const uint32_t GL_LINES = 0x0001;
	static const uint32_t GL_LINE_LOOP = 0x0002;
	static const uint32_t GL_LINE_STRIP = 0x0003;
	static const uint32_t GL_TRIANGLES = 0x0004;
	static const uint32_t GL_TRIANGLE_STRIP = 0x0005;
	static const uint32_t GL_TRIANGLE_FAN = 0x0006;
	static const uint32_t GL_QUADS = 0x0007;
	static const uint32_t GL_QUAD_STRIP = 0x0008;
	static const uint32_t GL_POLYGON = 0x0009;
	static const uint32_t GL_ZERO = 0;
	static const uint32_t GL_ONE = 1;
	static const uint32_t GL_SRC_COLOR = 0x0300;
	static const uint32_t GL_ONE_MINUS_SRC_COLOR = 0x0301;
	static const uint32_t GL_SRC_ALPHA = 0x0302;
	static const uint32_t GL_ONE_MINUS_SRC_ALPHA = 0x0303;
	static const uint32_t GL_DST_ALPHA = 0x0304;
	static const uint32_t GL_ONE_MINUS_DST_ALPHA = 0x0305;
	static const uint32_t GL_DST_COLOR = 0x0306;
	static const uint32_t GL_ONE_MINUS_DST_COLOR = 0x0307;
	static const uint32_t GL_SRC_ALPHA_SATURATE = 0x0308;
	static const uint32_t GL_TRUE = 1;
	static const uint32_t GL_FALSE = 0;
	static const uint32_t GL_CLIP_PLANE0 = 0x3000;
	static const uint32_t GL_CLIP_PLANE1 = 0x3001;
	static const uint32_t GL_CLIP_PLANE2 = 0x3002;
	static const uint32_t GL_CLIP_PLANE3 = 0x3003;
	static const uint32_t GL_CLIP_PLANE4 = 0x3004;
	static const uint32_t GL_CLIP_PLANE5 = 0x3005;
	static const uint32_t GL_BYTE = 0x1400;
	static const uint32_t GL_UNSIGNED_BYTE = 0x1401;
	static const uint32_t GL_SHORT = 0x1402;
	static const uint32_t GL_UNSIGNED_SHORT = 0x1403;
	static const uint32_t GL_INT = 0x1404;
	static const uint32_t GL_UNSIGNED_INT = 0x1405;
	static const uint32_t GL_FLOAT = 0x1406;
	static const uint32_t GL_2_BYTES = 0x1407;
	static const uint32_t GL_3_BYTES = 0x1408;
	static const uint32_t GL_4_BYTES = 0x1409;
	static const uint32_t GL_DOUBLE = 0x140A;
	static const uint32_t GL_NONE = 0;
	static const uint32_t GL_FRONT_LEFT = 0x0400;
	static const uint32_t GL_FRONT_RIGHT = 0x0401;
	static const uint32_t GL_BACK_LEFT = 0x0402;
	static const uint32_t GL_BACK_RIGHT = 0x0403;
	static const uint32_t GL_FRONT = 0x0404;
	static const uint32_t GL_BACK = 0x0405;
	static const uint32_t GL_LEFT = 0x0406;
	static const uint32_t GL_RIGHT = 0x0407;
	static const uint32_t GL_FRONT_AND_BACK = 0x0408;
	static const uint32_t GL_AUX0 = 0x0409;
	static const uint32_t GL_AUX1 = 0x040A;
	static const uint32_t GL_AUX2 = 0x040B;
	static const uint32_t GL_AUX3 = 0x040C;
	static const uint32_t GL_NO_ERROR = 0;
	static const uint32_t GL_INVALID_ENUM = 0x0500;
	static const uint32_t GL_INVALID_VALUE = 0x0501;
	static const uint32_t GL_INVALID_OPERATION = 0x0502;
	static const uint32_t GL_STACK_OVERFLOW = 0x0503;
	static const uint32_t GL_STACK_UNDERFLOW = 0x0504;
	static const uint32_t GL_OUT_OF_MEMORY = 0x0505;
	static const uint32_t GL_2D = 0x0600;
	static const uint32_t GL_3D = 0x0601;
	static const uint32_t GL_3D_COLOR = 0x0602;
	static const uint32_t GL_3D_COLOR_TEXTURE = 0x0603;
	static const uint32_t GL_4D_COLOR_TEXTURE = 0x0604;
	static const uint32_t GL_PASS_THROUGH_TOKEN = 0x0700;
	static const uint32_t GL_POINT_TOKEN = 0x0701;
	static const uint32_t GL_LINE_TOKEN = 0x0702;
	static const uint32_t GL_POLYGON_TOKEN = 0x0703;
	static const uint32_t GL_BITMAP_TOKEN = 0x0704;
	static const uint32_t GL_DRAW_PIXEL_TOKEN = 0x0705;
	static const uint32_t GL_COPY_PIXEL_TOKEN = 0x0706;
	static const uint32_t GL_LINE_RESET_TOKEN = 0x0707;
	static const uint32_t GL_EXP = 0x0800;
	static const uint32_t GL_EXP2 = 0x0801;
	static const uint32_t GL_CW = 0x0900;
	static const uint32_t GL_CCW = 0x0901;
	static const uint32_t GL_COEFF = 0x0A00;
	static const uint32_t GL_ORDER = 0x0A01;
	static const uint32_t GL_DOMAIN = 0x0A02;
	static const uint32_t GL_CURRENT_COLOR = 0x0B00;
	static const uint32_t GL_CURRENT_INDEX = 0x0B01;
	static const uint32_t GL_CURRENT_NORMAL = 0x0B02;
	static const uint32_t GL_CURRENT_TEXTURE_COORDS = 0x0B03;
	static const uint32_t GL_CURRENT_RASTER_COLOR = 0x0B04;
	static const uint32_t GL_CURRENT_RASTER_INDEX = 0x0B05;
	static const uint32_t GL_CURRENT_RASTER_TEXTURE_COORDS = 0x0B06;
	static const uint32_t GL_CURRENT_RASTER_POSITION = 0x0B07;
	static const uint32_t GL_CURRENT_RASTER_POSITION_VALID = 0x0B08;
	static const uint32_t GL_CURRENT_RASTER_DISTANCE = 0x0B09;
	static const uint32_t GL_POINT_SMOOTH = 0x0B10;
	static const uint32_t GL_POINT_SIZE = 0x0B11;
	static const uint32_t GL_POINT_SIZE_RANGE = 0x0B12;
	static const uint32_t GL_POINT_SIZE_GRANULARITY = 0x0B13;
	static const uint32_t GL_LINE_SMOOTH = 0x0B20;
	static const uint32_t GL_LINE_WIDTH = 0x0B21;
	static const uint32_t GL_LINE_WIDTH_RANGE = 0x0B22;
	static const uint32_t GL_LINE_WIDTH_GRANULARITY = 0x0B23;
	static const uint32_t GL_LINE_STIPPLE = 0x0B24;
	static const uint32_t GL_LINE_STIPPLE_PATTERN = 0x0B25;
	static const uint32_t GL_LINE_STIPPLE_REPEAT = 0x0B26;
	static const uint32_t GL_LIST_MODE = 0x0B30;
	static const uint32_t GL_MAX_LIST_NESTING = 0x0B31;
	static const uint32_t GL_LIST_BASE = 0x0B32;
	static const uint32_t GL_LIST_INDEX = 0x0B33;
	static const uint32_t GL_POLYGON_MODE = 0x0B40;
	static const uint32_t GL_POLYGON_SMOOTH = 0x0B41;
	static const uint32_t GL_POLYGON_STIPPLE = 0x0B42;
	static const uint32_t GL_EDGE_FLAG = 0x0B43;
	static const uint32_t GL_CULL_FACE = 0x0B44;
	static const uint32_t GL_CULL_FACE_MODE = 0x0B45;
	static const uint32_t GL_FRONT_FACE = 0x0B46;
	static const uint32_t GL_LIGHTING = 0x0B50;
	static const uint32_t GL_LIGHT_MODEL_LOCAL_VIEWER = 0x0B51;
	static const uint32_t GL_LIGHT_MODEL_TWO_SIDE = 0x0B52;
	static const uint32_t GL_LIGHT_MODEL_AMBIENT = 0x0B53;
	static const uint32_t GL_SHADE_MODEL = 0x0B54;
	static const uint32_t GL_COLOR_MATERIAL_FACE = 0x0B55;
	static const uint32_t GL_COLOR_MATERIAL_PARAMETER = 0x0B56;
	static const uint32_t GL_COLOR_MATERIAL = 0x0B57;
	static const uint32_t GL_FOG = 0x0B60;
	static const uint32_t GL_FOG_INDEX = 0x0B61;
	static const uint32_t GL_FOG_DENSITY = 0x0B62;
	static const uint32_t GL_FOG_START = 0x0B63;
	static const uint32_t GL_FOG_END = 0x0B64;
	static const uint32_t GL_FOG_MODE = 0x0B65;
	static const uint32_t GL_FOG_COLOR = 0x0B66;
	static const uint32_t GL_DEPTH_RANGE = 0x0B70;
	static const uint32_t GL_DEPTH_TEST = 0x0B71;
	static const uint32_t GL_DEPTH_WRITEMASK = 0x0B72;
	static const uint32_t GL_DEPTH_CLEAR_VALUE = 0x0B73;
	static const uint32_t GL_DEPTH_FUNC = 0x0B74;
	static const uint32_t GL_ACCUM_CLEAR_VALUE = 0x0B80;
	static const uint32_t GL_STENCIL_TEST = 0x0B90;
	static const uint32_t GL_STENCIL_CLEAR_VALUE = 0x0B91;
	static const uint32_t GL_STENCIL_FUNC = 0x0B92;
	static const uint32_t GL_STENCIL_VALUE_MASK = 0x0B93;
	static const uint32_t GL_STENCIL_FAIL = 0x0B94;
	static const uint32_t GL_STENCIL_PASS_DEPTH_FAIL = 0x0B95;
	static const uint32_t GL_STENCIL_PASS_DEPTH_PASS = 0x0B96;
	static const uint32_t GL_STENCIL_REF = 0x0B97;
	static const uint32_t GL_STENCIL_WRITEMASK = 0x0B98;
	static const uint32_t GL_MATRIX_MODE = 0x0BA0;
	static const uint32_t GL_NORMALIZE = 0x0BA1;
	static const uint32_t GL_VIEWPORT = 0x0BA2;
	static const uint32_t GL_MODELVIEW_STACK_DEPTH = 0x0BA3;
	static const uint32_t GL_PROJECTION_STACK_DEPTH = 0x0BA4;
	static const uint32_t GL_TEXTURE_STACK_DEPTH = 0x0BA5;
	static const uint32_t GL_MODELVIEW_MATRIX = 0x0BA6;
	static const uint32_t GL_PROJECTION_MATRIX = 0x0BA7;
	static const uint32_t GL_TEXTURE_MATRIX = 0x0BA8;
	static const uint32_t GL_ATTRIB_STACK_DEPTH = 0x0BB0;
	static const uint32_t GL_CLIENT_ATTRIB_STACK_DEPTH = 0x0BB1;
	static const uint32_t GL_ALPHA_TEST = 0x0BC0;
	static const uint32_t GL_ALPHA_TEST_FUNC = 0x0BC1;
	static const uint32_t GL_ALPHA_TEST_REF = 0x0BC2;
	static const uint32_t GL_DITHER = 0x0BD0;
	static const uint32_t GL_BLEND_DST = 0x0BE0;
	static const uint32_t GL_BLEND_SRC = 0x0BE1;
	static const uint32_t GL_BLEND = 0x0BE2;
	static const uint32_t GL_LOGIC_OP_MODE = 0x0BF0;
	static const uint32_t GL_INDEX_LOGIC_OP = 0x0BF1;
	static const uint32_t GL_COLOR_LOGIC_OP = 0x0BF2;
	static const uint32_t GL_AUX_BUFFERS = 0x0C00;
	static const uint32_t GL_DRAW_BUFFER = 0x0C01;
	static const uint32_t GL_READ_BUFFER = 0x0C02;
	static const uint32_t GL_SCISSOR_BOX = 0x0C10;
	static const uint32_t GL_SCISSOR_TEST = 0x0C11;
	static const uint32_t GL_INDEX_CLEAR_VALUE = 0x0C20;
	static const uint32_t GL_INDEX_WRITEMASK = 0x0C21;
	static const uint32_t GL_COLOR_CLEAR_VALUE = 0x0C22;
	static const uint32_t GL_COLOR_WRITEMASK = 0x0C23;
	static const uint32_t GL_INDEX_MODE = 0x0C30;
	static const uint32_t GL_RGBA_MODE = 0x0C31;
	static const uint32_t GL_DOUBLEBUFFER = 0x0C32;
	static const uint32_t GL_STEREO = 0x0C33;
	static const uint32_t GL_RENDER_MODE = 0x0C40;
	static const uint32_t GL_PERSPECTIVE_CORRECTION_HINT = 0x0C50;
	static const uint32_t GL_POINT_SMOOTH_HINT = 0x0C51;
	static const uint32_t GL_LINE_SMOOTH_HINT = 0x0C52;
	static const uint32_t GL_POLYGON_SMOOTH_HINT = 0x0C53;
	static const uint32_t GL_FOG_HINT = 0x0C54;
	static const uint32_t GL_TEXTURE_GEN_S = 0x0C60;
	static const uint32_t GL_TEXTURE_GEN_T = 0x0C61;
	static const uint32_t GL_TEXTURE_GEN_R = 0x0C62;
	static const uint32_t GL_TEXTURE_GEN_Q = 0x0C63;
	static const uint32_t GL_PIXEL_MAP_I_TO_I = 0x0C70;
	static const uint32_t GL_PIXEL_MAP_S_TO_S = 0x0C71;
	static const uint32_t GL_PIXEL_MAP_I_TO_R = 0x0C72;
	static const uint32_t GL_PIXEL_MAP_I_TO_G = 0x0C73;
	static const uint32_t GL_PIXEL_MAP_I_TO_B = 0x0C74;
	static const uint32_t GL_PIXEL_MAP_I_TO_A = 0x0C75;
	static const uint32_t GL_PIXEL_MAP_R_TO_R = 0x0C76;
	static const uint32_t GL_PIXEL_MAP_G_TO_G = 0x0C77;
	static const uint32_t GL_PIXEL_MAP_B_TO_B = 0x0C78;
	static const uint32_t GL_PIXEL_MAP_A_TO_A = 0x0C79;
	static const uint32_t GL_PIXEL_MAP_I_TO_I_SIZE = 0x0CB0;
	static const uint32_t GL_PIXEL_MAP_S_TO_S_SIZE = 0x0CB1;
	static const uint32_t GL_PIXEL_MAP_I_TO_R_SIZE = 0x0CB2;
	static const uint32_t GL_PIXEL_MAP_I_TO_G_SIZE = 0x0CB3;
	static const uint32_t GL_PIXEL_MAP_I_TO_B_SIZE = 0x0CB4;
	static const uint32_t GL_PIXEL_MAP_I_TO_A_SIZE = 0x0CB5;
	static const uint32_t GL_PIXEL_MAP_R_TO_R_SIZE = 0x0CB6;
	static const uint32_t GL_PIXEL_MAP_G_TO_G_SIZE = 0x0CB7;
	static const uint32_t GL_PIXEL_MAP_B_TO_B_SIZE = 0x0CB8;
	static const uint32_t GL_PIXEL_MAP_A_TO_A_SIZE = 0x0CB9;
	static const uint32_t GL_UNPACK_SWAP_BYTES = 0x0CF0;
	static const uint32_t GL_UNPACK_LSB_FIRST = 0x0CF1;
	static const uint32_t GL_UNPACK_ROW_LENGTH = 0x0CF2;
	static const uint32_t GL_UNPACK_SKIP_ROWS = 0x0CF3;
	static const uint32_t GL_UNPACK_SKIP_PIXELS = 0x0CF4;
	static const uint32_t GL_UNPACK_ALIGNMENT = 0x0CF5;
	static const uint32_t GL_PACK_SWAP_BYTES = 0x0D00;
	static const uint32_t GL_PACK_LSB_FIRST = 0x0D01;
	static const uint32_t GL_PACK_ROW_LENGTH = 0x0D02;
	static const uint32_t GL_PACK_SKIP_ROWS = 0x0D03;
	static const uint32_t GL_PACK_SKIP_PIXELS = 0x0D04;
	static const uint32_t GL_PACK_ALIGNMENT = 0x0D05;
	static const uint32_t GL_MAP_COLOR = 0x0D10;
	static const uint32_t GL_MAP_STENCIL = 0x0D11;
	static const uint32_t GL_INDEX_SHIFT = 0x0D12;
	static const uint32_t GL_INDEX_OFFSET = 0x0D13;
	static const uint32_t GL_RED_SCALE = 0x0D14;
	static const uint32_t GL_RED_BIAS = 0x0D15;
	static const uint32_t GL_ZOOM_X = 0x0D16;
	static const uint32_t GL_ZOOM_Y = 0x0D17;
	static const uint32_t GL_GREEN_SCALE = 0x0D18;
	static const uint32_t GL_GREEN_BIAS = 0x0D19;
	static const uint32_t GL_BLUE_SCALE = 0x0D1A;
	static const uint32_t GL_BLUE_BIAS = 0x0D1B;
	static const uint32_t GL_ALPHA_SCALE = 0x0D1C;
	static const uint32_t GL_ALPHA_BIAS = 0x0D1D;
	static const uint32_t GL_DEPTH_SCALE = 0x0D1E;
	static const uint32_t GL_DEPTH_BIAS = 0x0D1F;
	static const uint32_t GL_MAX_EVAL_ORDER = 0x0D30;
	static const uint32_t GL_MAX_LIGHTS = 0x0D31;
	static const uint32_t GL_MAX_CLIP_PLANES = 0x0D32;
	static const uint32_t GL_MAX_TEXTURE_SIZE = 0x0D33;
	static const uint32_t GL_MAX_PIXEL_MAP_TABLE = 0x0D34;
	static const uint32_t GL_MAX_ATTRIB_STACK_DEPTH = 0x0D35;
	static const uint32_t GL_MAX_MODELVIEW_STACK_DEPTH = 0x0D36;
	static const uint32_t GL_MAX_NAME_STACK_DEPTH = 0x0D37;
	static const uint32_t GL_MAX_PROJECTION_STACK_DEPTH = 0x0D38;
	static const uint32_t GL_MAX_TEXTURE_STACK_DEPTH = 0x0D39;
	static const uint32_t GL_MAX_VIEWPORT_DIMS = 0x0D3A;
	static const uint32_t GL_MAX_CLIENT_ATTRIB_STACK_DEPTH = 0x0D3B;
	static const uint32_t GL_SUBPIXEL_BITS = 0x0D50;
	static const uint32_t GL_INDEX_BITS = 0x0D51;
	static const uint32_t GL_RED_BITS = 0x0D52;
	static const uint32_t GL_GREEN_BITS = 0x0D53;
	static const uint32_t GL_BLUE_BITS = 0x0D54;
	static const uint32_t GL_ALPHA_BITS = 0x0D55;
	static const uint32_t GL_DEPTH_BITS = 0x0D56;
	static const uint32_t GL_STENCIL_BITS = 0x0D57;
	static const uint32_t GL_ACCUM_RED_BITS = 0x0D58;
	static const uint32_t GL_ACCUM_GREEN_BITS = 0x0D59;
	static const uint32_t GL_ACCUM_BLUE_BITS = 0x0D5A;
	static const uint32_t GL_ACCUM_ALPHA_BITS = 0x0D5B;
	static const uint32_t GL_NAME_STACK_DEPTH = 0x0D70;
	static const uint32_t GL_AUTO_NORMAL = 0x0D80;
	static const uint32_t GL_MAP1_COLOR_4 = 0x0D90;
	static const uint32_t GL_MAP1_INDEX = 0x0D91;
	static const uint32_t GL_MAP1_NORMAL = 0x0D92;
	static const uint32_t GL_MAP1_TEXTURE_COORD_1 = 0x0D93;
	static const uint32_t GL_MAP1_TEXTURE_COORD_2 = 0x0D94;
	static const uint32_t GL_MAP1_TEXTURE_COORD_3 = 0x0D95;
	static const uint32_t GL_MAP1_TEXTURE_COORD_4 = 0x0D96;
	static const uint32_t GL_MAP1_VERTEX_3 = 0x0D97;
	static const uint32_t GL_MAP1_VERTEX_4 = 0x0D98;
	static const uint32_t GL_MAP2_COLOR_4 = 0x0DB0;
	static const uint32_t GL_MAP2_INDEX = 0x0DB1;
	static const uint32_t GL_MAP2_NORMAL = 0x0DB2;
	static const uint32_t GL_MAP2_TEXTURE_COORD_1 = 0x0DB3;
	static const uint32_t GL_MAP2_TEXTURE_COORD_2 = 0x0DB4;
	static const uint32_t GL_MAP2_TEXTURE_COORD_3 = 0x0DB5;
	static const uint32_t GL_MAP2_TEXTURE_COORD_4 = 0x0DB6;
	static const uint32_t GL_MAP2_VERTEX_3 = 0x0DB7;
	static const uint32_t GL_MAP2_VERTEX_4 = 0x0DB8;
	static const uint32_t GL_MAP1_GRID_DOMAIN = 0x0DD0;
	static const uint32_t GL_MAP1_GRID_SEGMENTS = 0x0DD1;
	static const uint32_t GL_MAP2_GRID_DOMAIN = 0x0DD2;
	static const uint32_t GL_MAP2_GRID_SEGMENTS = 0x0DD3;
	static const uint32_t GL_TEXTURE_1D = 0x0DE0;
	static const uint32_t GL_TEXTURE_2D = 0x0DE1;
	static const uint32_t GL_FEEDBACK_BUFFER_POINTER = 0x0DF0;
	static const uint32_t GL_FEEDBACK_BUFFER_SIZE = 0x0DF1;
	static const uint32_t GL_FEEDBACK_BUFFER_TYPE = 0x0DF2;
	static const uint32_t GL_SELECTION_BUFFER_POINTER = 0x0DF3;
	static const uint32_t GL_SELECTION_BUFFER_SIZE = 0x0DF4;
	static const uint32_t GL_TEXTURE_WIDTH = 0x1000;
	static const uint32_t GL_TEXTURE_HEIGHT = 0x1001;
	static const uint32_t GL_TEXTURE_INTERNAL_FORMAT = 0x1003;
	static const uint32_t GL_TEXTURE_BORDER_COLOR = 0x1004;
	static const uint32_t GL_TEXTURE_BORDER = 0x1005;
	static const uint32_t GL_DONT_CARE = 0x1100;
	static const uint32_t GL_FASTEST = 0x1101;
	static const uint32_t GL_NICEST = 0x1102;
	static const uint32_t GL_LIGHT0 = 0x4000;
	static const uint32_t GL_LIGHT1 = 0x4001;
	static const uint32_t GL_LIGHT2 = 0x4002;
	static const uint32_t GL_LIGHT3 = 0x4003;
	static const uint32_t GL_LIGHT4 = 0x4004;
	static const uint32_t GL_LIGHT5 = 0x4005;
	static const uint32_t GL_LIGHT6 = 0x4006;
	static const uint32_t GL_LIGHT7 = 0x4007;
	static const uint32_t GL_AMBIENT = 0x1200;
	static const uint32_t GL_DIFFUSE = 0x1201;
	static const uint32_t GL_SPECULAR = 0x1202;
	static const uint32_t GL_POSITION = 0x1203;
	static const uint32_t GL_SPOT_DIRECTION = 0x1204;
	static const uint32_t GL_SPOT_EXPONENT = 0x1205;
	static const uint32_t GL_SPOT_CUTOFF = 0x1206;
	static const uint32_t GL_CONSTANT_ATTENUATION = 0x1207;
	static const uint32_t GL_LINEAR_ATTENUATION = 0x1208;
	static const uint32_t GL_QUADRATIC_ATTENUATION = 0x1209;
	static const uint32_t GL_COMPILE = 0x1300;
	static const uint32_t GL_COMPILE_AND_EXECUTE = 0x1301;
	static const uint32_t GL_CLEAR = 0x1500;
	static const uint32_t GL_AND = 0x1501;
	static const uint32_t GL_AND_REVERSE = 0x1502;
	static const uint32_t GL_COPY = 0x1503;
	static const uint32_t GL_AND_INVERTED = 0x1504;
	static const uint32_t GL_NOOP = 0x1505;
	static const uint32_t GL_XOR = 0x1506;
	static const uint32_t GL_OR = 0x1507;
	static const uint32_t GL_NOR = 0x1508;
	static const uint32_t GL_EQUIV = 0x1509;
	static const uint32_t GL_INVERT = 0x150A;
	static const uint32_t GL_OR_REVERSE = 0x150B;
	static const uint32_t GL_COPY_INVERTED = 0x150C;
	static const uint32_t GL_OR_INVERTED = 0x150D;
	static const uint32_t GL_NAND = 0x150E;
	static const uint32_t GL_SET = 0x150F;
	static const uint32_t GL_EMISSION = 0x1600;
	static const uint32_t GL_SHININESS = 0x1601;
	static const uint32_t GL_AMBIENT_AND_DIFFUSE = 0x1602;
	static const uint32_t GL_COLOR_INDEXES = 0x1603;
	static const uint32_t GL_MODELVIEW = 0x1700;
	static const uint32_t GL_PROJECTION = 0x1701;
	static const uint32_t GL_TEXTURE = 0x1702;
	static const uint32_t GL_COLOR = 0x1800;
	static const uint32_t GL_DEPTH = 0x1801;
	static const uint32_t GL_STENCIL = 0x1802;
	static const uint32_t GL_COLOR_INDEX = 0x1900;
	static const uint32_t GL_STENCIL_INDEX = 0x1901;
	static const uint32_t GL_DEPTH_COMPONENT = 0x1902;
	static const uint32_t GL_RED = 0x1903;
	static const uint32_t GL_GREEN = 0x1904;
	static const uint32_t GL_BLUE = 0x1905;
	static const uint32_t GL_ALPHA = 0x1906;
	static const uint32_t GL_RGB = 0x1907;
	static const uint32_t GL_RGBA = 0x1908;
	static const uint32_t GL_LUMINANCE = 0x1909;
	static const uint32_t GL_LUMINANCE_ALPHA = 0x190A;
	static const uint32_t GL_BITMAP = 0x1A00;
	static const uint32_t GL_POINT = 0x1B00;
	static const uint32_t GL_LINE = 0x1B01;
	static const uint32_t GL_FILL = 0x1B02;
	static const uint32_t GL_RENDER = 0x1C00;
	static const uint32_t GL_FEEDBACK = 0x1C01;
	static const uint32_t GL_SELECT = 0x1C02;
	static const uint32_t GL_FLAT = 0x1D00;
	static const uint32_t GL_SMOOTH = 0x1D01;
	static const uint32_t GL_KEEP = 0x1E00;
	static const uint32_t GL_REPLACE = 0x1E01;
	static const uint32_t GL_INCR = 0x1E02;
	static const uint32_t GL_DECR = 0x1E03;
	static const uint32_t GL_VENDOR = 0x1F00;
	static const uint32_t GL_RENDERER = 0x1F01;
	static const uint32_t GL_VERSION = 0x1F02;
	static const uint32_t GL_EXTENSIONS = 0x1F03;
	static const uint32_t GL_S = 0x2000;
	static const uint32_t GL_T = 0x2001;
	static const uint32_t GL_R = 0x2002;
	static const uint32_t GL_Q = 0x2003;
	static const uint32_t GL_MODULATE = 0x2100;
	static const uint32_t GL_DECAL = 0x2101;
	static const uint32_t GL_TEXTURE_ENV_MODE = 0x2200;
	static const uint32_t GL_TEXTURE_ENV_COLOR = 0x2201;
	static const uint32_t GL_TEXTURE_ENV = 0x2300;
	static const uint32_t GL_EYE_LINEAR = 0x2400;
	static const uint32_t GL_OBJECT_LINEAR = 0x2401;
	static const uint32_t GL_SPHERE_MAP = 0x2402;
	static const uint32_t GL_TEXTURE_GEN_MODE = 0x2500;
	static const uint32_t GL_OBJECT_PLANE = 0x2501;
	static const uint32_t GL_EYE_PLANE = 0x2502;
	static const uint32_t GL_NEAREST = 0x2600;
	static const uint32_t GL_LINEAR = 0x2601;
	static const uint32_t GL_NEAREST_MIPMAP_NEAREST = 0x2700;
	static const uint32_t GL_LINEAR_MIPMAP_NEAREST = 0x2701;
	static const uint32_t GL_NEAREST_MIPMAP_LINEAR = 0x2702;
	static const uint32_t GL_LINEAR_MIPMAP_LINEAR = 0x2703;
	static const uint32_t GL_TEXTURE_MAG_FILTER = 0x2800;
	static const uint32_t GL_TEXTURE_MIN_FILTER = 0x2801;
	static const uint32_t GL_TEXTURE_WRAP_S = 0x2802;
	static const uint32_t GL_TEXTURE_WRAP_T = 0x2803;
	static const uint32_t GL_CLAMP = 0x2900;
	static const uint32_t GL_REPEAT = 0x2901;
	static const uint32_t GL_CLIENT_PIXEL_STORE_BIT = 0x00000001;
	static const uint32_t GL_CLIENT_VERTEX_ARRAY_BIT = 0x00000002;
	static const uint32_t GL_CLIENT_ALL_ATTRIB_BITS = 0xffffffff;
	static const uint32_t GL_POLYGON_OFFSET_FACTOR = 0x8038;
	static const uint32_t GL_POLYGON_OFFSET_UNITS = 0x2A00;
	static const uint32_t GL_POLYGON_OFFSET_POINT = 0x2A01;
	static const uint32_t GL_POLYGON_OFFSET_LINE = 0x2A02;
	static const uint32_t GL_POLYGON_OFFSET_FILL = 0x8037;
	static const uint32_t GL_ALPHA4 = 0x803B;
	static const uint32_t GL_ALPHA8 = 0x803C;
	static const uint32_t GL_ALPHA12 = 0x803D;
	static const uint32_t GL_ALPHA16 = 0x803E;
	static const uint32_t GL_LUMINANCE4 = 0x803F;
	static const uint32_t GL_LUMINANCE8 = 0x8040;
	static const uint32_t GL_LUMINANCE12 = 0x8041;
	static const uint32_t GL_LUMINANCE16 = 0x8042;
	static const uint32_t GL_LUMINANCE4_ALPHA4 = 0x8043;
	static const uint32_t GL_LUMINANCE6_ALPHA2 = 0x8044;
	static const uint32_t GL_LUMINANCE8_ALPHA8 = 0x8045;
	static const uint32_t GL_LUMINANCE12_ALPHA4 = 0x8046;
	static const uint32_t GL_LUMINANCE12_ALPHA12 = 0x8047;
	static const uint32_t GL_LUMINANCE16_ALPHA16 = 0x8048;
	static const uint32_t GL_INTENSITY = 0x8049;
	static const uint32_t GL_INTENSITY4 = 0x804A;
	static const uint32_t GL_INTENSITY8 = 0x804B;
	static const uint32_t GL_INTENSITY12 = 0x804C;
	static const uint32_t GL_INTENSITY16 = 0x804D;
	static const uint32_t GL_R3_G3_B2 = 0x2A10;
	static const uint32_t GL_RGB4 = 0x804F;
	static const uint32_t GL_RGB5 = 0x8050;
	static const uint32_t GL_RGB8 = 0x8051;
	static const uint32_t GL_RGB10 = 0x8052;
	static const uint32_t GL_RGB12 = 0x8053;
	static const uint32_t GL_RGB16 = 0x8054;
	static const uint32_t GL_RGBA2 = 0x8055;
	static const uint32_t GL_RGBA4 = 0x8056;
	static const uint32_t GL_RGB5_A1 = 0x8057;
	static const uint32_t GL_RGBA8 = 0x8058;
	static const uint32_t GL_RGB10_A2 = 0x8059;
	static const uint32_t GL_RGBA12 = 0x805A;
	static const uint32_t GL_RGBA16 = 0x805B;
	static const uint32_t GL_TEXTURE_RED_SIZE = 0x805C;
	static const uint32_t GL_TEXTURE_GREEN_SIZE = 0x805D;
	static const uint32_t GL_TEXTURE_BLUE_SIZE = 0x805E;
	static const uint32_t GL_TEXTURE_ALPHA_SIZE = 0x805F;
	static const uint32_t GL_TEXTURE_LUMINANCE_SIZE = 0x8060;
	static const uint32_t GL_TEXTURE_INTENSITY_SIZE = 0x8061;
	static const uint32_t GL_PROXY_TEXTURE_1D = 0x8063;
	static const uint32_t GL_PROXY_TEXTURE_2D = 0x8064;
	static const uint32_t GL_TEXTURE_PRIORITY = 0x8066;
	static const uint32_t GL_TEXTURE_RESIDENT = 0x8067;
	static const uint32_t GL_TEXTURE_BINDING_1D = 0x8068;
	static const uint32_t GL_TEXTURE_BINDING_2D = 0x8069;
	static const uint32_t GL_VERTEX_ARRAY = 0x8074;
	static const uint32_t GL_NORMAL_ARRAY = 0x8075;
	static const uint32_t GL_COLOR_ARRAY = 0x8076;
	static const uint32_t GL_INDEX_ARRAY = 0x8077;
	static const uint32_t GL_TEXTURE_COORD_ARRAY = 0x8078;
	static const uint32_t GL_EDGE_FLAG_ARRAY = 0x8079;
	static const uint32_t GL_VERTEX_ARRAY_SIZE = 0x807A;
	static const uint32_t GL_VERTEX_ARRAY_TYPE = 0x807B;
	static const uint32_t GL_VERTEX_ARRAY_STRIDE = 0x807C;
	static const uint32_t GL_NORMAL_ARRAY_TYPE = 0x807E;
	static const uint32_t GL_NORMAL_ARRAY_STRIDE = 0x807F;
	static const uint32_t GL_COLOR_ARRAY_SIZE = 0x8081;
	static const uint32_t GL_COLOR_ARRAY_TYPE = 0x8082;
	static const uint32_t GL_COLOR_ARRAY_STRIDE = 0x8083;
	static const uint32_t GL_INDEX_ARRAY_TYPE = 0x8085;
	static const uint32_t GL_INDEX_ARRAY_STRIDE = 0x8086;
	static const uint32_t GL_TEXTURE_COORD_ARRAY_SIZE = 0x8088;
	static const uint32_t GL_TEXTURE_COORD_ARRAY_TYPE = 0x8089;
	static const uint32_t GL_TEXTURE_COORD_ARRAY_STRIDE = 0x808A;
	static const uint32_t GL_EDGE_FLAG_ARRAY_STRIDE = 0x808C;
	static const uint32_t GL_VERTEX_ARRAY_POINTER = 0x808E;
	static const uint32_t GL_NORMAL_ARRAY_POINTER = 0x808F;
	static const uint32_t GL_COLOR_ARRAY_POINTER = 0x8090;
	static const uint32_t GL_INDEX_ARRAY_POINTER = 0x8091;
	static const uint32_t GL_TEXTURE_COORD_ARRAY_POINTER = 0x8092;
	static const uint32_t GL_EDGE_FLAG_ARRAY_POINTER = 0x8093;
	static const uint32_t GL_V2F = 0x2A20;
	static const uint32_t GL_V3F = 0x2A21;
	static const uint32_t GL_C4UB_V2F = 0x2A22;
	static const uint32_t GL_C4UB_V3F = 0x2A23;
	static const uint32_t GL_C3F_V3F = 0x2A24;
	static const uint32_t GL_N3F_V3F = 0x2A25;
	static const uint32_t GL_C4F_N3F_V3F = 0x2A26;
	static const uint32_t GL_T2F_V3F = 0x2A27;
	static const uint32_t GL_T4F_V4F = 0x2A28;
	static const uint32_t GL_T2F_C4UB_V3F = 0x2A29;
	static const uint32_t GL_T2F_C3F_V3F = 0x2A2A;
	static const uint32_t GL_T2F_N3F_V3F = 0x2A2B;
	static const uint32_t GL_T2F_C4F_N3F_V3F = 0x2A2C;
	static const uint32_t GL_T4F_C4F_N3F_V4F = 0x2A2D;
	static const uint32_t GL_EXT_vertex_array = 1;
	static const uint32_t GL_EXT_bgra = 1;
	static const uint32_t GL_EXT_paletted_texture = 1;
	static const uint32_t GL_WIN_swap_hint = 1;
	static const uint32_t GL_WIN_draw_range_elements = 1;
	static const uint32_t GL_WIN_phong_shading = 1;
	static const uint32_t GL_WIN_specular_fog = 1;
	static const uint32_t GL_VERTEX_ARRAY_EXT = 0x8074;
	static const uint32_t GL_NORMAL_ARRAY_EXT = 0x8075;
	static const uint32_t GL_COLOR_ARRAY_EXT = 0x8076;
	static const uint32_t GL_INDEX_ARRAY_EXT = 0x8077;
	static const uint32_t GL_TEXTURE_COORD_ARRAY_EXT = 0x8078;
	static const uint32_t GL_EDGE_FLAG_ARRAY_EXT = 0x8079;
	static const uint32_t GL_VERTEX_ARRAY_SIZE_EXT = 0x807A;
	static const uint32_t GL_VERTEX_ARRAY_TYPE_EXT = 0x807B;
	static const uint32_t GL_VERTEX_ARRAY_STRIDE_EXT = 0x807C;
	static const uint32_t GL_VERTEX_ARRAY_COUNT_EXT = 0x807D;
	static const uint32_t GL_NORMAL_ARRAY_TYPE_EXT = 0x807E;
	static const uint32_t GL_NORMAL_ARRAY_STRIDE_EXT = 0x807F;
	static const uint32_t GL_NORMAL_ARRAY_COUNT_EXT = 0x8080;
	static const uint32_t GL_COLOR_ARRAY_SIZE_EXT = 0x8081;
	static const uint32_t GL_COLOR_ARRAY_TYPE_EXT = 0x8082;
	static const uint32_t GL_COLOR_ARRAY_STRIDE_EXT = 0x8083;
	static const uint32_t GL_COLOR_ARRAY_COUNT_EXT = 0x8084;
	static const uint32_t GL_INDEX_ARRAY_TYPE_EXT = 0x8085;
	static const uint32_t GL_INDEX_ARRAY_STRIDE_EXT = 0x8086;
	static const uint32_t GL_INDEX_ARRAY_COUNT_EXT = 0x8087;
	static const uint32_t GL_TEXTURE_COORD_ARRAY_SIZE_EXT = 0x8088;
	static const uint32_t GL_TEXTURE_COORD_ARRAY_TYPE_EXT = 0x8089;
	static const uint32_t GL_TEXTURE_COORD_ARRAY_STRIDE_EXT = 0x808A;
	static const uint32_t GL_TEXTURE_COORD_ARRAY_COUNT_EXT = 0x808B;
	static const uint32_t GL_EDGE_FLAG_ARRAY_STRIDE_EXT = 0x808C;
	static const uint32_t GL_EDGE_FLAG_ARRAY_COUNT_EXT = 0x808D;
	static const uint32_t GL_VERTEX_ARRAY_POINTER_EXT = 0x808E;
	static const uint32_t GL_NORMAL_ARRAY_POINTER_EXT = 0x808F;
	static const uint32_t GL_COLOR_ARRAY_POINTER_EXT = 0x8090;
	static const uint32_t GL_INDEX_ARRAY_POINTER_EXT = 0x8091;
	static const uint32_t GL_TEXTURE_COORD_ARRAY_POINTER_EXT = 0x8092;
	static const uint32_t GL_EDGE_FLAG_ARRAY_POINTER_EXT = 0x8093;
	static const uint32_t GL_DOUBLE_EXT = GL_DOUBLE;
	static const uint32_t GL_BGR_EXT = 0x80E0;
	static const uint32_t GL_BGRA_EXT = 0x80E1;
	static const uint32_t GL_COLOR_TABLE_FORMAT_EXT = 0x80D8;
	static const uint32_t GL_COLOR_TABLE_WIDTH_EXT = 0x80D9;
	static const uint32_t GL_COLOR_TABLE_RED_SIZE_EXT = 0x80DA;
	static const uint32_t GL_COLOR_TABLE_GREEN_SIZE_EXT = 0x80DB;
	static const uint32_t GL_COLOR_TABLE_BLUE_SIZE_EXT = 0x80DC;
	static const uint32_t GL_COLOR_TABLE_ALPHA_SIZE_EXT = 0x80DD;
	static const uint32_t GL_COLOR_TABLE_LUMINANCE_SIZE_EXT = 0x80DE;
	static const uint32_t GL_COLOR_TABLE_INTENSITY_SIZE_EXT = 0x80DF;
	static const uint32_t GL_COLOR_INDEX1_EXT = 0x80E2;
	static const uint32_t GL_COLOR_INDEX2_EXT = 0x80E3;
	static const uint32_t GL_COLOR_INDEX4_EXT = 0x80E4;
	static const uint32_t GL_COLOR_INDEX8_EXT = 0x80E5;
	static const uint32_t GL_COLOR_INDEX12_EXT = 0x80E6;
	static const uint32_t GL_COLOR_INDEX16_EXT = 0x80E7;
	static const uint32_t GL_MAX_ELEMENTS_VERTICES_WIN = 0x80E8;
	static const uint32_t GL_MAX_ELEMENTS_INDICES_WIN = 0x80E9;
	static const uint32_t GL_PHONG_WIN = 0x80EA;
	static const uint32_t GL_PHONG_HINT_WIN = 0x80EB;
	static const uint32_t GL_FOG_SPECULAR_TEXTURE_WIN = 0x80EC;
	static const uint32_t GL_LOGIC_OP = GL_INDEX_LOGIC_OP;
	static const uint32_t GL_TEXTURE_COMPONENTS = GL_TEXTURE_INTERNAL_FORMAT;
	typedef void (FDYNGL_APIENTRY gl_accum_func)(GLenum op, GLfloat value);
	static gl_accum_func* glAccum;
	typedef void (FDYNGL_APIENTRY gl_alpha_func_func)(GLenum func, GLclampf ref);
	static gl_alpha_func_func* glAlphaFunc;
	typedef GLboolean(FDYNGL_APIENTRY gl_are_textures_resident_func)(GLsizei n, const GLuint *textures, GLboolean *residences);
	static gl_are_textures_resident_func* glAreTexturesResident;
	typedef void (FDYNGL_APIENTRY gl_array_element_func)(GLint i);
	static gl_array_element_func* glArrayElement;
	typedef void (FDYNGL_APIENTRY gl_begin_func)(GLenum mode);
	static gl_begin_func* glBegin;
	typedef void (FDYNGL_APIENTRY gl_bind_texture_func)(GLenum target, GLuint texture);
	static gl_bind_texture_func* glBindTexture;
	typedef void (FDYNGL_APIENTRY gl_bitmap_func)(GLsizei width, GLsizei height, GLfloat xorig, GLfloat yorig, GLfloat xmove, GLfloat ymove, const GLubyte *bitmap);
	static gl_bitmap_func* glBitmap;
	typedef void (FDYNGL_APIENTRY gl_blend_func_func)(GLenum sfactor, GLenum dfactor);
	static gl_blend_func_func* glBlendFunc;
	typedef void (FDYNGL_APIENTRY gl_call_list_func)(GLuint list);
	static gl_call_list_func* glCallList;
	typedef void (FDYNGL_APIENTRY gl_call_lists_func)(GLsizei n, GLenum type, const GLvoid *lists);
	static gl_call_lists_func* glCallLists;
	typedef void (FDYNGL_APIENTRY gl_clear_func)(GLbitfield mask);
	static gl_clear_func* glClear;
	typedef void (FDYNGL_APIENTRY gl_clear_accum_func)(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha);
	static gl_clear_accum_func* glClearAccum;
	typedef void (FDYNGL_APIENTRY gl_clear_color_func)(GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha);
	static gl_clear_color_func* glClearColor;
	typedef void (FDYNGL_APIENTRY gl_clear_depth_func)(GLclampd depth);
	static gl_clear_depth_func* glClearDepth;
	typedef void (FDYNGL_APIENTRY gl_clear_index_func)(GLfloat c);
	static gl_clear_index_func* glClearIndex;
	typedef void (FDYNGL_APIENTRY gl_clear_stencil_func)(GLint s);
	static gl_clear_stencil_func* glClearStencil;
	typedef void (FDYNGL_APIENTRY gl_clip_plane_func)(GLenum plane, const GLdouble *equation);
	static gl_clip_plane_func* glClipPlane;
	typedef void (FDYNGL_APIENTRY gl_color3b_func)(GLbyte red, GLbyte green, GLbyte blue);
	static gl_color3b_func* glColor3b;
	typedef void (FDYNGL_APIENTRY gl_color3bv_func)(const GLbyte *v);
	static gl_color3bv_func* glColor3bv;
	typedef void (FDYNGL_APIENTRY gl_color3d_func)(GLdouble red, GLdouble green, GLdouble blue);
	static gl_color3d_func* glColor3d;
	typedef void (FDYNGL_APIENTRY gl_color3dv_func)(const GLdouble *v);
	static gl_color3dv_func* glColor3dv;
	typedef void (FDYNGL_APIENTRY gl_color3f_func)(GLfloat red, GLfloat green, GLfloat blue);
	static gl_color3f_func* glColor3f;
	typedef void (FDYNGL_APIENTRY gl_color3fv_func)(const GLfloat *v);
	static gl_color3fv_func* glColor3fv;
	typedef void (FDYNGL_APIENTRY gl_color3i_func)(GLint red, GLint green, GLint blue);
	static gl_color3i_func* glColor3i;
	typedef void (FDYNGL_APIENTRY gl_color3iv_func)(const GLint *v);
	static gl_color3iv_func* glColor3iv;
	typedef void (FDYNGL_APIENTRY gl_color3s_func)(GLshort red, GLshort green, GLshort blue);
	static gl_color3s_func* glColor3s;
	typedef void (FDYNGL_APIENTRY gl_color3sv_func)(const GLshort *v);
	static gl_color3sv_func* glColor3sv;
	typedef void (FDYNGL_APIENTRY gl_color3ub_func)(GLubyte red, GLubyte green, GLubyte blue);
	static gl_color3ub_func* glColor3ub;
	typedef void (FDYNGL_APIENTRY gl_color3ubv_func)(const GLubyte *v);
	static gl_color3ubv_func* glColor3ubv;
	typedef void (FDYNGL_APIENTRY gl_color3ui_func)(GLuint red, GLuint green, GLuint blue);
	static gl_color3ui_func* glColor3ui;
	typedef void (FDYNGL_APIENTRY gl_color3uiv_func)(const GLuint *v);
	static gl_color3uiv_func* glColor3uiv;
	typedef void (FDYNGL_APIENTRY gl_color3us_func)(GLushort red, GLushort green, GLushort blue);
	static gl_color3us_func* glColor3us;
	typedef void (FDYNGL_APIENTRY gl_color3usv_func)(const GLushort *v);
	static gl_color3usv_func* glColor3usv;
	typedef void (FDYNGL_APIENTRY gl_color4b_func)(GLbyte red, GLbyte green, GLbyte blue, GLbyte alpha);
	static gl_color4b_func* glColor4b;
	typedef void (FDYNGL_APIENTRY gl_color4bv_func)(const GLbyte *v);
	static gl_color4bv_func* glColor4bv;
	typedef void (FDYNGL_APIENTRY gl_color4d_func)(GLdouble red, GLdouble green, GLdouble blue, GLdouble alpha);
	static gl_color4d_func* glColor4d;
	typedef void (FDYNGL_APIENTRY gl_color4dv_func)(const GLdouble *v);
	static gl_color4dv_func* glColor4dv;
	typedef void (FDYNGL_APIENTRY gl_color4f_func)(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha);
	static gl_color4f_func* glColor4f;
	typedef void (FDYNGL_APIENTRY gl_color4fv_func)(const GLfloat *v);
	static gl_color4fv_func* glColor4fv;
	typedef void (FDYNGL_APIENTRY gl_color4i_func)(GLint red, GLint green, GLint blue, GLint alpha);
	static gl_color4i_func* glColor4i;
	typedef void (FDYNGL_APIENTRY gl_color4iv_func)(const GLint *v);
	static gl_color4iv_func* glColor4iv;
	typedef void (FDYNGL_APIENTRY gl_color4s_func)(GLshort red, GLshort green, GLshort blue, GLshort alpha);
	static gl_color4s_func* glColor4s;
	typedef void (FDYNGL_APIENTRY gl_color4sv_func)(const GLshort *v);
	static gl_color4sv_func* glColor4sv;
	typedef void (FDYNGL_APIENTRY gl_color4ub_func)(GLubyte red, GLubyte green, GLubyte blue, GLubyte alpha);
	static gl_color4ub_func* glColor4ub;
	typedef void (FDYNGL_APIENTRY gl_color4ubv_func)(const GLubyte *v);
	static gl_color4ubv_func* glColor4ubv;
	typedef void (FDYNGL_APIENTRY gl_color4ui_func)(GLuint red, GLuint green, GLuint blue, GLuint alpha);
	static gl_color4ui_func* glColor4ui;
	typedef void (FDYNGL_APIENTRY gl_color4uiv_func)(const GLuint *v);
	static gl_color4uiv_func* glColor4uiv;
	typedef void (FDYNGL_APIENTRY gl_color4us_func)(GLushort red, GLushort green, GLushort blue, GLushort alpha);
	static gl_color4us_func* glColor4us;
	typedef void (FDYNGL_APIENTRY gl_color4usv_func)(const GLushort *v);
	static gl_color4usv_func* glColor4usv;
	typedef void (FDYNGL_APIENTRY gl_color_mask_func)(GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha);
	static gl_color_mask_func* glColorMask;
	typedef void (FDYNGL_APIENTRY gl_color_material_func)(GLenum face, GLenum mode);
	static gl_color_material_func* glColorMaterial;
	typedef void (FDYNGL_APIENTRY gl_color_pointer_func)(GLint size, GLenum type, GLsizei stride, const GLvoid *pointer);
	static gl_color_pointer_func* glColorPointer;
	typedef void (FDYNGL_APIENTRY gl_copy_pixels_func)(GLint x, GLint y, GLsizei width, GLsizei height, GLenum type);
	static gl_copy_pixels_func* glCopyPixels;
	typedef void (FDYNGL_APIENTRY gl_copy_tex_image1d_func)(GLenum target, GLint level, GLenum internalFormat, GLint x, GLint y, GLsizei width, GLint border);
	static gl_copy_tex_image1d_func* glCopyTexImage1D;
	typedef void (FDYNGL_APIENTRY gl_copy_tex_image2d_func)(GLenum target, GLint level, GLenum internalFormat, GLint x, GLint y, GLsizei width, GLsizei height, GLint border);
	static gl_copy_tex_image2d_func* glCopyTexImage2D;
	typedef void (FDYNGL_APIENTRY gl_copy_tex_sub_image1d_func)(GLenum target, GLint level, GLint xoffset, GLint x, GLint y, GLsizei width);
	static gl_copy_tex_sub_image1d_func* glCopyTexSubImage1D;
	typedef void (FDYNGL_APIENTRY gl_copy_tex_sub_image2d_func)(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height);
	static gl_copy_tex_sub_image2d_func* glCopyTexSubImage2D;
	typedef void (FDYNGL_APIENTRY gl_cull_face_func)(GLenum mode);
	static gl_cull_face_func* glCullFace;
	typedef void (FDYNGL_APIENTRY gl_delete_lists_func)(GLuint list, GLsizei range);
	static gl_delete_lists_func* glDeleteLists;
	typedef void (FDYNGL_APIENTRY gl_delete_textures_func)(GLsizei n, const GLuint *textures);
	static gl_delete_textures_func* glDeleteTextures;
	typedef void (FDYNGL_APIENTRY gl_depth_func_func)(GLenum func);
	static gl_depth_func_func* glDepthFunc;
	typedef void (FDYNGL_APIENTRY gl_depth_mask_func)(GLboolean flag);
	static gl_depth_mask_func* glDepthMask;
	typedef void (FDYNGL_APIENTRY gl_depth_range_func)(GLclampd zNear, GLclampd zFar);
	static gl_depth_range_func* glDepthRange;
	typedef void (FDYNGL_APIENTRY gl_disable_func)(GLenum cap);
	static gl_disable_func* glDisable;
	typedef void (FDYNGL_APIENTRY gl_disable_client_state_func)(GLenum array);
	static gl_disable_client_state_func* glDisableClientState;
	typedef void (FDYNGL_APIENTRY gl_draw_arrays_func)(GLenum mode, GLint first, GLsizei count);
	static gl_draw_arrays_func* glDrawArrays;
	typedef void (FDYNGL_APIENTRY gl_draw_buffer_func)(GLenum mode);
	static gl_draw_buffer_func* glDrawBuffer;
	typedef void (FDYNGL_APIENTRY gl_draw_elements_func)(GLenum mode, GLsizei count, GLenum type, const GLvoid *indices);
	static gl_draw_elements_func* glDrawElements;
	typedef void (FDYNGL_APIENTRY gl_draw_pixels_func)(GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid *pixels);
	static gl_draw_pixels_func* glDrawPixels;
	typedef void (FDYNGL_APIENTRY gl_edge_flag_func)(GLboolean flag);
	static gl_edge_flag_func* glEdgeFlag;
	typedef void (FDYNGL_APIENTRY gl_edge_flag_pointer_func)(GLsizei stride, const GLvoid *pointer);
	static gl_edge_flag_pointer_func* glEdgeFlagPointer;
	typedef void (FDYNGL_APIENTRY gl_edge_flagv_func)(const GLboolean *flag);
	static gl_edge_flagv_func* glEdgeFlagv;
	typedef void (FDYNGL_APIENTRY gl_enable_func)(GLenum cap);
	static gl_enable_func* glEnable;
	typedef void (FDYNGL_APIENTRY gl_enable_client_state_func)(GLenum array);
	static gl_enable_client_state_func* glEnableClientState;
	typedef void (FDYNGL_APIENTRY gl_end_func)(void);
	static gl_end_func* glEnd;
	typedef void (FDYNGL_APIENTRY gl_end_list_func)(void);
	static gl_end_list_func* glEndList;
	typedef void (FDYNGL_APIENTRY gl_eval_coord1d_func)(GLdouble u);
	static gl_eval_coord1d_func* glEvalCoord1d;
	typedef void (FDYNGL_APIENTRY gl_eval_coord1dv_func)(const GLdouble *u);
	static gl_eval_coord1dv_func* glEvalCoord1dv;
	typedef void (FDYNGL_APIENTRY gl_eval_coord1f_func)(GLfloat u);
	static gl_eval_coord1f_func* glEvalCoord1f;
	typedef void (FDYNGL_APIENTRY gl_eval_coord1fv_func)(const GLfloat *u);
	static gl_eval_coord1fv_func* glEvalCoord1fv;
	typedef void (FDYNGL_APIENTRY gl_eval_coord2d_func)(GLdouble u, GLdouble v);
	static gl_eval_coord2d_func* glEvalCoord2d;
	typedef void (FDYNGL_APIENTRY gl_eval_coord2dv_func)(const GLdouble *u);
	static gl_eval_coord2dv_func* glEvalCoord2dv;
	typedef void (FDYNGL_APIENTRY gl_eval_coord2f_func)(GLfloat u, GLfloat v);
	static gl_eval_coord2f_func* glEvalCoord2f;
	typedef void (FDYNGL_APIENTRY gl_eval_coord2fv_func)(const GLfloat *u);
	static gl_eval_coord2fv_func* glEvalCoord2fv;
	typedef void (FDYNGL_APIENTRY gl_eval_mesh1_func)(GLenum mode, GLint i1, GLint i2);
	static gl_eval_mesh1_func* glEvalMesh1;
	typedef void (FDYNGL_APIENTRY gl_eval_mesh2_func)(GLenum mode, GLint i1, GLint i2, GLint j1, GLint j2);
	static gl_eval_mesh2_func* glEvalMesh2;
	typedef void (FDYNGL_APIENTRY gl_eval_point1_func)(GLint i);
	static gl_eval_point1_func* glEvalPoint1;
	typedef void (FDYNGL_APIENTRY gl_eval_point2_func)(GLint i, GLint j);
	static gl_eval_point2_func* glEvalPoint2;
	typedef void (FDYNGL_APIENTRY gl_feedback_buffer_func)(GLsizei size, GLenum type, GLfloat *buffer);
	static gl_feedback_buffer_func* glFeedbackBuffer;
	typedef void (FDYNGL_APIENTRY gl_finish_func)(void);
	static gl_finish_func* glFinish;
	typedef void (FDYNGL_APIENTRY gl_flush_func)(void);
	static gl_flush_func* glFlush;
	typedef void (FDYNGL_APIENTRY gl_fogf_func)(GLenum pname, GLfloat param);
	static gl_fogf_func* glFogf;
	typedef void (FDYNGL_APIENTRY gl_fogfv_func)(GLenum pname, const GLfloat *params);
	static gl_fogfv_func* glFogfv;
	typedef void (FDYNGL_APIENTRY gl_fogi_func)(GLenum pname, GLint param);
	static gl_fogi_func* glFogi;
	typedef void (FDYNGL_APIENTRY gl_fogiv_func)(GLenum pname, const GLint *params);
	static gl_fogiv_func* glFogiv;
	typedef void (FDYNGL_APIENTRY gl_front_face_func)(GLenum mode);
	static gl_front_face_func* glFrontFace;
	typedef void (FDYNGL_APIENTRY gl_frustum_func)(GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble zNear, GLdouble zFar);
	static gl_frustum_func* glFrustum;
	typedef GLuint(FDYNGL_APIENTRY gl_gen_lists_func)(GLsizei range);
	static gl_gen_lists_func* glGenLists;
	typedef void (FDYNGL_APIENTRY gl_gen_textures_func)(GLsizei n, GLuint *textures);
	static gl_gen_textures_func* glGenTextures;
	typedef void (FDYNGL_APIENTRY gl_get_booleanv_func)(GLenum pname, GLboolean *params);
	static gl_get_booleanv_func* glGetBooleanv;
	typedef void (FDYNGL_APIENTRY gl_get_clip_plane_func)(GLenum plane, GLdouble *equation);
	static gl_get_clip_plane_func* glGetClipPlane;
	typedef void (FDYNGL_APIENTRY gl_get_doublev_func)(GLenum pname, GLdouble *params);
	static gl_get_doublev_func* glGetDoublev;
	typedef GLenum(FDYNGL_APIENTRY gl_get_error_func)(void);
	static gl_get_error_func* glGetError;
	typedef void (FDYNGL_APIENTRY gl_get_floatv_func)(GLenum pname, GLfloat *params);
	static gl_get_floatv_func* glGetFloatv;
	typedef void (FDYNGL_APIENTRY gl_get_integerv_func)(GLenum pname, GLint *params);
	static gl_get_integerv_func* glGetIntegerv;
	typedef void (FDYNGL_APIENTRY gl_get_lightfv_func)(GLenum light, GLenum pname, GLfloat *params);
	static gl_get_lightfv_func* glGetLightfv;
	typedef void (FDYNGL_APIENTRY gl_get_lightiv_func)(GLenum light, GLenum pname, GLint *params);
	static gl_get_lightiv_func* glGetLightiv;
	typedef void (FDYNGL_APIENTRY gl_get_mapdv_func)(GLenum target, GLenum query, GLdouble *v);
	static gl_get_mapdv_func* glGetMapdv;
	typedef void (FDYNGL_APIENTRY gl_get_mapfv_func)(GLenum target, GLenum query, GLfloat *v);
	static gl_get_mapfv_func* glGetMapfv;
	typedef void (FDYNGL_APIENTRY gl_get_mapiv_func)(GLenum target, GLenum query, GLint *v);
	static gl_get_mapiv_func* glGetMapiv;
	typedef void (FDYNGL_APIENTRY gl_get_materialfv_func)(GLenum face, GLenum pname, GLfloat *params);
	static gl_get_materialfv_func* glGetMaterialfv;
	typedef void (FDYNGL_APIENTRY gl_get_materialiv_func)(GLenum face, GLenum pname, GLint *params);
	static gl_get_materialiv_func* glGetMaterialiv;
	typedef void (FDYNGL_APIENTRY gl_get_pixel_mapfv_func)(GLenum map, GLfloat *values);
	static gl_get_pixel_mapfv_func* glGetPixelMapfv;
	typedef void (FDYNGL_APIENTRY gl_get_pixel_mapuiv_func)(GLenum map, GLuint *values);
	static gl_get_pixel_mapuiv_func* glGetPixelMapuiv;
	typedef void (FDYNGL_APIENTRY gl_get_pixel_mapusv_func)(GLenum map, GLushort *values);
	static gl_get_pixel_mapusv_func* glGetPixelMapusv;
	typedef void (FDYNGL_APIENTRY gl_get_pointerv_func)(GLenum pname, GLvoid* *params);
	static gl_get_pointerv_func* glGetPointerv;
	typedef void (FDYNGL_APIENTRY gl_get_polygon_stipple_func)(GLubyte *mask);
	static gl_get_polygon_stipple_func* glGetPolygonStipple;
	typedef const GLubyte * (FDYNGL_APIENTRY gl_get_string_func)(GLenum name);
	static gl_get_string_func* glGetString;
	typedef void (FDYNGL_APIENTRY gl_get_tex_envfv_func)(GLenum target, GLenum pname, GLfloat *params);
	static gl_get_tex_envfv_func* glGetTexEnvfv;
	typedef void (FDYNGL_APIENTRY gl_get_tex_enviv_func)(GLenum target, GLenum pname, GLint *params);
	static gl_get_tex_enviv_func* glGetTexEnviv;
	typedef void (FDYNGL_APIENTRY gl_get_tex_gendv_func)(GLenum coord, GLenum pname, GLdouble *params);
	static gl_get_tex_gendv_func* glGetTexGendv;
	typedef void (FDYNGL_APIENTRY gl_get_tex_genfv_func)(GLenum coord, GLenum pname, GLfloat *params);
	static gl_get_tex_genfv_func* glGetTexGenfv;
	typedef void (FDYNGL_APIENTRY gl_get_tex_geniv_func)(GLenum coord, GLenum pname, GLint *params);
	static gl_get_tex_geniv_func* glGetTexGeniv;
	typedef void (FDYNGL_APIENTRY gl_get_tex_image_func)(GLenum target, GLint level, GLenum format, GLenum type, GLvoid *pixels);
	static gl_get_tex_image_func* glGetTexImage;
	typedef void (FDYNGL_APIENTRY gl_get_tex_level_parameterfv_func)(GLenum target, GLint level, GLenum pname, GLfloat *params);
	static gl_get_tex_level_parameterfv_func* glGetTexLevelParameterfv;
	typedef void (FDYNGL_APIENTRY gl_get_tex_level_parameteriv_func)(GLenum target, GLint level, GLenum pname, GLint *params);
	static gl_get_tex_level_parameteriv_func* glGetTexLevelParameteriv;
	typedef void (FDYNGL_APIENTRY gl_get_tex_parameterfv_func)(GLenum target, GLenum pname, GLfloat *params);
	static gl_get_tex_parameterfv_func* glGetTexParameterfv;
	typedef void (FDYNGL_APIENTRY gl_get_tex_parameteriv_func)(GLenum target, GLenum pname, GLint *params);
	static gl_get_tex_parameteriv_func* glGetTexParameteriv;
	typedef void (FDYNGL_APIENTRY gl_hint_func)(GLenum target, GLenum mode);
	static gl_hint_func* glHint;
	typedef void (FDYNGL_APIENTRY gl_index_mask_func)(GLuint mask);
	static gl_index_mask_func* glIndexMask;
	typedef void (FDYNGL_APIENTRY gl_index_pointer_func)(GLenum type, GLsizei stride, const GLvoid *pointer);
	static gl_index_pointer_func* glIndexPointer;
	typedef void (FDYNGL_APIENTRY gl_indexd_func)(GLdouble c);
	static gl_indexd_func* glIndexd;
	typedef void (FDYNGL_APIENTRY gl_indexdv_func)(const GLdouble *c);
	static gl_indexdv_func* glIndexdv;
	typedef void (FDYNGL_APIENTRY gl_indexf_func)(GLfloat c);
	static gl_indexf_func* glIndexf;
	typedef void (FDYNGL_APIENTRY gl_indexfv_func)(const GLfloat *c);
	static gl_indexfv_func* glIndexfv;
	typedef void (FDYNGL_APIENTRY gl_indexi_func)(GLint c);
	static gl_indexi_func* glIndexi;
	typedef void (FDYNGL_APIENTRY gl_indexiv_func)(const GLint *c);
	static gl_indexiv_func* glIndexiv;
	typedef void (FDYNGL_APIENTRY gl_indexs_func)(GLshort c);
	static gl_indexs_func* glIndexs;
	typedef void (FDYNGL_APIENTRY gl_indexsv_func)(const GLshort *c);
	static gl_indexsv_func* glIndexsv;
	typedef void (FDYNGL_APIENTRY gl_indexub_func)(GLubyte c);
	static gl_indexub_func* glIndexub;
	typedef void (FDYNGL_APIENTRY gl_indexubv_func)(const GLubyte *c);
	static gl_indexubv_func* glIndexubv;
	typedef void (FDYNGL_APIENTRY gl_init_names_func)(void);
	static gl_init_names_func* glInitNames;
	typedef void (FDYNGL_APIENTRY gl_interleaved_arrays_func)(GLenum format, GLsizei stride, const GLvoid *pointer);
	static gl_interleaved_arrays_func* glInterleavedArrays;
	typedef GLboolean(FDYNGL_APIENTRY gl_is_enabled_func)(GLenum cap);
	static gl_is_enabled_func* glIsEnabled;
	typedef GLboolean(FDYNGL_APIENTRY gl_is_list_func)(GLuint list);
	static gl_is_list_func* glIsList;
	typedef GLboolean(FDYNGL_APIENTRY gl_is_texture_func)(GLuint texture);
	static gl_is_texture_func* glIsTexture;
	typedef void (FDYNGL_APIENTRY gl_light_modelf_func)(GLenum pname, GLfloat param);
	static gl_light_modelf_func* glLightModelf;
	typedef void (FDYNGL_APIENTRY gl_light_modelfv_func)(GLenum pname, const GLfloat *params);
	static gl_light_modelfv_func* glLightModelfv;
	typedef void (FDYNGL_APIENTRY gl_light_modeli_func)(GLenum pname, GLint param);
	static gl_light_modeli_func* glLightModeli;
	typedef void (FDYNGL_APIENTRY gl_light_modeliv_func)(GLenum pname, const GLint *params);
	static gl_light_modeliv_func* glLightModeliv;
	typedef void (FDYNGL_APIENTRY gl_lightf_func)(GLenum light, GLenum pname, GLfloat param);
	static gl_lightf_func* glLightf;
	typedef void (FDYNGL_APIENTRY gl_lightfv_func)(GLenum light, GLenum pname, const GLfloat *params);
	static gl_lightfv_func* glLightfv;
	typedef void (FDYNGL_APIENTRY gl_lighti_func)(GLenum light, GLenum pname, GLint param);
	static gl_lighti_func* glLighti;
	typedef void (FDYNGL_APIENTRY gl_lightiv_func)(GLenum light, GLenum pname, const GLint *params);
	static gl_lightiv_func* glLightiv;
	typedef void (FDYNGL_APIENTRY gl_line_stipple_func)(GLint factor, GLushort pattern);
	static gl_line_stipple_func* glLineStipple;
	typedef void (FDYNGL_APIENTRY gl_line_width_func)(GLfloat width);
	static gl_line_width_func* glLineWidth;
	typedef void (FDYNGL_APIENTRY gl_list_base_func)(GLuint base);
	static gl_list_base_func* glListBase;
	typedef void (FDYNGL_APIENTRY gl_load_identity_func)(void);
	static gl_load_identity_func* glLoadIdentity;
	typedef void (FDYNGL_APIENTRY gl_load_matrixd_func)(const GLdouble *m);
	static gl_load_matrixd_func* glLoadMatrixd;
	typedef void (FDYNGL_APIENTRY gl_load_matrixf_func)(const GLfloat *m);
	static gl_load_matrixf_func* glLoadMatrixf;
	typedef void (FDYNGL_APIENTRY gl_load_name_func)(GLuint name);
	static gl_load_name_func* glLoadName;
	typedef void (FDYNGL_APIENTRY gl_logic_op_func)(GLenum opcode);
	static gl_logic_op_func* glLogicOp;
	typedef void (FDYNGL_APIENTRY gl_map1d_func)(GLenum target, GLdouble u1, GLdouble u2, GLint stride, GLint order, const GLdouble *points);
	static gl_map1d_func* glMap1d;
	typedef void (FDYNGL_APIENTRY gl_map1f_func)(GLenum target, GLfloat u1, GLfloat u2, GLint stride, GLint order, const GLfloat *points);
	static gl_map1f_func* glMap1f;
	typedef void (FDYNGL_APIENTRY gl_map2d_func)(GLenum target, GLdouble u1, GLdouble u2, GLint ustride, GLint uorder, GLdouble v1, GLdouble v2, GLint vstride, GLint vorder, const GLdouble *points);
	static gl_map2d_func* glMap2d;
	typedef void (FDYNGL_APIENTRY gl_map2f_func)(GLenum target, GLfloat u1, GLfloat u2, GLint ustride, GLint uorder, GLfloat v1, GLfloat v2, GLint vstride, GLint vorder, const GLfloat *points);
	static gl_map2f_func* glMap2f;
	typedef void (FDYNGL_APIENTRY gl_map_grid1d_func)(GLint un, GLdouble u1, GLdouble u2);
	static gl_map_grid1d_func* glMapGrid1d;
	typedef void (FDYNGL_APIENTRY gl_map_grid1f_func)(GLint un, GLfloat u1, GLfloat u2);
	static gl_map_grid1f_func* glMapGrid1f;
	typedef void (FDYNGL_APIENTRY gl_map_grid2d_func)(GLint un, GLdouble u1, GLdouble u2, GLint vn, GLdouble v1, GLdouble v2);
	static gl_map_grid2d_func* glMapGrid2d;
	typedef void (FDYNGL_APIENTRY gl_map_grid2f_func)(GLint un, GLfloat u1, GLfloat u2, GLint vn, GLfloat v1, GLfloat v2);
	static gl_map_grid2f_func* glMapGrid2f;
	typedef void (FDYNGL_APIENTRY gl_materialf_func)(GLenum face, GLenum pname, GLfloat param);
	static gl_materialf_func* glMaterialf;
	typedef void (FDYNGL_APIENTRY gl_materialfv_func)(GLenum face, GLenum pname, const GLfloat *params);
	static gl_materialfv_func* glMaterialfv;
	typedef void (FDYNGL_APIENTRY gl_materiali_func)(GLenum face, GLenum pname, GLint param);
	static gl_materiali_func* glMateriali;
	typedef void (FDYNGL_APIENTRY gl_materialiv_func)(GLenum face, GLenum pname, const GLint *params);
	static gl_materialiv_func* glMaterialiv;
	typedef void (FDYNGL_APIENTRY gl_matrix_mode_func)(GLenum mode);
	static gl_matrix_mode_func* glMatrixMode;
	typedef void (FDYNGL_APIENTRY gl_mult_matrixd_func)(const GLdouble *m);
	static gl_mult_matrixd_func* glMultMatrixd;
	typedef void (FDYNGL_APIENTRY gl_mult_matrixf_func)(const GLfloat *m);
	static gl_mult_matrixf_func* glMultMatrixf;
	typedef void (FDYNGL_APIENTRY gl_new_list_func)(GLuint list, GLenum mode);
	static gl_new_list_func* glNewList;
	typedef void (FDYNGL_APIENTRY gl_normal3b_func)(GLbyte nx, GLbyte ny, GLbyte nz);
	static gl_normal3b_func* glNormal3b;
	typedef void (FDYNGL_APIENTRY gl_normal3bv_func)(const GLbyte *v);
	static gl_normal3bv_func* glNormal3bv;
	typedef void (FDYNGL_APIENTRY gl_normal3d_func)(GLdouble nx, GLdouble ny, GLdouble nz);
	static gl_normal3d_func* glNormal3d;
	typedef void (FDYNGL_APIENTRY gl_normal3dv_func)(const GLdouble *v);
	static gl_normal3dv_func* glNormal3dv;
	typedef void (FDYNGL_APIENTRY gl_normal3f_func)(GLfloat nx, GLfloat ny, GLfloat nz);
	static gl_normal3f_func* glNormal3f;
	typedef void (FDYNGL_APIENTRY gl_normal3fv_func)(const GLfloat *v);
	static gl_normal3fv_func* glNormal3fv;
	typedef void (FDYNGL_APIENTRY gl_normal3i_func)(GLint nx, GLint ny, GLint nz);
	static gl_normal3i_func* glNormal3i;
	typedef void (FDYNGL_APIENTRY gl_normal3iv_func)(const GLint *v);
	static gl_normal3iv_func* glNormal3iv;
	typedef void (FDYNGL_APIENTRY gl_normal3s_func)(GLshort nx, GLshort ny, GLshort nz);
	static gl_normal3s_func* glNormal3s;
	typedef void (FDYNGL_APIENTRY gl_normal3sv_func)(const GLshort *v);
	static gl_normal3sv_func* glNormal3sv;
	typedef void (FDYNGL_APIENTRY gl_normal_pointer_func)(GLenum type, GLsizei stride, const GLvoid *pointer);
	static gl_normal_pointer_func* glNormalPointer;
	typedef void (FDYNGL_APIENTRY gl_ortho_func)(GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble zNear, GLdouble zFar);
	static gl_ortho_func* glOrtho;
	typedef void (FDYNGL_APIENTRY gl_pass_through_func)(GLfloat token);
	static gl_pass_through_func* glPassThrough;
	typedef void (FDYNGL_APIENTRY gl_pixel_mapfv_func)(GLenum map, GLsizei mapsize, const GLfloat *values);
	static gl_pixel_mapfv_func* glPixelMapfv;
	typedef void (FDYNGL_APIENTRY gl_pixel_mapuiv_func)(GLenum map, GLsizei mapsize, const GLuint *values);
	static gl_pixel_mapuiv_func* glPixelMapuiv;
	typedef void (FDYNGL_APIENTRY gl_pixel_mapusv_func)(GLenum map, GLsizei mapsize, const GLushort *values);
	static gl_pixel_mapusv_func* glPixelMapusv;
	typedef void (FDYNGL_APIENTRY gl_pixel_storef_func)(GLenum pname, GLfloat param);
	static gl_pixel_storef_func* glPixelStoref;
	typedef void (FDYNGL_APIENTRY gl_pixel_storei_func)(GLenum pname, GLint param);
	static gl_pixel_storei_func* glPixelStorei;
	typedef void (FDYNGL_APIENTRY gl_pixel_transferf_func)(GLenum pname, GLfloat param);
	static gl_pixel_transferf_func* glPixelTransferf;
	typedef void (FDYNGL_APIENTRY gl_pixel_transferi_func)(GLenum pname, GLint param);
	static gl_pixel_transferi_func* glPixelTransferi;
	typedef void (FDYNGL_APIENTRY gl_pixel_zoom_func)(GLfloat xfactor, GLfloat yfactor);
	static gl_pixel_zoom_func* glPixelZoom;
	typedef void (FDYNGL_APIENTRY gl_point_size_func)(GLfloat size);
	static gl_point_size_func* glPointSize;
	typedef void (FDYNGL_APIENTRY gl_polygon_mode_func)(GLenum face, GLenum mode);
	static gl_polygon_mode_func* glPolygonMode;
	typedef void (FDYNGL_APIENTRY gl_polygon_offset_func)(GLfloat factor, GLfloat units);
	static gl_polygon_offset_func* glPolygonOffset;
	typedef void (FDYNGL_APIENTRY gl_polygon_stipple_func)(const GLubyte *mask);
	static gl_polygon_stipple_func* glPolygonStipple;
	typedef void (FDYNGL_APIENTRY gl_pop_attrib_func)(void);
	static gl_pop_attrib_func* glPopAttrib;
	typedef void (FDYNGL_APIENTRY gl_pop_client_attrib_func)(void);
	static gl_pop_client_attrib_func* glPopClientAttrib;
	typedef void (FDYNGL_APIENTRY gl_pop_matrix_func)(void);
	static gl_pop_matrix_func* glPopMatrix;
	typedef void (FDYNGL_APIENTRY gl_pop_name_func)(void);
	static gl_pop_name_func* glPopName;
	typedef void (FDYNGL_APIENTRY gl_prioritize_textures_func)(GLsizei n, const GLuint *textures, const GLclampf *priorities);
	static gl_prioritize_textures_func* glPrioritizeTextures;
	typedef void (FDYNGL_APIENTRY gl_push_attrib_func)(GLbitfield mask);
	static gl_push_attrib_func* glPushAttrib;
	typedef void (FDYNGL_APIENTRY gl_push_client_attrib_func)(GLbitfield mask);
	static gl_push_client_attrib_func* glPushClientAttrib;
	typedef void (FDYNGL_APIENTRY gl_push_matrix_func)(void);
	static gl_push_matrix_func* glPushMatrix;
	typedef void (FDYNGL_APIENTRY gl_push_name_func)(GLuint name);
	static gl_push_name_func* glPushName;
	typedef void (FDYNGL_APIENTRY gl_raster_pos2d_func)(GLdouble x, GLdouble y);
	static gl_raster_pos2d_func* glRasterPos2d;
	typedef void (FDYNGL_APIENTRY gl_raster_pos2dv_func)(const GLdouble *v);
	static gl_raster_pos2dv_func* glRasterPos2dv;
	typedef void (FDYNGL_APIENTRY gl_raster_pos2f_func)(GLfloat x, GLfloat y);
	static gl_raster_pos2f_func* glRasterPos2f;
	typedef void (FDYNGL_APIENTRY gl_raster_pos2fv_func)(const GLfloat *v);
	static gl_raster_pos2fv_func* glRasterPos2fv;
	typedef void (FDYNGL_APIENTRY gl_raster_pos2i_func)(GLint x, GLint y);
	static gl_raster_pos2i_func* glRasterPos2i;
	typedef void (FDYNGL_APIENTRY gl_raster_pos2iv_func)(const GLint *v);
	static gl_raster_pos2iv_func* glRasterPos2iv;
	typedef void (FDYNGL_APIENTRY gl_raster_pos2s_func)(GLshort x, GLshort y);
	static gl_raster_pos2s_func* glRasterPos2s;
	typedef void (FDYNGL_APIENTRY gl_raster_pos2sv_func)(const GLshort *v);
	static gl_raster_pos2sv_func* glRasterPos2sv;
	typedef void (FDYNGL_APIENTRY gl_raster_pos3d_func)(GLdouble x, GLdouble y, GLdouble z);
	static gl_raster_pos3d_func* glRasterPos3d;
	typedef void (FDYNGL_APIENTRY gl_raster_pos3dv_func)(const GLdouble *v);
	static gl_raster_pos3dv_func* glRasterPos3dv;
	typedef void (FDYNGL_APIENTRY gl_raster_pos3f_func)(GLfloat x, GLfloat y, GLfloat z);
	static gl_raster_pos3f_func* glRasterPos3f;
	typedef void (FDYNGL_APIENTRY gl_raster_pos3fv_func)(const GLfloat *v);
	static gl_raster_pos3fv_func* glRasterPos3fv;
	typedef void (FDYNGL_APIENTRY gl_raster_pos3i_func)(GLint x, GLint y, GLint z);
	static gl_raster_pos3i_func* glRasterPos3i;
	typedef void (FDYNGL_APIENTRY gl_raster_pos3iv_func)(const GLint *v);
	static gl_raster_pos3iv_func* glRasterPos3iv;
	typedef void (FDYNGL_APIENTRY gl_raster_pos3s_func)(GLshort x, GLshort y, GLshort z);
	static gl_raster_pos3s_func* glRasterPos3s;
	typedef void (FDYNGL_APIENTRY gl_raster_pos3sv_func)(const GLshort *v);
	static gl_raster_pos3sv_func* glRasterPos3sv;
	typedef void (FDYNGL_APIENTRY gl_raster_pos4d_func)(GLdouble x, GLdouble y, GLdouble z, GLdouble w);
	static gl_raster_pos4d_func* glRasterPos4d;
	typedef void (FDYNGL_APIENTRY gl_raster_pos4dv_func)(const GLdouble *v);
	static gl_raster_pos4dv_func* glRasterPos4dv;
	typedef void (FDYNGL_APIENTRY gl_raster_pos4f_func)(GLfloat x, GLfloat y, GLfloat z, GLfloat w);
	static gl_raster_pos4f_func* glRasterPos4f;
	typedef void (FDYNGL_APIENTRY gl_raster_pos4fv_func)(const GLfloat *v);
	static gl_raster_pos4fv_func* glRasterPos4fv;
	typedef void (FDYNGL_APIENTRY gl_raster_pos4i_func)(GLint x, GLint y, GLint z, GLint w);
	static gl_raster_pos4i_func* glRasterPos4i;
	typedef void (FDYNGL_APIENTRY gl_raster_pos4iv_func)(const GLint *v);
	static gl_raster_pos4iv_func* glRasterPos4iv;
	typedef void (FDYNGL_APIENTRY gl_raster_pos4s_func)(GLshort x, GLshort y, GLshort z, GLshort w);
	static gl_raster_pos4s_func* glRasterPos4s;
	typedef void (FDYNGL_APIENTRY gl_raster_pos4sv_func)(const GLshort *v);
	static gl_raster_pos4sv_func* glRasterPos4sv;
	typedef void (FDYNGL_APIENTRY gl_read_buffer_func)(GLenum mode);
	static gl_read_buffer_func* glReadBuffer;
	typedef void (FDYNGL_APIENTRY gl_read_pixels_func)(GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, GLvoid *pixels);
	static gl_read_pixels_func* glReadPixels;
	typedef void (FDYNGL_APIENTRY gl_rectd_func)(GLdouble x1, GLdouble y1, GLdouble x2, GLdouble y2);
	static gl_rectd_func* glRectd;
	typedef void (FDYNGL_APIENTRY gl_rectdv_func)(const GLdouble *v1, const GLdouble *v2);
	static gl_rectdv_func* glRectdv;
	typedef void (FDYNGL_APIENTRY gl_rectf_func)(GLfloat x1, GLfloat y1, GLfloat x2, GLfloat y2);
	static gl_rectf_func* glRectf;
	typedef void (FDYNGL_APIENTRY gl_rectfv_func)(const GLfloat *v1, const GLfloat *v2);
	static gl_rectfv_func* glRectfv;
	typedef void (FDYNGL_APIENTRY gl_recti_func)(GLint x1, GLint y1, GLint x2, GLint y2);
	static gl_recti_func* glRecti;
	typedef void (FDYNGL_APIENTRY gl_rectiv_func)(const GLint *v1, const GLint *v2);
	static gl_rectiv_func* glRectiv;
	typedef void (FDYNGL_APIENTRY gl_rects_func)(GLshort x1, GLshort y1, GLshort x2, GLshort y2);
	static gl_rects_func* glRects;
	typedef void (FDYNGL_APIENTRY gl_rectsv_func)(const GLshort *v1, const GLshort *v2);
	static gl_rectsv_func* glRectsv;
	typedef GLint(FDYNGL_APIENTRY gl_render_mode_func)(GLenum mode);
	static gl_render_mode_func* glRenderMode;
	typedef void (FDYNGL_APIENTRY gl_rotated_func)(GLdouble angle, GLdouble x, GLdouble y, GLdouble z);
	static gl_rotated_func* glRotated;
	typedef void (FDYNGL_APIENTRY gl_rotatef_func)(GLfloat angle, GLfloat x, GLfloat y, GLfloat z);
	static gl_rotatef_func* glRotatef;
	typedef void (FDYNGL_APIENTRY gl_scaled_func)(GLdouble x, GLdouble y, GLdouble z);
	static gl_scaled_func* glScaled;
	typedef void (FDYNGL_APIENTRY gl_scalef_func)(GLfloat x, GLfloat y, GLfloat z);
	static gl_scalef_func* glScalef;
	typedef void (FDYNGL_APIENTRY gl_scissor_func)(GLint x, GLint y, GLsizei width, GLsizei height);
	static gl_scissor_func* glScissor;
	typedef void (FDYNGL_APIENTRY gl_select_buffer_func)(GLsizei size, GLuint *buffer);
	static gl_select_buffer_func* glSelectBuffer;
	typedef void (FDYNGL_APIENTRY gl_shade_model_func)(GLenum mode);
	static gl_shade_model_func* glShadeModel;
	typedef void (FDYNGL_APIENTRY gl_stencil_func_func)(GLenum func, GLint ref, GLuint mask);
	static gl_stencil_func_func* glStencilFunc;
	typedef void (FDYNGL_APIENTRY gl_stencil_mask_func)(GLuint mask);
	static gl_stencil_mask_func* glStencilMask;
	typedef void (FDYNGL_APIENTRY gl_stencil_op_func)(GLenum fail, GLenum zfail, GLenum zpass);
	static gl_stencil_op_func* glStencilOp;
	typedef void (FDYNGL_APIENTRY gl_tex_coord1d_func)(GLdouble s);
	static gl_tex_coord1d_func* glTexCoord1d;
	typedef void (FDYNGL_APIENTRY gl_tex_coord1dv_func)(const GLdouble *v);
	static gl_tex_coord1dv_func* glTexCoord1dv;
	typedef void (FDYNGL_APIENTRY gl_tex_coord1f_func)(GLfloat s);
	static gl_tex_coord1f_func* glTexCoord1f;
	typedef void (FDYNGL_APIENTRY gl_tex_coord1fv_func)(const GLfloat *v);
	static gl_tex_coord1fv_func* glTexCoord1fv;
	typedef void (FDYNGL_APIENTRY gl_tex_coord1i_func)(GLint s);
	static gl_tex_coord1i_func* glTexCoord1i;
	typedef void (FDYNGL_APIENTRY gl_tex_coord1iv_func)(const GLint *v);
	static gl_tex_coord1iv_func* glTexCoord1iv;
	typedef void (FDYNGL_APIENTRY gl_tex_coord1s_func)(GLshort s);
	static gl_tex_coord1s_func* glTexCoord1s;
	typedef void (FDYNGL_APIENTRY gl_tex_coord1sv_func)(const GLshort *v);
	static gl_tex_coord1sv_func* glTexCoord1sv;
	typedef void (FDYNGL_APIENTRY gl_tex_coord2d_func)(GLdouble s, GLdouble t);
	static gl_tex_coord2d_func* glTexCoord2d;
	typedef void (FDYNGL_APIENTRY gl_tex_coord2dv_func)(const GLdouble *v);
	static gl_tex_coord2dv_func* glTexCoord2dv;
	typedef void (FDYNGL_APIENTRY gl_tex_coord2f_func)(GLfloat s, GLfloat t);
	static gl_tex_coord2f_func* glTexCoord2f;
	typedef void (FDYNGL_APIENTRY gl_tex_coord2fv_func)(const GLfloat *v);
	static gl_tex_coord2fv_func* glTexCoord2fv;
	typedef void (FDYNGL_APIENTRY gl_tex_coord2i_func)(GLint s, GLint t);
	static gl_tex_coord2i_func* glTexCoord2i;
	typedef void (FDYNGL_APIENTRY gl_tex_coord2iv_func)(const GLint *v);
	static gl_tex_coord2iv_func* glTexCoord2iv;
	typedef void (FDYNGL_APIENTRY gl_tex_coord2s_func)(GLshort s, GLshort t);
	static gl_tex_coord2s_func* glTexCoord2s;
	typedef void (FDYNGL_APIENTRY gl_tex_coord2sv_func)(const GLshort *v);
	static gl_tex_coord2sv_func* glTexCoord2sv;
	typedef void (FDYNGL_APIENTRY gl_tex_coord3d_func)(GLdouble s, GLdouble t, GLdouble r);
	static gl_tex_coord3d_func* glTexCoord3d;
	typedef void (FDYNGL_APIENTRY gl_tex_coord3dv_func)(const GLdouble *v);
	static gl_tex_coord3dv_func* glTexCoord3dv;
	typedef void (FDYNGL_APIENTRY gl_tex_coord3f_func)(GLfloat s, GLfloat t, GLfloat r);
	static gl_tex_coord3f_func* glTexCoord3f;
	typedef void (FDYNGL_APIENTRY gl_tex_coord3fv_func)(const GLfloat *v);
	static gl_tex_coord3fv_func* glTexCoord3fv;
	typedef void (FDYNGL_APIENTRY gl_tex_coord3i_func)(GLint s, GLint t, GLint r);
	static gl_tex_coord3i_func* glTexCoord3i;
	typedef void (FDYNGL_APIENTRY gl_tex_coord3iv_func)(const GLint *v);
	static gl_tex_coord3iv_func* glTexCoord3iv;
	typedef void (FDYNGL_APIENTRY gl_tex_coord3s_func)(GLshort s, GLshort t, GLshort r);
	static gl_tex_coord3s_func* glTexCoord3s;
	typedef void (FDYNGL_APIENTRY gl_tex_coord3sv_func)(const GLshort *v);
	static gl_tex_coord3sv_func* glTexCoord3sv;
	typedef void (FDYNGL_APIENTRY gl_tex_coord4d_func)(GLdouble s, GLdouble t, GLdouble r, GLdouble q);
	static gl_tex_coord4d_func* glTexCoord4d;
	typedef void (FDYNGL_APIENTRY gl_tex_coord4dv_func)(const GLdouble *v);
	static gl_tex_coord4dv_func* glTexCoord4dv;
	typedef void (FDYNGL_APIENTRY gl_tex_coord4f_func)(GLfloat s, GLfloat t, GLfloat r, GLfloat q);
	static gl_tex_coord4f_func* glTexCoord4f;
	typedef void (FDYNGL_APIENTRY gl_tex_coord4fv_func)(const GLfloat *v);
	static gl_tex_coord4fv_func* glTexCoord4fv;
	typedef void (FDYNGL_APIENTRY gl_tex_coord4i_func)(GLint s, GLint t, GLint r, GLint q);
	static gl_tex_coord4i_func* glTexCoord4i;
	typedef void (FDYNGL_APIENTRY gl_tex_coord4iv_func)(const GLint *v);
	static gl_tex_coord4iv_func* glTexCoord4iv;
	typedef void (FDYNGL_APIENTRY gl_tex_coord4s_func)(GLshort s, GLshort t, GLshort r, GLshort q);
	static gl_tex_coord4s_func* glTexCoord4s;
	typedef void (FDYNGL_APIENTRY gl_tex_coord4sv_func)(const GLshort *v);
	static gl_tex_coord4sv_func* glTexCoord4sv;
	typedef void (FDYNGL_APIENTRY gl_tex_coord_pointer_func)(GLint size, GLenum type, GLsizei stride, const GLvoid *pointer);
	static gl_tex_coord_pointer_func* glTexCoordPointer;
	typedef void (FDYNGL_APIENTRY gl_tex_envf_func)(GLenum target, GLenum pname, GLfloat param);
	static gl_tex_envf_func* glTexEnvf;
	typedef void (FDYNGL_APIENTRY gl_tex_envfv_func)(GLenum target, GLenum pname, const GLfloat *params);
	static gl_tex_envfv_func* glTexEnvfv;
	typedef void (FDYNGL_APIENTRY gl_tex_envi_func)(GLenum target, GLenum pname, GLint param);
	static gl_tex_envi_func* glTexEnvi;
	typedef void (FDYNGL_APIENTRY gl_tex_enviv_func)(GLenum target, GLenum pname, const GLint *params);
	static gl_tex_enviv_func* glTexEnviv;
	typedef void (FDYNGL_APIENTRY gl_tex_gend_func)(GLenum coord, GLenum pname, GLdouble param);
	static gl_tex_gend_func* glTexGend;
	typedef void (FDYNGL_APIENTRY gl_tex_gendv_func)(GLenum coord, GLenum pname, const GLdouble *params);
	static gl_tex_gendv_func* glTexGendv;
	typedef void (FDYNGL_APIENTRY gl_tex_genf_func)(GLenum coord, GLenum pname, GLfloat param);
	static gl_tex_genf_func* glTexGenf;
	typedef void (FDYNGL_APIENTRY gl_tex_genfv_func)(GLenum coord, GLenum pname, const GLfloat *params);
	static gl_tex_genfv_func* glTexGenfv;
	typedef void (FDYNGL_APIENTRY gl_tex_geni_func)(GLenum coord, GLenum pname, GLint param);
	static gl_tex_geni_func* glTexGeni;
	typedef void (FDYNGL_APIENTRY gl_tex_geniv_func)(GLenum coord, GLenum pname, const GLint *params);
	static gl_tex_geniv_func* glTexGeniv;
	typedef void (FDYNGL_APIENTRY gl_tex_image1d_func)(GLenum target, GLint level, GLint internalformat, GLsizei width, GLint border, GLenum format, GLenum type, const GLvoid *pixels);
	static gl_tex_image1d_func* glTexImage1D;
	typedef void (FDYNGL_APIENTRY gl_tex_image2d_func)(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const GLvoid *pixels);
	static gl_tex_image2d_func* glTexImage2D;
	typedef void (FDYNGL_APIENTRY gl_tex_parameterf_func)(GLenum target, GLenum pname, GLfloat param);
	static gl_tex_parameterf_func* glTexParameterf;
	typedef void (FDYNGL_APIENTRY gl_tex_parameterfv_func)(GLenum target, GLenum pname, const GLfloat *params);
	static gl_tex_parameterfv_func* glTexParameterfv;
	typedef void (FDYNGL_APIENTRY gl_tex_parameteri_func)(GLenum target, GLenum pname, GLint param);
	static gl_tex_parameteri_func* glTexParameteri;
	typedef void (FDYNGL_APIENTRY gl_tex_parameteriv_func)(GLenum target, GLenum pname, const GLint *params);
	static gl_tex_parameteriv_func* glTexParameteriv;
	typedef void (FDYNGL_APIENTRY gl_tex_sub_image1d_func)(GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLenum type, const GLvoid *pixels);
	static gl_tex_sub_image1d_func* glTexSubImage1D;
	typedef void (FDYNGL_APIENTRY gl_tex_sub_image2d_func)(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid *pixels);
	static gl_tex_sub_image2d_func* glTexSubImage2D;
	typedef void (FDYNGL_APIENTRY gl_translated_func)(GLdouble x, GLdouble y, GLdouble z);
	static gl_translated_func* glTranslated;
	typedef void (FDYNGL_APIENTRY gl_translatef_func)(GLfloat x, GLfloat y, GLfloat z);
	static gl_translatef_func* glTranslatef;
	typedef void (FDYNGL_APIENTRY gl_vertex2d_func)(GLdouble x, GLdouble y);
	static gl_vertex2d_func* glVertex2d;
	typedef void (FDYNGL_APIENTRY gl_vertex2dv_func)(const GLdouble *v);
	static gl_vertex2dv_func* glVertex2dv;
	typedef void (FDYNGL_APIENTRY gl_vertex2f_func)(GLfloat x, GLfloat y);
	static gl_vertex2f_func* glVertex2f;
	typedef void (FDYNGL_APIENTRY gl_vertex2fv_func)(const GLfloat *v);
	static gl_vertex2fv_func* glVertex2fv;
	typedef void (FDYNGL_APIENTRY gl_vertex2i_func)(GLint x, GLint y);
	static gl_vertex2i_func* glVertex2i;
	typedef void (FDYNGL_APIENTRY gl_vertex2iv_func)(const GLint *v);
	static gl_vertex2iv_func* glVertex2iv;
	typedef void (FDYNGL_APIENTRY gl_vertex2s_func)(GLshort x, GLshort y);
	static gl_vertex2s_func* glVertex2s;
	typedef void (FDYNGL_APIENTRY gl_vertex2sv_func)(const GLshort *v);
	static gl_vertex2sv_func* glVertex2sv;
	typedef void (FDYNGL_APIENTRY gl_vertex3d_func)(GLdouble x, GLdouble y, GLdouble z);
	static gl_vertex3d_func* glVertex3d;
	typedef void (FDYNGL_APIENTRY gl_vertex3dv_func)(const GLdouble *v);
	static gl_vertex3dv_func* glVertex3dv;
	typedef void (FDYNGL_APIENTRY gl_vertex3f_func)(GLfloat x, GLfloat y, GLfloat z);
	static gl_vertex3f_func* glVertex3f;
	typedef void (FDYNGL_APIENTRY gl_vertex3fv_func)(const GLfloat *v);
	static gl_vertex3fv_func* glVertex3fv;
	typedef void (FDYNGL_APIENTRY gl_vertex3i_func)(GLint x, GLint y, GLint z);
	static gl_vertex3i_func* glVertex3i;
	typedef void (FDYNGL_APIENTRY gl_vertex3iv_func)(const GLint *v);
	static gl_vertex3iv_func* glVertex3iv;
	typedef void (FDYNGL_APIENTRY gl_vertex3s_func)(GLshort x, GLshort y, GLshort z);
	static gl_vertex3s_func* glVertex3s;
	typedef void (FDYNGL_APIENTRY gl_vertex3sv_func)(const GLshort *v);
	static gl_vertex3sv_func* glVertex3sv;
	typedef void (FDYNGL_APIENTRY gl_vertex4d_func)(GLdouble x, GLdouble y, GLdouble z, GLdouble w);
	static gl_vertex4d_func* glVertex4d;
	typedef void (FDYNGL_APIENTRY gl_vertex4dv_func)(const GLdouble *v);
	static gl_vertex4dv_func* glVertex4dv;
	typedef void (FDYNGL_APIENTRY gl_vertex4f_func)(GLfloat x, GLfloat y, GLfloat z, GLfloat w);
	static gl_vertex4f_func* glVertex4f;
	typedef void (FDYNGL_APIENTRY gl_vertex4fv_func)(const GLfloat *v);
	static gl_vertex4fv_func* glVertex4fv;
	typedef void (FDYNGL_APIENTRY gl_vertex4i_func)(GLint x, GLint y, GLint z, GLint w);
	static gl_vertex4i_func* glVertex4i;
	typedef void (FDYNGL_APIENTRY gl_vertex4iv_func)(const GLint *v);
	static gl_vertex4iv_func* glVertex4iv;
	typedef void (FDYNGL_APIENTRY gl_vertex4s_func)(GLshort x, GLshort y, GLshort z, GLshort w);
	static gl_vertex4s_func* glVertex4s;
	typedef void (FDYNGL_APIENTRY gl_vertex4sv_func)(const GLshort *v);
	static gl_vertex4sv_func* glVertex4sv;
	typedef void (FDYNGL_APIENTRY gl_vertex_pointer_func)(GLint size, GLenum type, GLsizei stride, const GLvoid *pointer);
	static gl_vertex_pointer_func* glVertexPointer;
	typedef void (FDYNGL_APIENTRY gl_viewport_func)(GLint x, GLint y, GLsizei width, GLsizei height);
	static gl_viewport_func* glViewport;
#	endif // GL_VERSION_1_1
#	ifndef GL_VERSION_1_2
#		define GL_VERSION_1_2 1
	static bool isGL_VERSION_1_2;
	static const uint32_t GL_UNSIGNED_BYTE_3_3_2 = 0x8032;
	static const uint32_t GL_UNSIGNED_SHORT_4_4_4_4 = 0x8033;
	static const uint32_t GL_UNSIGNED_SHORT_5_5_5_1 = 0x8034;
	static const uint32_t GL_UNSIGNED_INT_8_8_8_8 = 0x8035;
	static const uint32_t GL_UNSIGNED_INT_10_10_10_2 = 0x8036;
	static const uint32_t GL_TEXTURE_BINDING_3D = 0x806A;
	static const uint32_t GL_PACK_SKIP_IMAGES = 0x806B;
	static const uint32_t GL_PACK_IMAGE_HEIGHT = 0x806C;
	static const uint32_t GL_UNPACK_SKIP_IMAGES = 0x806D;
	static const uint32_t GL_UNPACK_IMAGE_HEIGHT = 0x806E;
	static const uint32_t GL_TEXTURE_3D = 0x806F;
	static const uint32_t GL_PROXY_TEXTURE_3D = 0x8070;
	static const uint32_t GL_TEXTURE_DEPTH = 0x8071;
	static const uint32_t GL_TEXTURE_WRAP_R = 0x8072;
	static const uint32_t GL_MAX_3D_TEXTURE_SIZE = 0x8073;
	static const uint32_t GL_UNSIGNED_BYTE_2_3_3_REV = 0x8362;
	static const uint32_t GL_UNSIGNED_SHORT_5_6_5 = 0x8363;
	static const uint32_t GL_UNSIGNED_SHORT_5_6_5_REV = 0x8364;
	static const uint32_t GL_UNSIGNED_SHORT_4_4_4_4_REV = 0x8365;
	static const uint32_t GL_UNSIGNED_SHORT_1_5_5_5_REV = 0x8366;
	static const uint32_t GL_UNSIGNED_INT_8_8_8_8_REV = 0x8367;
	static const uint32_t GL_UNSIGNED_INT_2_10_10_10_REV = 0x8368;
	static const uint32_t GL_BGR = 0x80E0;
	static const uint32_t GL_BGRA = 0x80E1;
	static const uint32_t GL_MAX_ELEMENTS_VERTICES = 0x80E8;
	static const uint32_t GL_MAX_ELEMENTS_INDICES = 0x80E9;
	static const uint32_t GL_CLAMP_TO_EDGE = 0x812F;
	static const uint32_t GL_TEXTURE_MIN_LOD = 0x813A;
	static const uint32_t GL_TEXTURE_MAX_LOD = 0x813B;
	static const uint32_t GL_TEXTURE_BASE_LEVEL = 0x813C;
	static const uint32_t GL_TEXTURE_MAX_LEVEL = 0x813D;
	static const uint32_t GL_SMOOTH_POINT_SIZE_RANGE = 0x0B12;
	static const uint32_t GL_SMOOTH_POINT_SIZE_GRANULARITY = 0x0B13;
	static const uint32_t GL_SMOOTH_LINE_WIDTH_RANGE = 0x0B22;
	static const uint32_t GL_SMOOTH_LINE_WIDTH_GRANULARITY = 0x0B23;
	static const uint32_t GL_ALIASED_LINE_WIDTH_RANGE = 0x846E;
	static const uint32_t GL_RESCALE_NORMAL = 0x803A;
	static const uint32_t GL_LIGHT_MODEL_COLOR_CONTROL = 0x81F8;
	static const uint32_t GL_SINGLE_COLOR = 0x81F9;
	static const uint32_t GL_SEPARATE_SPECULAR_COLOR = 0x81FA;
	static const uint32_t GL_ALIASED_POINT_SIZE_RANGE = 0x846D;
	typedef void (FDYNGL_APIENTRY gl_draw_range_elements_func)(GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, const void *indices);
	static gl_draw_range_elements_func* glDrawRangeElements;
	typedef void (FDYNGL_APIENTRY gl_tex_image3d_func)(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLenum format, GLenum type, const void *pixels);
	static gl_tex_image3d_func* glTexImage3D;
	typedef void (FDYNGL_APIENTRY gl_tex_sub_image3d_func)(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const void *pixels);
	static gl_tex_sub_image3d_func* glTexSubImage3D;
	typedef void (FDYNGL_APIENTRY gl_copy_tex_sub_image3d_func)(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLint x, GLint y, GLsizei width, GLsizei height);
	static gl_copy_tex_sub_image3d_func* glCopyTexSubImage3D;
#	endif // GL_VERSION_1_2
#	ifndef GL_VERSION_1_3
#		define GL_VERSION_1_3 1
	static bool isGL_VERSION_1_3;
	static const uint32_t GL_TEXTURE0 = 0x84C0;
	static const uint32_t GL_TEXTURE1 = 0x84C1;
	static const uint32_t GL_TEXTURE2 = 0x84C2;
	static const uint32_t GL_TEXTURE3 = 0x84C3;
	static const uint32_t GL_TEXTURE4 = 0x84C4;
	static const uint32_t GL_TEXTURE5 = 0x84C5;
	static const uint32_t GL_TEXTURE6 = 0x84C6;
	static const uint32_t GL_TEXTURE7 = 0x84C7;
	static const uint32_t GL_TEXTURE8 = 0x84C8;
	static const uint32_t GL_TEXTURE9 = 0x84C9;
	static const uint32_t GL_TEXTURE10 = 0x84CA;
	static const uint32_t GL_TEXTURE11 = 0x84CB;
	static const uint32_t GL_TEXTURE12 = 0x84CC;
	static const uint32_t GL_TEXTURE13 = 0x84CD;
	static const uint32_t GL_TEXTURE14 = 0x84CE;
	static const uint32_t GL_TEXTURE15 = 0x84CF;
	static const uint32_t GL_TEXTURE16 = 0x84D0;
	static const uint32_t GL_TEXTURE17 = 0x84D1;
	static const uint32_t GL_TEXTURE18 = 0x84D2;
	static const uint32_t GL_TEXTURE19 = 0x84D3;
	static const uint32_t GL_TEXTURE20 = 0x84D4;
	static const uint32_t GL_TEXTURE21 = 0x84D5;
	static const uint32_t GL_TEXTURE22 = 0x84D6;
	static const uint32_t GL_TEXTURE23 = 0x84D7;
	static const uint32_t GL_TEXTURE24 = 0x84D8;
	static const uint32_t GL_TEXTURE25 = 0x84D9;
	static const uint32_t GL_TEXTURE26 = 0x84DA;
	static const uint32_t GL_TEXTURE27 = 0x84DB;
	static const uint32_t GL_TEXTURE28 = 0x84DC;
	static const uint32_t GL_TEXTURE29 = 0x84DD;
	static const uint32_t GL_TEXTURE30 = 0x84DE;
	static const uint32_t GL_TEXTURE31 = 0x84DF;
	static const uint32_t GL_ACTIVE_TEXTURE = 0x84E0;
	static const uint32_t GL_MULTISAMPLE = 0x809D;
	static const uint32_t GL_SAMPLE_ALPHA_TO_COVERAGE = 0x809E;
	static const uint32_t GL_SAMPLE_ALPHA_TO_ONE = 0x809F;
	static const uint32_t GL_SAMPLE_COVERAGE = 0x80A0;
	static const uint32_t GL_SAMPLE_BUFFERS = 0x80A8;
	static const uint32_t GL_SAMPLES = 0x80A9;
	static const uint32_t GL_SAMPLE_COVERAGE_VALUE = 0x80AA;
	static const uint32_t GL_SAMPLE_COVERAGE_INVERT = 0x80AB;
	static const uint32_t GL_TEXTURE_CUBE_MAP = 0x8513;
	static const uint32_t GL_TEXTURE_BINDING_CUBE_MAP = 0x8514;
	static const uint32_t GL_TEXTURE_CUBE_MAP_POSITIVE_X = 0x8515;
	static const uint32_t GL_TEXTURE_CUBE_MAP_NEGATIVE_X = 0x8516;
	static const uint32_t GL_TEXTURE_CUBE_MAP_POSITIVE_Y = 0x8517;
	static const uint32_t GL_TEXTURE_CUBE_MAP_NEGATIVE_Y = 0x8518;
	static const uint32_t GL_TEXTURE_CUBE_MAP_POSITIVE_Z = 0x8519;
	static const uint32_t GL_TEXTURE_CUBE_MAP_NEGATIVE_Z = 0x851A;
	static const uint32_t GL_PROXY_TEXTURE_CUBE_MAP = 0x851B;
	static const uint32_t GL_MAX_CUBE_MAP_TEXTURE_SIZE = 0x851C;
	static const uint32_t GL_COMPRESSED_RGB = 0x84ED;
	static const uint32_t GL_COMPRESSED_RGBA = 0x84EE;
	static const uint32_t GL_TEXTURE_COMPRESSION_HINT = 0x84EF;
	static const uint32_t GL_TEXTURE_COMPRESSED_IMAGE_SIZE = 0x86A0;
	static const uint32_t GL_TEXTURE_COMPRESSED = 0x86A1;
	static const uint32_t GL_NUM_COMPRESSED_TEXTURE_FORMATS = 0x86A2;
	static const uint32_t GL_COMPRESSED_TEXTURE_FORMATS = 0x86A3;
	static const uint32_t GL_CLAMP_TO_BORDER = 0x812D;
	static const uint32_t GL_CLIENT_ACTIVE_TEXTURE = 0x84E1;
	static const uint32_t GL_MAX_TEXTURE_UNITS = 0x84E2;
	static const uint32_t GL_TRANSPOSE_MODELVIEW_MATRIX = 0x84E3;
	static const uint32_t GL_TRANSPOSE_PROJECTION_MATRIX = 0x84E4;
	static const uint32_t GL_TRANSPOSE_TEXTURE_MATRIX = 0x84E5;
	static const uint32_t GL_TRANSPOSE_COLOR_MATRIX = 0x84E6;
	static const uint32_t GL_MULTISAMPLE_BIT = 0x20000000;
	static const uint32_t GL_NORMAL_MAP = 0x8511;
	static const uint32_t GL_REFLECTION_MAP = 0x8512;
	static const uint32_t GL_COMPRESSED_ALPHA = 0x84E9;
	static const uint32_t GL_COMPRESSED_LUMINANCE = 0x84EA;
	static const uint32_t GL_COMPRESSED_LUMINANCE_ALPHA = 0x84EB;
	static const uint32_t GL_COMPRESSED_INTENSITY = 0x84EC;
	static const uint32_t GL_COMBINE = 0x8570;
	static const uint32_t GL_COMBINE_RGB = 0x8571;
	static const uint32_t GL_COMBINE_ALPHA = 0x8572;
	static const uint32_t GL_SOURCE0_RGB = 0x8580;
	static const uint32_t GL_SOURCE1_RGB = 0x8581;
	static const uint32_t GL_SOURCE2_RGB = 0x8582;
	static const uint32_t GL_SOURCE0_ALPHA = 0x8588;
	static const uint32_t GL_SOURCE1_ALPHA = 0x8589;
	static const uint32_t GL_SOURCE2_ALPHA = 0x858A;
	static const uint32_t GL_OPERAND0_RGB = 0x8590;
	static const uint32_t GL_OPERAND1_RGB = 0x8591;
	static const uint32_t GL_OPERAND2_RGB = 0x8592;
	static const uint32_t GL_OPERAND0_ALPHA = 0x8598;
	static const uint32_t GL_OPERAND1_ALPHA = 0x8599;
	static const uint32_t GL_OPERAND2_ALPHA = 0x859A;
	static const uint32_t GL_RGB_SCALE = 0x8573;
	static const uint32_t GL_ADD_SIGNED = 0x8574;
	static const uint32_t GL_INTERPOLATE = 0x8575;
	static const uint32_t GL_SUBTRACT = 0x84E7;
	static const uint32_t GL_CONSTANT = 0x8576;
	static const uint32_t GL_PRIMARY_COLOR = 0x8577;
	static const uint32_t GL_PREVIOUS = 0x8578;
	static const uint32_t GL_DOT3_RGB = 0x86AE;
	static const uint32_t GL_DOT3_RGBA = 0x86AF;
	typedef void (FDYNGL_APIENTRY gl_active_texture_func)(GLenum texture);
	static gl_active_texture_func* glActiveTexture;
	typedef void (FDYNGL_APIENTRY gl_sample_coverage_func)(GLfloat value, GLboolean invert);
	static gl_sample_coverage_func* glSampleCoverage;
	typedef void (FDYNGL_APIENTRY gl_compressed_tex_image3d_func)(GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLsizei imageSize, const void *data);
	static gl_compressed_tex_image3d_func* glCompressedTexImage3D;
	typedef void (FDYNGL_APIENTRY gl_compressed_tex_image2d_func)(GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLint border, GLsizei imageSize, const void *data);
	static gl_compressed_tex_image2d_func* glCompressedTexImage2D;
	typedef void (FDYNGL_APIENTRY gl_compressed_tex_image1d_func)(GLenum target, GLint level, GLenum internalformat, GLsizei width, GLint border, GLsizei imageSize, const void *data);
	static gl_compressed_tex_image1d_func* glCompressedTexImage1D;
	typedef void (FDYNGL_APIENTRY gl_compressed_tex_sub_image3d_func)(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLsizei imageSize, const void *data);
	static gl_compressed_tex_sub_image3d_func* glCompressedTexSubImage3D;
	typedef void (FDYNGL_APIENTRY gl_compressed_tex_sub_image2d_func)(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLsizei imageSize, const void *data);
	static gl_compressed_tex_sub_image2d_func* glCompressedTexSubImage2D;
	typedef void (FDYNGL_APIENTRY gl_compressed_tex_sub_image1d_func)(GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLsizei imageSize, const void *data);
	static gl_compressed_tex_sub_image1d_func* glCompressedTexSubImage1D;
	typedef void (FDYNGL_APIENTRY gl_get_compressed_tex_image_func)(GLenum target, GLint level, void *img);
	static gl_get_compressed_tex_image_func* glGetCompressedTexImage;
	typedef void (FDYNGL_APIENTRY gl_client_active_texture_func)(GLenum texture);
	static gl_client_active_texture_func* glClientActiveTexture;
	typedef void (FDYNGL_APIENTRY gl_multi_tex_coord1d_func)(GLenum target, GLdouble s);
	static gl_multi_tex_coord1d_func* glMultiTexCoord1d;
	typedef void (FDYNGL_APIENTRY gl_multi_tex_coord1dv_func)(GLenum target, const GLdouble *v);
	static gl_multi_tex_coord1dv_func* glMultiTexCoord1dv;
	typedef void (FDYNGL_APIENTRY gl_multi_tex_coord1f_func)(GLenum target, GLfloat s);
	static gl_multi_tex_coord1f_func* glMultiTexCoord1f;
	typedef void (FDYNGL_APIENTRY gl_multi_tex_coord1fv_func)(GLenum target, const GLfloat *v);
	static gl_multi_tex_coord1fv_func* glMultiTexCoord1fv;
	typedef void (FDYNGL_APIENTRY gl_multi_tex_coord1i_func)(GLenum target, GLint s);
	static gl_multi_tex_coord1i_func* glMultiTexCoord1i;
	typedef void (FDYNGL_APIENTRY gl_multi_tex_coord1iv_func)(GLenum target, const GLint *v);
	static gl_multi_tex_coord1iv_func* glMultiTexCoord1iv;
	typedef void (FDYNGL_APIENTRY gl_multi_tex_coord1s_func)(GLenum target, GLshort s);
	static gl_multi_tex_coord1s_func* glMultiTexCoord1s;
	typedef void (FDYNGL_APIENTRY gl_multi_tex_coord1sv_func)(GLenum target, const GLshort *v);
	static gl_multi_tex_coord1sv_func* glMultiTexCoord1sv;
	typedef void (FDYNGL_APIENTRY gl_multi_tex_coord2d_func)(GLenum target, GLdouble s, GLdouble t);
	static gl_multi_tex_coord2d_func* glMultiTexCoord2d;
	typedef void (FDYNGL_APIENTRY gl_multi_tex_coord2dv_func)(GLenum target, const GLdouble *v);
	static gl_multi_tex_coord2dv_func* glMultiTexCoord2dv;
	typedef void (FDYNGL_APIENTRY gl_multi_tex_coord2f_func)(GLenum target, GLfloat s, GLfloat t);
	static gl_multi_tex_coord2f_func* glMultiTexCoord2f;
	typedef void (FDYNGL_APIENTRY gl_multi_tex_coord2fv_func)(GLenum target, const GLfloat *v);
	static gl_multi_tex_coord2fv_func* glMultiTexCoord2fv;
	typedef void (FDYNGL_APIENTRY gl_multi_tex_coord2i_func)(GLenum target, GLint s, GLint t);
	static gl_multi_tex_coord2i_func* glMultiTexCoord2i;
	typedef void (FDYNGL_APIENTRY gl_multi_tex_coord2iv_func)(GLenum target, const GLint *v);
	static gl_multi_tex_coord2iv_func* glMultiTexCoord2iv;
	typedef void (FDYNGL_APIENTRY gl_multi_tex_coord2s_func)(GLenum target, GLshort s, GLshort t);
	static gl_multi_tex_coord2s_func* glMultiTexCoord2s;
	typedef void (FDYNGL_APIENTRY gl_multi_tex_coord2sv_func)(GLenum target, const GLshort *v);
	static gl_multi_tex_coord2sv_func* glMultiTexCoord2sv;
	typedef void (FDYNGL_APIENTRY gl_multi_tex_coord3d_func)(GLenum target, GLdouble s, GLdouble t, GLdouble r);
	static gl_multi_tex_coord3d_func* glMultiTexCoord3d;
	typedef void (FDYNGL_APIENTRY gl_multi_tex_coord3dv_func)(GLenum target, const GLdouble *v);
	static gl_multi_tex_coord3dv_func* glMultiTexCoord3dv;
	typedef void (FDYNGL_APIENTRY gl_multi_tex_coord3f_func)(GLenum target, GLfloat s, GLfloat t, GLfloat r);
	static gl_multi_tex_coord3f_func* glMultiTexCoord3f;
	typedef void (FDYNGL_APIENTRY gl_multi_tex_coord3fv_func)(GLenum target, const GLfloat *v);
	static gl_multi_tex_coord3fv_func* glMultiTexCoord3fv;
	typedef void (FDYNGL_APIENTRY gl_multi_tex_coord3i_func)(GLenum target, GLint s, GLint t, GLint r);
	static gl_multi_tex_coord3i_func* glMultiTexCoord3i;
	typedef void (FDYNGL_APIENTRY gl_multi_tex_coord3iv_func)(GLenum target, const GLint *v);
	static gl_multi_tex_coord3iv_func* glMultiTexCoord3iv;
	typedef void (FDYNGL_APIENTRY gl_multi_tex_coord3s_func)(GLenum target, GLshort s, GLshort t, GLshort r);
	static gl_multi_tex_coord3s_func* glMultiTexCoord3s;
	typedef void (FDYNGL_APIENTRY gl_multi_tex_coord3sv_func)(GLenum target, const GLshort *v);
	static gl_multi_tex_coord3sv_func* glMultiTexCoord3sv;
	typedef void (FDYNGL_APIENTRY gl_multi_tex_coord4d_func)(GLenum target, GLdouble s, GLdouble t, GLdouble r, GLdouble q);
	static gl_multi_tex_coord4d_func* glMultiTexCoord4d;
	typedef void (FDYNGL_APIENTRY gl_multi_tex_coord4dv_func)(GLenum target, const GLdouble *v);
	static gl_multi_tex_coord4dv_func* glMultiTexCoord4dv;
	typedef void (FDYNGL_APIENTRY gl_multi_tex_coord4f_func)(GLenum target, GLfloat s, GLfloat t, GLfloat r, GLfloat q);
	static gl_multi_tex_coord4f_func* glMultiTexCoord4f;
	typedef void (FDYNGL_APIENTRY gl_multi_tex_coord4fv_func)(GLenum target, const GLfloat *v);
	static gl_multi_tex_coord4fv_func* glMultiTexCoord4fv;
	typedef void (FDYNGL_APIENTRY gl_multi_tex_coord4i_func)(GLenum target, GLint s, GLint t, GLint r, GLint q);
	static gl_multi_tex_coord4i_func* glMultiTexCoord4i;
	typedef void (FDYNGL_APIENTRY gl_multi_tex_coord4iv_func)(GLenum target, const GLint *v);
	static gl_multi_tex_coord4iv_func* glMultiTexCoord4iv;
	typedef void (FDYNGL_APIENTRY gl_multi_tex_coord4s_func)(GLenum target, GLshort s, GLshort t, GLshort r, GLshort q);
	static gl_multi_tex_coord4s_func* glMultiTexCoord4s;
	typedef void (FDYNGL_APIENTRY gl_multi_tex_coord4sv_func)(GLenum target, const GLshort *v);
	static gl_multi_tex_coord4sv_func* glMultiTexCoord4sv;
	typedef void (FDYNGL_APIENTRY gl_load_transpose_matrixf_func)(const GLfloat *m);
	static gl_load_transpose_matrixf_func* glLoadTransposeMatrixf;
	typedef void (FDYNGL_APIENTRY gl_load_transpose_matrixd_func)(const GLdouble *m);
	static gl_load_transpose_matrixd_func* glLoadTransposeMatrixd;
	typedef void (FDYNGL_APIENTRY gl_mult_transpose_matrixf_func)(const GLfloat *m);
	static gl_mult_transpose_matrixf_func* glMultTransposeMatrixf;
	typedef void (FDYNGL_APIENTRY gl_mult_transpose_matrixd_func)(const GLdouble *m);
	static gl_mult_transpose_matrixd_func* glMultTransposeMatrixd;
#	endif // GL_VERSION_1_3
#	ifndef GL_VERSION_1_4
#		define GL_VERSION_1_4 1
	static bool isGL_VERSION_1_4;
	static const uint32_t GL_BLEND_DST_RGB = 0x80C8;
	static const uint32_t GL_BLEND_SRC_RGB = 0x80C9;
	static const uint32_t GL_BLEND_DST_ALPHA = 0x80CA;
	static const uint32_t GL_BLEND_SRC_ALPHA = 0x80CB;
	static const uint32_t GL_POINT_FADE_THRESHOLD_SIZE = 0x8128;
	static const uint32_t GL_DEPTH_COMPONENT16 = 0x81A5;
	static const uint32_t GL_DEPTH_COMPONENT24 = 0x81A6;
	static const uint32_t GL_DEPTH_COMPONENT32 = 0x81A7;
	static const uint32_t GL_MIRRORED_REPEAT = 0x8370;
	static const uint32_t GL_MAX_TEXTURE_LOD_BIAS = 0x84FD;
	static const uint32_t GL_TEXTURE_LOD_BIAS = 0x8501;
	static const uint32_t GL_INCR_WRAP = 0x8507;
	static const uint32_t GL_DECR_WRAP = 0x8508;
	static const uint32_t GL_TEXTURE_DEPTH_SIZE = 0x884A;
	static const uint32_t GL_TEXTURE_COMPARE_MODE = 0x884C;
	static const uint32_t GL_TEXTURE_COMPARE_FUNC = 0x884D;
	static const uint32_t GL_POINT_SIZE_MIN = 0x8126;
	static const uint32_t GL_POINT_SIZE_MAX = 0x8127;
	static const uint32_t GL_POINT_DISTANCE_ATTENUATION = 0x8129;
	static const uint32_t GL_GENERATE_MIPMAP = 0x8191;
	static const uint32_t GL_GENERATE_MIPMAP_HINT = 0x8192;
	static const uint32_t GL_FOG_COORDINATE_SOURCE = 0x8450;
	static const uint32_t GL_FOG_COORDINATE = 0x8451;
	static const uint32_t GL_FRAGMENT_DEPTH = 0x8452;
	static const uint32_t GL_CURRENT_FOG_COORDINATE = 0x8453;
	static const uint32_t GL_FOG_COORDINATE_ARRAY_TYPE = 0x8454;
	static const uint32_t GL_FOG_COORDINATE_ARRAY_STRIDE = 0x8455;
	static const uint32_t GL_FOG_COORDINATE_ARRAY_POINTER = 0x8456;
	static const uint32_t GL_FOG_COORDINATE_ARRAY = 0x8457;
	static const uint32_t GL_COLOR_SUM = 0x8458;
	static const uint32_t GL_CURRENT_SECONDARY_COLOR = 0x8459;
	static const uint32_t GL_SECONDARY_COLOR_ARRAY_SIZE = 0x845A;
	static const uint32_t GL_SECONDARY_COLOR_ARRAY_TYPE = 0x845B;
	static const uint32_t GL_SECONDARY_COLOR_ARRAY_STRIDE = 0x845C;
	static const uint32_t GL_SECONDARY_COLOR_ARRAY_POINTER = 0x845D;
	static const uint32_t GL_SECONDARY_COLOR_ARRAY = 0x845E;
	static const uint32_t GL_TEXTURE_FILTER_CONTROL = 0x8500;
	static const uint32_t GL_DEPTH_TEXTURE_MODE = 0x884B;
	static const uint32_t GL_COMPARE_R_TO_TEXTURE = 0x884E;
	static const uint32_t GL_BLEND_COLOR = 0x8005;
	static const uint32_t GL_BLEND_EQUATION = 0x8009;
	static const uint32_t GL_CONSTANT_COLOR = 0x8001;
	static const uint32_t GL_ONE_MINUS_CONSTANT_COLOR = 0x8002;
	static const uint32_t GL_CONSTANT_ALPHA = 0x8003;
	static const uint32_t GL_ONE_MINUS_CONSTANT_ALPHA = 0x8004;
	static const uint32_t GL_FUNC_ADD = 0x8006;
	static const uint32_t GL_FUNC_REVERSE_SUBTRACT = 0x800B;
	static const uint32_t GL_FUNC_SUBTRACT = 0x800A;
	static const uint32_t GL_MIN = 0x8007;
	static const uint32_t GL_MAX = 0x8008;
	typedef void (FDYNGL_APIENTRY gl_blend_func_separate_func)(GLenum sfactorRGB, GLenum dfactorRGB, GLenum sfactorAlpha, GLenum dfactorAlpha);
	static gl_blend_func_separate_func* glBlendFuncSeparate;
	typedef void (FDYNGL_APIENTRY gl_multi_draw_arrays_func)(GLenum mode, const GLint *first, const GLsizei *count, GLsizei drawcount);
	static gl_multi_draw_arrays_func* glMultiDrawArrays;
	typedef void (FDYNGL_APIENTRY gl_multi_draw_elements_func)(GLenum mode, const GLsizei *count, GLenum type, const void *const*indices, GLsizei drawcount);
	static gl_multi_draw_elements_func* glMultiDrawElements;
	typedef void (FDYNGL_APIENTRY gl_point_parameterf_func)(GLenum pname, GLfloat param);
	static gl_point_parameterf_func* glPointParameterf;
	typedef void (FDYNGL_APIENTRY gl_point_parameterfv_func)(GLenum pname, const GLfloat *params);
	static gl_point_parameterfv_func* glPointParameterfv;
	typedef void (FDYNGL_APIENTRY gl_point_parameteri_func)(GLenum pname, GLint param);
	static gl_point_parameteri_func* glPointParameteri;
	typedef void (FDYNGL_APIENTRY gl_point_parameteriv_func)(GLenum pname, const GLint *params);
	static gl_point_parameteriv_func* glPointParameteriv;
	typedef void (FDYNGL_APIENTRY gl_fog_coordf_func)(GLfloat coord);
	static gl_fog_coordf_func* glFogCoordf;
	typedef void (FDYNGL_APIENTRY gl_fog_coordfv_func)(const GLfloat *coord);
	static gl_fog_coordfv_func* glFogCoordfv;
	typedef void (FDYNGL_APIENTRY gl_fog_coordd_func)(GLdouble coord);
	static gl_fog_coordd_func* glFogCoordd;
	typedef void (FDYNGL_APIENTRY gl_fog_coorddv_func)(const GLdouble *coord);
	static gl_fog_coorddv_func* glFogCoorddv;
	typedef void (FDYNGL_APIENTRY gl_fog_coord_pointer_func)(GLenum type, GLsizei stride, const void *pointer);
	static gl_fog_coord_pointer_func* glFogCoordPointer;
	typedef void (FDYNGL_APIENTRY gl_secondary_color3b_func)(GLbyte red, GLbyte green, GLbyte blue);
	static gl_secondary_color3b_func* glSecondaryColor3b;
	typedef void (FDYNGL_APIENTRY gl_secondary_color3bv_func)(const GLbyte *v);
	static gl_secondary_color3bv_func* glSecondaryColor3bv;
	typedef void (FDYNGL_APIENTRY gl_secondary_color3d_func)(GLdouble red, GLdouble green, GLdouble blue);
	static gl_secondary_color3d_func* glSecondaryColor3d;
	typedef void (FDYNGL_APIENTRY gl_secondary_color3dv_func)(const GLdouble *v);
	static gl_secondary_color3dv_func* glSecondaryColor3dv;
	typedef void (FDYNGL_APIENTRY gl_secondary_color3f_func)(GLfloat red, GLfloat green, GLfloat blue);
	static gl_secondary_color3f_func* glSecondaryColor3f;
	typedef void (FDYNGL_APIENTRY gl_secondary_color3fv_func)(const GLfloat *v);
	static gl_secondary_color3fv_func* glSecondaryColor3fv;
	typedef void (FDYNGL_APIENTRY gl_secondary_color3i_func)(GLint red, GLint green, GLint blue);
	static gl_secondary_color3i_func* glSecondaryColor3i;
	typedef void (FDYNGL_APIENTRY gl_secondary_color3iv_func)(const GLint *v);
	static gl_secondary_color3iv_func* glSecondaryColor3iv;
	typedef void (FDYNGL_APIENTRY gl_secondary_color3s_func)(GLshort red, GLshort green, GLshort blue);
	static gl_secondary_color3s_func* glSecondaryColor3s;
	typedef void (FDYNGL_APIENTRY gl_secondary_color3sv_func)(const GLshort *v);
	static gl_secondary_color3sv_func* glSecondaryColor3sv;
	typedef void (FDYNGL_APIENTRY gl_secondary_color3ub_func)(GLubyte red, GLubyte green, GLubyte blue);
	static gl_secondary_color3ub_func* glSecondaryColor3ub;
	typedef void (FDYNGL_APIENTRY gl_secondary_color3ubv_func)(const GLubyte *v);
	static gl_secondary_color3ubv_func* glSecondaryColor3ubv;
	typedef void (FDYNGL_APIENTRY gl_secondary_color3ui_func)(GLuint red, GLuint green, GLuint blue);
	static gl_secondary_color3ui_func* glSecondaryColor3ui;
	typedef void (FDYNGL_APIENTRY gl_secondary_color3uiv_func)(const GLuint *v);
	static gl_secondary_color3uiv_func* glSecondaryColor3uiv;
	typedef void (FDYNGL_APIENTRY gl_secondary_color3us_func)(GLushort red, GLushort green, GLushort blue);
	static gl_secondary_color3us_func* glSecondaryColor3us;
	typedef void (FDYNGL_APIENTRY gl_secondary_color3usv_func)(const GLushort *v);
	static gl_secondary_color3usv_func* glSecondaryColor3usv;
	typedef void (FDYNGL_APIENTRY gl_secondary_color_pointer_func)(GLint size, GLenum type, GLsizei stride, const void *pointer);
	static gl_secondary_color_pointer_func* glSecondaryColorPointer;
	typedef void (FDYNGL_APIENTRY gl_window_pos2d_func)(GLdouble x, GLdouble y);
	static gl_window_pos2d_func* glWindowPos2d;
	typedef void (FDYNGL_APIENTRY gl_window_pos2dv_func)(const GLdouble *v);
	static gl_window_pos2dv_func* glWindowPos2dv;
	typedef void (FDYNGL_APIENTRY gl_window_pos2f_func)(GLfloat x, GLfloat y);
	static gl_window_pos2f_func* glWindowPos2f;
	typedef void (FDYNGL_APIENTRY gl_window_pos2fv_func)(const GLfloat *v);
	static gl_window_pos2fv_func* glWindowPos2fv;
	typedef void (FDYNGL_APIENTRY gl_window_pos2i_func)(GLint x, GLint y);
	static gl_window_pos2i_func* glWindowPos2i;
	typedef void (FDYNGL_APIENTRY gl_window_pos2iv_func)(const GLint *v);
	static gl_window_pos2iv_func* glWindowPos2iv;
	typedef void (FDYNGL_APIENTRY gl_window_pos2s_func)(GLshort x, GLshort y);
	static gl_window_pos2s_func* glWindowPos2s;
	typedef void (FDYNGL_APIENTRY gl_window_pos2sv_func)(const GLshort *v);
	static gl_window_pos2sv_func* glWindowPos2sv;
	typedef void (FDYNGL_APIENTRY gl_window_pos3d_func)(GLdouble x, GLdouble y, GLdouble z);
	static gl_window_pos3d_func* glWindowPos3d;
	typedef void (FDYNGL_APIENTRY gl_window_pos3dv_func)(const GLdouble *v);
	static gl_window_pos3dv_func* glWindowPos3dv;
	typedef void (FDYNGL_APIENTRY gl_window_pos3f_func)(GLfloat x, GLfloat y, GLfloat z);
	static gl_window_pos3f_func* glWindowPos3f;
	typedef void (FDYNGL_APIENTRY gl_window_pos3fv_func)(const GLfloat *v);
	static gl_window_pos3fv_func* glWindowPos3fv;
	typedef void (FDYNGL_APIENTRY gl_window_pos3i_func)(GLint x, GLint y, GLint z);
	static gl_window_pos3i_func* glWindowPos3i;
	typedef void (FDYNGL_APIENTRY gl_window_pos3iv_func)(const GLint *v);
	static gl_window_pos3iv_func* glWindowPos3iv;
	typedef void (FDYNGL_APIENTRY gl_window_pos3s_func)(GLshort x, GLshort y, GLshort z);
	static gl_window_pos3s_func* glWindowPos3s;
	typedef void (FDYNGL_APIENTRY gl_window_pos3sv_func)(const GLshort *v);
	static gl_window_pos3sv_func* glWindowPos3sv;
	typedef void (FDYNGL_APIENTRY gl_blend_color_func)(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha);
	static gl_blend_color_func* glBlendColor;
	typedef void (FDYNGL_APIENTRY gl_blend_equation_func)(GLenum mode);
	static gl_blend_equation_func* glBlendEquation;
#	endif // GL_VERSION_1_4
#	ifndef GL_VERSION_1_5
#		define GL_VERSION_1_5 1
	static bool isGL_VERSION_1_5;
	typedef ptrdiff_t GLsizeiptr;
	typedef ptrdiff_t GLintptr;
	static const uint32_t GL_BUFFER_SIZE = 0x8764;
	static const uint32_t GL_BUFFER_USAGE = 0x8765;
	static const uint32_t GL_QUERY_COUNTER_BITS = 0x8864;
	static const uint32_t GL_CURRENT_QUERY = 0x8865;
	static const uint32_t GL_QUERY_RESULT = 0x8866;
	static const uint32_t GL_QUERY_RESULT_AVAILABLE = 0x8867;
	static const uint32_t GL_ARRAY_BUFFER = 0x8892;
	static const uint32_t GL_ELEMENT_ARRAY_BUFFER = 0x8893;
	static const uint32_t GL_ARRAY_BUFFER_BINDING = 0x8894;
	static const uint32_t GL_ELEMENT_ARRAY_BUFFER_BINDING = 0x8895;
	static const uint32_t GL_VERTEX_ATTRIB_ARRAY_BUFFER_BINDING = 0x889F;
	static const uint32_t GL_READ_ONLY = 0x88B8;
	static const uint32_t GL_WRITE_ONLY = 0x88B9;
	static const uint32_t GL_READ_WRITE = 0x88BA;
	static const uint32_t GL_BUFFER_ACCESS = 0x88BB;
	static const uint32_t GL_BUFFER_MAPPED = 0x88BC;
	static const uint32_t GL_BUFFER_MAP_POINTER = 0x88BD;
	static const uint32_t GL_STREAM_DRAW = 0x88E0;
	static const uint32_t GL_STREAM_READ = 0x88E1;
	static const uint32_t GL_STREAM_COPY = 0x88E2;
	static const uint32_t GL_STATIC_DRAW = 0x88E4;
	static const uint32_t GL_STATIC_READ = 0x88E5;
	static const uint32_t GL_STATIC_COPY = 0x88E6;
	static const uint32_t GL_DYNAMIC_DRAW = 0x88E8;
	static const uint32_t GL_DYNAMIC_READ = 0x88E9;
	static const uint32_t GL_DYNAMIC_COPY = 0x88EA;
	static const uint32_t GL_SAMPLES_PASSED = 0x8914;
	static const uint32_t GL_SRC1_ALPHA = 0x8589;
	static const uint32_t GL_VERTEX_ARRAY_BUFFER_BINDING = 0x8896;
	static const uint32_t GL_NORMAL_ARRAY_BUFFER_BINDING = 0x8897;
	static const uint32_t GL_COLOR_ARRAY_BUFFER_BINDING = 0x8898;
	static const uint32_t GL_INDEX_ARRAY_BUFFER_BINDING = 0x8899;
	static const uint32_t GL_TEXTURE_COORD_ARRAY_BUFFER_BINDING = 0x889A;
	static const uint32_t GL_EDGE_FLAG_ARRAY_BUFFER_BINDING = 0x889B;
	static const uint32_t GL_SECONDARY_COLOR_ARRAY_BUFFER_BINDING = 0x889C;
	static const uint32_t GL_FOG_COORDINATE_ARRAY_BUFFER_BINDING = 0x889D;
	static const uint32_t GL_WEIGHT_ARRAY_BUFFER_BINDING = 0x889E;
	static const uint32_t GL_FOG_COORD_SRC = 0x8450;
	static const uint32_t GL_FOG_COORD = 0x8451;
	static const uint32_t GL_CURRENT_FOG_COORD = 0x8453;
	static const uint32_t GL_FOG_COORD_ARRAY_TYPE = 0x8454;
	static const uint32_t GL_FOG_COORD_ARRAY_STRIDE = 0x8455;
	static const uint32_t GL_FOG_COORD_ARRAY_POINTER = 0x8456;
	static const uint32_t GL_FOG_COORD_ARRAY = 0x8457;
	static const uint32_t GL_FOG_COORD_ARRAY_BUFFER_BINDING = 0x889D;
	static const uint32_t GL_SRC0_RGB = 0x8580;
	static const uint32_t GL_SRC1_RGB = 0x8581;
	static const uint32_t GL_SRC2_RGB = 0x8582;
	static const uint32_t GL_SRC0_ALPHA = 0x8588;
	static const uint32_t GL_SRC2_ALPHA = 0x858A;
	typedef void (FDYNGL_APIENTRY gl_gen_queries_func)(GLsizei n, GLuint *ids);
	static gl_gen_queries_func* glGenQueries;
	typedef void (FDYNGL_APIENTRY gl_delete_queries_func)(GLsizei n, const GLuint *ids);
	static gl_delete_queries_func* glDeleteQueries;
	typedef GLboolean(FDYNGL_APIENTRY gl_is_query_func)(GLuint id);
	static gl_is_query_func* glIsQuery;
	typedef void (FDYNGL_APIENTRY gl_begin_query_func)(GLenum target, GLuint id);
	static gl_begin_query_func* glBeginQuery;
	typedef void (FDYNGL_APIENTRY gl_end_query_func)(GLenum target);
	static gl_end_query_func* glEndQuery;
	typedef void (FDYNGL_APIENTRY gl_get_queryiv_func)(GLenum target, GLenum pname, GLint *params);
	static gl_get_queryiv_func* glGetQueryiv;
	typedef void (FDYNGL_APIENTRY gl_get_query_objectiv_func)(GLuint id, GLenum pname, GLint *params);
	static gl_get_query_objectiv_func* glGetQueryObjectiv;
	typedef void (FDYNGL_APIENTRY gl_get_query_objectuiv_func)(GLuint id, GLenum pname, GLuint *params);
	static gl_get_query_objectuiv_func* glGetQueryObjectuiv;
	typedef void (FDYNGL_APIENTRY gl_bind_buffer_func)(GLenum target, GLuint buffer);
	static gl_bind_buffer_func* glBindBuffer;
	typedef void (FDYNGL_APIENTRY gl_delete_buffers_func)(GLsizei n, const GLuint *buffers);
	static gl_delete_buffers_func* glDeleteBuffers;
	typedef void (FDYNGL_APIENTRY gl_gen_buffers_func)(GLsizei n, GLuint *buffers);
	static gl_gen_buffers_func* glGenBuffers;
	typedef GLboolean(FDYNGL_APIENTRY gl_is_buffer_func)(GLuint buffer);
	static gl_is_buffer_func* glIsBuffer;
	typedef void (FDYNGL_APIENTRY gl_buffer_data_func)(GLenum target, GLsizeiptr size, const void *data, GLenum usage);
	static gl_buffer_data_func* glBufferData;
	typedef void (FDYNGL_APIENTRY gl_buffer_sub_data_func)(GLenum target, GLintptr offset, GLsizeiptr size, const void *data);
	static gl_buffer_sub_data_func* glBufferSubData;
	typedef void (FDYNGL_APIENTRY gl_get_buffer_sub_data_func)(GLenum target, GLintptr offset, GLsizeiptr size, void *data);
	static gl_get_buffer_sub_data_func* glGetBufferSubData;
	typedef void * (FDYNGL_APIENTRY gl_map_buffer_func)(GLenum target, GLenum access);
	static gl_map_buffer_func* glMapBuffer;
	typedef GLboolean(FDYNGL_APIENTRY gl_unmap_buffer_func)(GLenum target);
	static gl_unmap_buffer_func* glUnmapBuffer;
	typedef void (FDYNGL_APIENTRY gl_get_buffer_parameteriv_func)(GLenum target, GLenum pname, GLint *params);
	static gl_get_buffer_parameteriv_func* glGetBufferParameteriv;
	typedef void (FDYNGL_APIENTRY gl_get_buffer_pointerv_func)(GLenum target, GLenum pname, void **params);
	static gl_get_buffer_pointerv_func* glGetBufferPointerv;
#	endif // GL_VERSION_1_5
#	ifndef GL_VERSION_2_0
#		define GL_VERSION_2_0 1
	static bool isGL_VERSION_2_0;
	typedef char GLchar;
	static const uint32_t GL_BLEND_EQUATION_RGB = 0x8009;
	static const uint32_t GL_VERTEX_ATTRIB_ARRAY_ENABLED = 0x8622;
	static const uint32_t GL_VERTEX_ATTRIB_ARRAY_SIZE = 0x8623;
	static const uint32_t GL_VERTEX_ATTRIB_ARRAY_STRIDE = 0x8624;
	static const uint32_t GL_VERTEX_ATTRIB_ARRAY_TYPE = 0x8625;
	static const uint32_t GL_CURRENT_VERTEX_ATTRIB = 0x8626;
	static const uint32_t GL_VERTEX_PROGRAM_POINT_SIZE = 0x8642;
	static const uint32_t GL_VERTEX_ATTRIB_ARRAY_POINTER = 0x8645;
	static const uint32_t GL_STENCIL_BACK_FUNC = 0x8800;
	static const uint32_t GL_STENCIL_BACK_FAIL = 0x8801;
	static const uint32_t GL_STENCIL_BACK_PASS_DEPTH_FAIL = 0x8802;
	static const uint32_t GL_STENCIL_BACK_PASS_DEPTH_PASS = 0x8803;
	static const uint32_t GL_MAX_DRAW_BUFFERS = 0x8824;
	static const uint32_t GL_DRAW_BUFFER0 = 0x8825;
	static const uint32_t GL_DRAW_BUFFER1 = 0x8826;
	static const uint32_t GL_DRAW_BUFFER2 = 0x8827;
	static const uint32_t GL_DRAW_BUFFER3 = 0x8828;
	static const uint32_t GL_DRAW_BUFFER4 = 0x8829;
	static const uint32_t GL_DRAW_BUFFER5 = 0x882A;
	static const uint32_t GL_DRAW_BUFFER6 = 0x882B;
	static const uint32_t GL_DRAW_BUFFER7 = 0x882C;
	static const uint32_t GL_DRAW_BUFFER8 = 0x882D;
	static const uint32_t GL_DRAW_BUFFER9 = 0x882E;
	static const uint32_t GL_DRAW_BUFFER10 = 0x882F;
	static const uint32_t GL_DRAW_BUFFER11 = 0x8830;
	static const uint32_t GL_DRAW_BUFFER12 = 0x8831;
	static const uint32_t GL_DRAW_BUFFER13 = 0x8832;
	static const uint32_t GL_DRAW_BUFFER14 = 0x8833;
	static const uint32_t GL_DRAW_BUFFER15 = 0x8834;
	static const uint32_t GL_BLEND_EQUATION_ALPHA = 0x883D;
	static const uint32_t GL_MAX_VERTEX_ATTRIBS = 0x8869;
	static const uint32_t GL_VERTEX_ATTRIB_ARRAY_NORMALIZED = 0x886A;
	static const uint32_t GL_MAX_TEXTURE_IMAGE_UNITS = 0x8872;
	static const uint32_t GL_FRAGMENT_SHADER = 0x8B30;
	static const uint32_t GL_VERTEX_SHADER = 0x8B31;
	static const uint32_t GL_MAX_FRAGMENT_UNIFORM_COMPONENTS = 0x8B49;
	static const uint32_t GL_MAX_VERTEX_UNIFORM_COMPONENTS = 0x8B4A;
	static const uint32_t GL_MAX_VARYING_FLOATS = 0x8B4B;
	static const uint32_t GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS = 0x8B4C;
	static const uint32_t GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS = 0x8B4D;
	static const uint32_t GL_SHADER_TYPE = 0x8B4F;
	static const uint32_t GL_FLOAT_VEC2 = 0x8B50;
	static const uint32_t GL_FLOAT_VEC3 = 0x8B51;
	static const uint32_t GL_FLOAT_VEC4 = 0x8B52;
	static const uint32_t GL_INT_VEC2 = 0x8B53;
	static const uint32_t GL_INT_VEC3 = 0x8B54;
	static const uint32_t GL_INT_VEC4 = 0x8B55;
	static const uint32_t GL_BOOL = 0x8B56;
	static const uint32_t GL_BOOL_VEC2 = 0x8B57;
	static const uint32_t GL_BOOL_VEC3 = 0x8B58;
	static const uint32_t GL_BOOL_VEC4 = 0x8B59;
	static const uint32_t GL_FLOAT_MAT2 = 0x8B5A;
	static const uint32_t GL_FLOAT_MAT3 = 0x8B5B;
	static const uint32_t GL_FLOAT_MAT4 = 0x8B5C;
	static const uint32_t GL_SAMPLER_1D = 0x8B5D;
	static const uint32_t GL_SAMPLER_2D = 0x8B5E;
	static const uint32_t GL_SAMPLER_3D = 0x8B5F;
	static const uint32_t GL_SAMPLER_CUBE = 0x8B60;
	static const uint32_t GL_SAMPLER_1D_SHADOW = 0x8B61;
	static const uint32_t GL_SAMPLER_2D_SHADOW = 0x8B62;
	static const uint32_t GL_DELETE_STATUS = 0x8B80;
	static const uint32_t GL_COMPILE_STATUS = 0x8B81;
	static const uint32_t GL_LINK_STATUS = 0x8B82;
	static const uint32_t GL_VALIDATE_STATUS = 0x8B83;
	static const uint32_t GL_INFO_LOG_LENGTH = 0x8B84;
	static const uint32_t GL_ATTACHED_SHADERS = 0x8B85;
	static const uint32_t GL_ACTIVE_UNIFORMS = 0x8B86;
	static const uint32_t GL_ACTIVE_UNIFORM_MAX_LENGTH = 0x8B87;
	static const uint32_t GL_SHADER_SOURCE_LENGTH = 0x8B88;
	static const uint32_t GL_ACTIVE_ATTRIBUTES = 0x8B89;
	static const uint32_t GL_ACTIVE_ATTRIBUTE_MAX_LENGTH = 0x8B8A;
	static const uint32_t GL_FRAGMENT_SHADER_DERIVATIVE_HINT = 0x8B8B;
	static const uint32_t GL_SHADING_LANGUAGE_VERSION = 0x8B8C;
	static const uint32_t GL_CURRENT_PROGRAM = 0x8B8D;
	static const uint32_t GL_POINT_SPRITE_COORD_ORIGIN = 0x8CA0;
	static const uint32_t GL_LOWER_LEFT = 0x8CA1;
	static const uint32_t GL_UPPER_LEFT = 0x8CA2;
	static const uint32_t GL_STENCIL_BACK_REF = 0x8CA3;
	static const uint32_t GL_STENCIL_BACK_VALUE_MASK = 0x8CA4;
	static const uint32_t GL_STENCIL_BACK_WRITEMASK = 0x8CA5;
	static const uint32_t GL_VERTEX_PROGRAM_TWO_SIDE = 0x8643;
	static const uint32_t GL_POINT_SPRITE = 0x8861;
	static const uint32_t GL_COORD_REPLACE = 0x8862;
	static const uint32_t GL_MAX_TEXTURE_COORDS = 0x8871;
	typedef void (FDYNGL_APIENTRY gl_blend_equation_separate_func)(GLenum modeRGB, GLenum modeAlpha);
	static gl_blend_equation_separate_func* glBlendEquationSeparate;
	typedef void (FDYNGL_APIENTRY gl_draw_buffers_func)(GLsizei n, const GLenum *bufs);
	static gl_draw_buffers_func* glDrawBuffers;
	typedef void (FDYNGL_APIENTRY gl_stencil_op_separate_func)(GLenum face, GLenum sfail, GLenum dpfail, GLenum dppass);
	static gl_stencil_op_separate_func* glStencilOpSeparate;
	typedef void (FDYNGL_APIENTRY gl_stencil_func_separate_func)(GLenum face, GLenum func, GLint ref, GLuint mask);
	static gl_stencil_func_separate_func* glStencilFuncSeparate;
	typedef void (FDYNGL_APIENTRY gl_stencil_mask_separate_func)(GLenum face, GLuint mask);
	static gl_stencil_mask_separate_func* glStencilMaskSeparate;
	typedef void (FDYNGL_APIENTRY gl_attach_shader_func)(GLuint program, GLuint shader);
	static gl_attach_shader_func* glAttachShader;
	typedef void (FDYNGL_APIENTRY gl_bind_attrib_location_func)(GLuint program, GLuint index, const GLchar *name);
	static gl_bind_attrib_location_func* glBindAttribLocation;
	typedef void (FDYNGL_APIENTRY gl_compile_shader_func)(GLuint shader);
	static gl_compile_shader_func* glCompileShader;
	typedef GLuint(FDYNGL_APIENTRY gl_create_program_func)(void);
	static gl_create_program_func* glCreateProgram;
	typedef GLuint(FDYNGL_APIENTRY gl_create_shader_func)(GLenum type);
	static gl_create_shader_func* glCreateShader;
	typedef void (FDYNGL_APIENTRY gl_delete_program_func)(GLuint program);
	static gl_delete_program_func* glDeleteProgram;
	typedef void (FDYNGL_APIENTRY gl_delete_shader_func)(GLuint shader);
	static gl_delete_shader_func* glDeleteShader;
	typedef void (FDYNGL_APIENTRY gl_detach_shader_func)(GLuint program, GLuint shader);
	static gl_detach_shader_func* glDetachShader;
	typedef void (FDYNGL_APIENTRY gl_disable_vertex_attrib_array_func)(GLuint index);
	static gl_disable_vertex_attrib_array_func* glDisableVertexAttribArray;
	typedef void (FDYNGL_APIENTRY gl_enable_vertex_attrib_array_func)(GLuint index);
	static gl_enable_vertex_attrib_array_func* glEnableVertexAttribArray;
	typedef void (FDYNGL_APIENTRY gl_get_active_attrib_func)(GLuint program, GLuint index, GLsizei bufSize, GLsizei *length, GLint *size, GLenum *type, GLchar *name);
	static gl_get_active_attrib_func* glGetActiveAttrib;
	typedef void (FDYNGL_APIENTRY gl_get_active_uniform_func)(GLuint program, GLuint index, GLsizei bufSize, GLsizei *length, GLint *size, GLenum *type, GLchar *name);
	static gl_get_active_uniform_func* glGetActiveUniform;
	typedef void (FDYNGL_APIENTRY gl_get_attached_shaders_func)(GLuint program, GLsizei maxCount, GLsizei *count, GLuint *shaders);
	static gl_get_attached_shaders_func* glGetAttachedShaders;
	typedef GLint(FDYNGL_APIENTRY gl_get_attrib_location_func)(GLuint program, const GLchar *name);
	static gl_get_attrib_location_func* glGetAttribLocation;
	typedef void (FDYNGL_APIENTRY gl_get_programiv_func)(GLuint program, GLenum pname, GLint *params);
	static gl_get_programiv_func* glGetProgramiv;
	typedef void (FDYNGL_APIENTRY gl_get_program_info_log_func)(GLuint program, GLsizei bufSize, GLsizei *length, GLchar *infoLog);
	static gl_get_program_info_log_func* glGetProgramInfoLog;
	typedef void (FDYNGL_APIENTRY gl_get_shaderiv_func)(GLuint shader, GLenum pname, GLint *params);
	static gl_get_shaderiv_func* glGetShaderiv;
	typedef void (FDYNGL_APIENTRY gl_get_shader_info_log_func)(GLuint shader, GLsizei bufSize, GLsizei *length, GLchar *infoLog);
	static gl_get_shader_info_log_func* glGetShaderInfoLog;
	typedef void (FDYNGL_APIENTRY gl_get_shader_source_func)(GLuint shader, GLsizei bufSize, GLsizei *length, GLchar *source);
	static gl_get_shader_source_func* glGetShaderSource;
	typedef GLint(FDYNGL_APIENTRY gl_get_uniform_location_func)(GLuint program, const GLchar *name);
	static gl_get_uniform_location_func* glGetUniformLocation;
	typedef void (FDYNGL_APIENTRY gl_get_uniformfv_func)(GLuint program, GLint location, GLfloat *params);
	static gl_get_uniformfv_func* glGetUniformfv;
	typedef void (FDYNGL_APIENTRY gl_get_uniformiv_func)(GLuint program, GLint location, GLint *params);
	static gl_get_uniformiv_func* glGetUniformiv;
	typedef void (FDYNGL_APIENTRY gl_get_vertex_attribdv_func)(GLuint index, GLenum pname, GLdouble *params);
	static gl_get_vertex_attribdv_func* glGetVertexAttribdv;
	typedef void (FDYNGL_APIENTRY gl_get_vertex_attribfv_func)(GLuint index, GLenum pname, GLfloat *params);
	static gl_get_vertex_attribfv_func* glGetVertexAttribfv;
	typedef void (FDYNGL_APIENTRY gl_get_vertex_attribiv_func)(GLuint index, GLenum pname, GLint *params);
	static gl_get_vertex_attribiv_func* glGetVertexAttribiv;
	typedef void (FDYNGL_APIENTRY gl_get_vertex_attrib_pointerv_func)(GLuint index, GLenum pname, void **pointer);
	static gl_get_vertex_attrib_pointerv_func* glGetVertexAttribPointerv;
	typedef GLboolean(FDYNGL_APIENTRY gl_is_program_func)(GLuint program);
	static gl_is_program_func* glIsProgram;
	typedef GLboolean(FDYNGL_APIENTRY gl_is_shader_func)(GLuint shader);
	static gl_is_shader_func* glIsShader;
	typedef void (FDYNGL_APIENTRY gl_link_program_func)(GLuint program);
	static gl_link_program_func* glLinkProgram;
	typedef void (FDYNGL_APIENTRY gl_shader_source_func)(GLuint shader, GLsizei count, const GLchar *const*string, const GLint *length);
	static gl_shader_source_func* glShaderSource;
	typedef void (FDYNGL_APIENTRY gl_use_program_func)(GLuint program);
	static gl_use_program_func* glUseProgram;
	typedef void (FDYNGL_APIENTRY gl_uniform1f_func)(GLint location, GLfloat v0);
	static gl_uniform1f_func* glUniform1f;
	typedef void (FDYNGL_APIENTRY gl_uniform2f_func)(GLint location, GLfloat v0, GLfloat v1);
	static gl_uniform2f_func* glUniform2f;
	typedef void (FDYNGL_APIENTRY gl_uniform3f_func)(GLint location, GLfloat v0, GLfloat v1, GLfloat v2);
	static gl_uniform3f_func* glUniform3f;
	typedef void (FDYNGL_APIENTRY gl_uniform4f_func)(GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3);
	static gl_uniform4f_func* glUniform4f;
	typedef void (FDYNGL_APIENTRY gl_uniform1i_func)(GLint location, GLint v0);
	static gl_uniform1i_func* glUniform1i;
	typedef void (FDYNGL_APIENTRY gl_uniform2i_func)(GLint location, GLint v0, GLint v1);
	static gl_uniform2i_func* glUniform2i;
	typedef void (FDYNGL_APIENTRY gl_uniform3i_func)(GLint location, GLint v0, GLint v1, GLint v2);
	static gl_uniform3i_func* glUniform3i;
	typedef void (FDYNGL_APIENTRY gl_uniform4i_func)(GLint location, GLint v0, GLint v1, GLint v2, GLint v3);
	static gl_uniform4i_func* glUniform4i;
	typedef void (FDYNGL_APIENTRY gl_uniform1fv_func)(GLint location, GLsizei count, const GLfloat *value);
	static gl_uniform1fv_func* glUniform1fv;
	typedef void (FDYNGL_APIENTRY gl_uniform2fv_func)(GLint location, GLsizei count, const GLfloat *value);
	static gl_uniform2fv_func* glUniform2fv;
	typedef void (FDYNGL_APIENTRY gl_uniform3fv_func)(GLint location, GLsizei count, const GLfloat *value);
	static gl_uniform3fv_func* glUniform3fv;
	typedef void (FDYNGL_APIENTRY gl_uniform4fv_func)(GLint location, GLsizei count, const GLfloat *value);
	static gl_uniform4fv_func* glUniform4fv;
	typedef void (FDYNGL_APIENTRY gl_uniform1iv_func)(GLint location, GLsizei count, const GLint *value);
	static gl_uniform1iv_func* glUniform1iv;
	typedef void (FDYNGL_APIENTRY gl_uniform2iv_func)(GLint location, GLsizei count, const GLint *value);
	static gl_uniform2iv_func* glUniform2iv;
	typedef void (FDYNGL_APIENTRY gl_uniform3iv_func)(GLint location, GLsizei count, const GLint *value);
	static gl_uniform3iv_func* glUniform3iv;
	typedef void (FDYNGL_APIENTRY gl_uniform4iv_func)(GLint location, GLsizei count, const GLint *value);
	static gl_uniform4iv_func* glUniform4iv;
	typedef void (FDYNGL_APIENTRY gl_uniform_matrix2fv_func)(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
	static gl_uniform_matrix2fv_func* glUniformMatrix2fv;
	typedef void (FDYNGL_APIENTRY gl_uniform_matrix3fv_func)(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
	static gl_uniform_matrix3fv_func* glUniformMatrix3fv;
	typedef void (FDYNGL_APIENTRY gl_uniform_matrix4fv_func)(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
	static gl_uniform_matrix4fv_func* glUniformMatrix4fv;
	typedef void (FDYNGL_APIENTRY gl_validate_program_func)(GLuint program);
	static gl_validate_program_func* glValidateProgram;
	typedef void (FDYNGL_APIENTRY gl_vertex_attrib1d_func)(GLuint index, GLdouble x);
	static gl_vertex_attrib1d_func* glVertexAttrib1d;
	typedef void (FDYNGL_APIENTRY gl_vertex_attrib1dv_func)(GLuint index, const GLdouble *v);
	static gl_vertex_attrib1dv_func* glVertexAttrib1dv;
	typedef void (FDYNGL_APIENTRY gl_vertex_attrib1f_func)(GLuint index, GLfloat x);
	static gl_vertex_attrib1f_func* glVertexAttrib1f;
	typedef void (FDYNGL_APIENTRY gl_vertex_attrib1fv_func)(GLuint index, const GLfloat *v);
	static gl_vertex_attrib1fv_func* glVertexAttrib1fv;
	typedef void (FDYNGL_APIENTRY gl_vertex_attrib1s_func)(GLuint index, GLshort x);
	static gl_vertex_attrib1s_func* glVertexAttrib1s;
	typedef void (FDYNGL_APIENTRY gl_vertex_attrib1sv_func)(GLuint index, const GLshort *v);
	static gl_vertex_attrib1sv_func* glVertexAttrib1sv;
	typedef void (FDYNGL_APIENTRY gl_vertex_attrib2d_func)(GLuint index, GLdouble x, GLdouble y);
	static gl_vertex_attrib2d_func* glVertexAttrib2d;
	typedef void (FDYNGL_APIENTRY gl_vertex_attrib2dv_func)(GLuint index, const GLdouble *v);
	static gl_vertex_attrib2dv_func* glVertexAttrib2dv;
	typedef void (FDYNGL_APIENTRY gl_vertex_attrib2f_func)(GLuint index, GLfloat x, GLfloat y);
	static gl_vertex_attrib2f_func* glVertexAttrib2f;
	typedef void (FDYNGL_APIENTRY gl_vertex_attrib2fv_func)(GLuint index, const GLfloat *v);
	static gl_vertex_attrib2fv_func* glVertexAttrib2fv;
	typedef void (FDYNGL_APIENTRY gl_vertex_attrib2s_func)(GLuint index, GLshort x, GLshort y);
	static gl_vertex_attrib2s_func* glVertexAttrib2s;
	typedef void (FDYNGL_APIENTRY gl_vertex_attrib2sv_func)(GLuint index, const GLshort *v);
	static gl_vertex_attrib2sv_func* glVertexAttrib2sv;
	typedef void (FDYNGL_APIENTRY gl_vertex_attrib3d_func)(GLuint index, GLdouble x, GLdouble y, GLdouble z);
	static gl_vertex_attrib3d_func* glVertexAttrib3d;
	typedef void (FDYNGL_APIENTRY gl_vertex_attrib3dv_func)(GLuint index, const GLdouble *v);
	static gl_vertex_attrib3dv_func* glVertexAttrib3dv;
	typedef void (FDYNGL_APIENTRY gl_vertex_attrib3f_func)(GLuint index, GLfloat x, GLfloat y, GLfloat z);
	static gl_vertex_attrib3f_func* glVertexAttrib3f;
	typedef void (FDYNGL_APIENTRY gl_vertex_attrib3fv_func)(GLuint index, const GLfloat *v);
	static gl_vertex_attrib3fv_func* glVertexAttrib3fv;
	typedef void (FDYNGL_APIENTRY gl_vertex_attrib3s_func)(GLuint index, GLshort x, GLshort y, GLshort z);
	static gl_vertex_attrib3s_func* glVertexAttrib3s;
	typedef void (FDYNGL_APIENTRY gl_vertex_attrib3sv_func)(GLuint index, const GLshort *v);
	static gl_vertex_attrib3sv_func* glVertexAttrib3sv;
	typedef void (FDYNGL_APIENTRY gl_vertex_attrib4_nbv_func)(GLuint index, const GLbyte *v);
	static gl_vertex_attrib4_nbv_func* glVertexAttrib4Nbv;
	typedef void (FDYNGL_APIENTRY gl_vertex_attrib4_niv_func)(GLuint index, const GLint *v);
	static gl_vertex_attrib4_niv_func* glVertexAttrib4Niv;
	typedef void (FDYNGL_APIENTRY gl_vertex_attrib4_nsv_func)(GLuint index, const GLshort *v);
	static gl_vertex_attrib4_nsv_func* glVertexAttrib4Nsv;
	typedef void (FDYNGL_APIENTRY gl_vertex_attrib4_nub_func)(GLuint index, GLubyte x, GLubyte y, GLubyte z, GLubyte w);
	static gl_vertex_attrib4_nub_func* glVertexAttrib4Nub;
	typedef void (FDYNGL_APIENTRY gl_vertex_attrib4_nubv_func)(GLuint index, const GLubyte *v);
	static gl_vertex_attrib4_nubv_func* glVertexAttrib4Nubv;
	typedef void (FDYNGL_APIENTRY gl_vertex_attrib4_nuiv_func)(GLuint index, const GLuint *v);
	static gl_vertex_attrib4_nuiv_func* glVertexAttrib4Nuiv;
	typedef void (FDYNGL_APIENTRY gl_vertex_attrib4_nusv_func)(GLuint index, const GLushort *v);
	static gl_vertex_attrib4_nusv_func* glVertexAttrib4Nusv;
	typedef void (FDYNGL_APIENTRY gl_vertex_attrib4bv_func)(GLuint index, const GLbyte *v);
	static gl_vertex_attrib4bv_func* glVertexAttrib4bv;
	typedef void (FDYNGL_APIENTRY gl_vertex_attrib4d_func)(GLuint index, GLdouble x, GLdouble y, GLdouble z, GLdouble w);
	static gl_vertex_attrib4d_func* glVertexAttrib4d;
	typedef void (FDYNGL_APIENTRY gl_vertex_attrib4dv_func)(GLuint index, const GLdouble *v);
	static gl_vertex_attrib4dv_func* glVertexAttrib4dv;
	typedef void (FDYNGL_APIENTRY gl_vertex_attrib4f_func)(GLuint index, GLfloat x, GLfloat y, GLfloat z, GLfloat w);
	static gl_vertex_attrib4f_func* glVertexAttrib4f;
	typedef void (FDYNGL_APIENTRY gl_vertex_attrib4fv_func)(GLuint index, const GLfloat *v);
	static gl_vertex_attrib4fv_func* glVertexAttrib4fv;
	typedef void (FDYNGL_APIENTRY gl_vertex_attrib4iv_func)(GLuint index, const GLint *v);
	static gl_vertex_attrib4iv_func* glVertexAttrib4iv;
	typedef void (FDYNGL_APIENTRY gl_vertex_attrib4s_func)(GLuint index, GLshort x, GLshort y, GLshort z, GLshort w);
	static gl_vertex_attrib4s_func* glVertexAttrib4s;
	typedef void (FDYNGL_APIENTRY gl_vertex_attrib4sv_func)(GLuint index, const GLshort *v);
	static gl_vertex_attrib4sv_func* glVertexAttrib4sv;
	typedef void (FDYNGL_APIENTRY gl_vertex_attrib4ubv_func)(GLuint index, const GLubyte *v);
	static gl_vertex_attrib4ubv_func* glVertexAttrib4ubv;
	typedef void (FDYNGL_APIENTRY gl_vertex_attrib4uiv_func)(GLuint index, const GLuint *v);
	static gl_vertex_attrib4uiv_func* glVertexAttrib4uiv;
	typedef void (FDYNGL_APIENTRY gl_vertex_attrib4usv_func)(GLuint index, const GLushort *v);
	static gl_vertex_attrib4usv_func* glVertexAttrib4usv;
	typedef void (FDYNGL_APIENTRY gl_vertex_attrib_pointer_func)(GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const void *pointer);
	static gl_vertex_attrib_pointer_func* glVertexAttribPointer;
#	endif // GL_VERSION_2_0
#	ifndef GL_VERSION_2_1
#		define GL_VERSION_2_1 1
	static bool isGL_VERSION_2_1;
	static const uint32_t GL_PIXEL_PACK_BUFFER = 0x88EB;
	static const uint32_t GL_PIXEL_UNPACK_BUFFER = 0x88EC;
	static const uint32_t GL_PIXEL_PACK_BUFFER_BINDING = 0x88ED;
	static const uint32_t GL_PIXEL_UNPACK_BUFFER_BINDING = 0x88EF;
	static const uint32_t GL_FLOAT_MAT2x3 = 0x8B65;
	static const uint32_t GL_FLOAT_MAT2x4 = 0x8B66;
	static const uint32_t GL_FLOAT_MAT3x2 = 0x8B67;
	static const uint32_t GL_FLOAT_MAT3x4 = 0x8B68;
	static const uint32_t GL_FLOAT_MAT4x2 = 0x8B69;
	static const uint32_t GL_FLOAT_MAT4x3 = 0x8B6A;
	static const uint32_t GL_SRGB = 0x8C40;
	static const uint32_t GL_SRGB8 = 0x8C41;
	static const uint32_t GL_SRGB_ALPHA = 0x8C42;
	static const uint32_t GL_SRGB8_ALPHA8 = 0x8C43;
	static const uint32_t GL_COMPRESSED_SRGB = 0x8C48;
	static const uint32_t GL_COMPRESSED_SRGB_ALPHA = 0x8C49;
	static const uint32_t GL_CURRENT_RASTER_SECONDARY_COLOR = 0x845F;
	static const uint32_t GL_SLUMINANCE_ALPHA = 0x8C44;
	static const uint32_t GL_SLUMINANCE8_ALPHA8 = 0x8C45;
	static const uint32_t GL_SLUMINANCE = 0x8C46;
	static const uint32_t GL_SLUMINANCE8 = 0x8C47;
	static const uint32_t GL_COMPRESSED_SLUMINANCE = 0x8C4A;
	static const uint32_t GL_COMPRESSED_SLUMINANCE_ALPHA = 0x8C4B;
	typedef void (FDYNGL_APIENTRY gl_uniform_matrix2x3fv_func)(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
	static gl_uniform_matrix2x3fv_func* glUniformMatrix2x3fv;
	typedef void (FDYNGL_APIENTRY gl_uniform_matrix3x2fv_func)(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
	static gl_uniform_matrix3x2fv_func* glUniformMatrix3x2fv;
	typedef void (FDYNGL_APIENTRY gl_uniform_matrix2x4fv_func)(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
	static gl_uniform_matrix2x4fv_func* glUniformMatrix2x4fv;
	typedef void (FDYNGL_APIENTRY gl_uniform_matrix4x2fv_func)(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
	static gl_uniform_matrix4x2fv_func* glUniformMatrix4x2fv;
	typedef void (FDYNGL_APIENTRY gl_uniform_matrix3x4fv_func)(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
	static gl_uniform_matrix3x4fv_func* glUniformMatrix3x4fv;
	typedef void (FDYNGL_APIENTRY gl_uniform_matrix4x3fv_func)(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
	static gl_uniform_matrix4x3fv_func* glUniformMatrix4x3fv;
#	endif // GL_VERSION_2_1
#	ifndef GL_VERSION_3_0
#		define GL_VERSION_3_0 1
	static bool isGL_VERSION_3_0;
	static const uint32_t GL_COMPARE_REF_TO_TEXTURE = 0x884E;
	static const uint32_t GL_CLIP_DISTANCE0 = 0x3000;
	static const uint32_t GL_CLIP_DISTANCE1 = 0x3001;
	static const uint32_t GL_CLIP_DISTANCE2 = 0x3002;
	static const uint32_t GL_CLIP_DISTANCE3 = 0x3003;
	static const uint32_t GL_CLIP_DISTANCE4 = 0x3004;
	static const uint32_t GL_CLIP_DISTANCE5 = 0x3005;
	static const uint32_t GL_CLIP_DISTANCE6 = 0x3006;
	static const uint32_t GL_CLIP_DISTANCE7 = 0x3007;
	static const uint32_t GL_MAX_CLIP_DISTANCES = 0x0D32;
	static const uint32_t GL_MAJOR_VERSION = 0x821B;
	static const uint32_t GL_MINOR_VERSION = 0x821C;
	static const uint32_t GL_NUM_EXTENSIONS = 0x821D;
	static const uint32_t GL_CONTEXT_FLAGS = 0x821E;
	static const uint32_t GL_COMPRESSED_RED = 0x8225;
	static const uint32_t GL_COMPRESSED_RG = 0x8226;
	static const uint32_t GL_CONTEXT_FLAG_FORWARD_COMPATIBLE_BIT = 0x00000001;
	static const uint32_t GL_RGBA32F = 0x8814;
	static const uint32_t GL_RGB32F = 0x8815;
	static const uint32_t GL_RGBA16F = 0x881A;
	static const uint32_t GL_RGB16F = 0x881B;
	static const uint32_t GL_VERTEX_ATTRIB_ARRAY_INTEGER = 0x88FD;
	static const uint32_t GL_MAX_ARRAY_TEXTURE_LAYERS = 0x88FF;
	static const uint32_t GL_MIN_PROGRAM_TEXEL_OFFSET = 0x8904;
	static const uint32_t GL_MAX_PROGRAM_TEXEL_OFFSET = 0x8905;
	static const uint32_t GL_CLAMP_READ_COLOR = 0x891C;
	static const uint32_t GL_FIXED_ONLY = 0x891D;
	static const uint32_t GL_MAX_VARYING_COMPONENTS = 0x8B4B;
	static const uint32_t GL_TEXTURE_1D_ARRAY = 0x8C18;
	static const uint32_t GL_PROXY_TEXTURE_1D_ARRAY = 0x8C19;
	static const uint32_t GL_TEXTURE_2D_ARRAY = 0x8C1A;
	static const uint32_t GL_PROXY_TEXTURE_2D_ARRAY = 0x8C1B;
	static const uint32_t GL_TEXTURE_BINDING_1D_ARRAY = 0x8C1C;
	static const uint32_t GL_TEXTURE_BINDING_2D_ARRAY = 0x8C1D;
	static const uint32_t GL_R11F_G11F_B10F = 0x8C3A;
	static const uint32_t GL_UNSIGNED_INT_10F_11F_11F_REV = 0x8C3B;
	static const uint32_t GL_RGB9_E5 = 0x8C3D;
	static const uint32_t GL_UNSIGNED_INT_5_9_9_9_REV = 0x8C3E;
	static const uint32_t GL_TEXTURE_SHARED_SIZE = 0x8C3F;
	static const uint32_t GL_TRANSFORM_FEEDBACK_VARYING_MAX_LENGTH = 0x8C76;
	static const uint32_t GL_TRANSFORM_FEEDBACK_BUFFER_MODE = 0x8C7F;
	static const uint32_t GL_MAX_TRANSFORM_FEEDBACK_SEPARATE_COMPONENTS = 0x8C80;
	static const uint32_t GL_TRANSFORM_FEEDBACK_VARYINGS = 0x8C83;
	static const uint32_t GL_TRANSFORM_FEEDBACK_BUFFER_START = 0x8C84;
	static const uint32_t GL_TRANSFORM_FEEDBACK_BUFFER_SIZE = 0x8C85;
	static const uint32_t GL_PRIMITIVES_GENERATED = 0x8C87;
	static const uint32_t GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN = 0x8C88;
	static const uint32_t GL_RASTERIZER_DISCARD = 0x8C89;
	static const uint32_t GL_MAX_TRANSFORM_FEEDBACK_INTERLEAVED_COMPONENTS = 0x8C8A;
	static const uint32_t GL_MAX_TRANSFORM_FEEDBACK_SEPARATE_ATTRIBS = 0x8C8B;
	static const uint32_t GL_INTERLEAVED_ATTRIBS = 0x8C8C;
	static const uint32_t GL_SEPARATE_ATTRIBS = 0x8C8D;
	static const uint32_t GL_TRANSFORM_FEEDBACK_BUFFER = 0x8C8E;
	static const uint32_t GL_TRANSFORM_FEEDBACK_BUFFER_BINDING = 0x8C8F;
	static const uint32_t GL_RGBA32UI = 0x8D70;
	static const uint32_t GL_RGB32UI = 0x8D71;
	static const uint32_t GL_RGBA16UI = 0x8D76;
	static const uint32_t GL_RGB16UI = 0x8D77;
	static const uint32_t GL_RGBA8UI = 0x8D7C;
	static const uint32_t GL_RGB8UI = 0x8D7D;
	static const uint32_t GL_RGBA32I = 0x8D82;
	static const uint32_t GL_RGB32I = 0x8D83;
	static const uint32_t GL_RGBA16I = 0x8D88;
	static const uint32_t GL_RGB16I = 0x8D89;
	static const uint32_t GL_RGBA8I = 0x8D8E;
	static const uint32_t GL_RGB8I = 0x8D8F;
	static const uint32_t GL_RED_INTEGER = 0x8D94;
	static const uint32_t GL_GREEN_INTEGER = 0x8D95;
	static const uint32_t GL_BLUE_INTEGER = 0x8D96;
	static const uint32_t GL_RGB_INTEGER = 0x8D98;
	static const uint32_t GL_RGBA_INTEGER = 0x8D99;
	static const uint32_t GL_BGR_INTEGER = 0x8D9A;
	static const uint32_t GL_BGRA_INTEGER = 0x8D9B;
	static const uint32_t GL_SAMPLER_1D_ARRAY = 0x8DC0;
	static const uint32_t GL_SAMPLER_2D_ARRAY = 0x8DC1;
	static const uint32_t GL_SAMPLER_1D_ARRAY_SHADOW = 0x8DC3;
	static const uint32_t GL_SAMPLER_2D_ARRAY_SHADOW = 0x8DC4;
	static const uint32_t GL_SAMPLER_CUBE_SHADOW = 0x8DC5;
	static const uint32_t GL_UNSIGNED_INT_VEC2 = 0x8DC6;
	static const uint32_t GL_UNSIGNED_INT_VEC3 = 0x8DC7;
	static const uint32_t GL_UNSIGNED_INT_VEC4 = 0x8DC8;
	static const uint32_t GL_INT_SAMPLER_1D = 0x8DC9;
	static const uint32_t GL_INT_SAMPLER_2D = 0x8DCA;
	static const uint32_t GL_INT_SAMPLER_3D = 0x8DCB;
	static const uint32_t GL_INT_SAMPLER_CUBE = 0x8DCC;
	static const uint32_t GL_INT_SAMPLER_1D_ARRAY = 0x8DCE;
	static const uint32_t GL_INT_SAMPLER_2D_ARRAY = 0x8DCF;
	static const uint32_t GL_UNSIGNED_INT_SAMPLER_1D = 0x8DD1;
	static const uint32_t GL_UNSIGNED_INT_SAMPLER_2D = 0x8DD2;
	static const uint32_t GL_UNSIGNED_INT_SAMPLER_3D = 0x8DD3;
	static const uint32_t GL_UNSIGNED_INT_SAMPLER_CUBE = 0x8DD4;
	static const uint32_t GL_UNSIGNED_INT_SAMPLER_1D_ARRAY = 0x8DD6;
	static const uint32_t GL_UNSIGNED_INT_SAMPLER_2D_ARRAY = 0x8DD7;
	static const uint32_t GL_QUERY_WAIT = 0x8E13;
	static const uint32_t GL_QUERY_NO_WAIT = 0x8E14;
	static const uint32_t GL_QUERY_BY_REGION_WAIT = 0x8E15;
	static const uint32_t GL_QUERY_BY_REGION_NO_WAIT = 0x8E16;
	static const uint32_t GL_BUFFER_ACCESS_FLAGS = 0x911F;
	static const uint32_t GL_BUFFER_MAP_LENGTH = 0x9120;
	static const uint32_t GL_BUFFER_MAP_OFFSET = 0x9121;
	static const uint32_t GL_DEPTH_COMPONENT32F = 0x8CAC;
	static const uint32_t GL_DEPTH32F_STENCIL8 = 0x8CAD;
	static const uint32_t GL_FLOAT_32_UNSIGNED_INT_24_8_REV = 0x8DAD;
	static const uint32_t GL_INVALID_FRAMEBUFFER_OPERATION = 0x0506;
	static const uint32_t GL_FRAMEBUFFER_ATTACHMENT_COLOR_ENCODING = 0x8210;
	static const uint32_t GL_FRAMEBUFFER_ATTACHMENT_COMPONENT_TYPE = 0x8211;
	static const uint32_t GL_FRAMEBUFFER_ATTACHMENT_RED_SIZE = 0x8212;
	static const uint32_t GL_FRAMEBUFFER_ATTACHMENT_GREEN_SIZE = 0x8213;
	static const uint32_t GL_FRAMEBUFFER_ATTACHMENT_BLUE_SIZE = 0x8214;
	static const uint32_t GL_FRAMEBUFFER_ATTACHMENT_ALPHA_SIZE = 0x8215;
	static const uint32_t GL_FRAMEBUFFER_ATTACHMENT_DEPTH_SIZE = 0x8216;
	static const uint32_t GL_FRAMEBUFFER_ATTACHMENT_STENCIL_SIZE = 0x8217;
	static const uint32_t GL_FRAMEBUFFER_DEFAULT = 0x8218;
	static const uint32_t GL_FRAMEBUFFER_UNDEFINED = 0x8219;
	static const uint32_t GL_DEPTH_STENCIL_ATTACHMENT = 0x821A;
	static const uint32_t GL_MAX_RENDERBUFFER_SIZE = 0x84E8;
	static const uint32_t GL_DEPTH_STENCIL = 0x84F9;
	static const uint32_t GL_UNSIGNED_INT_24_8 = 0x84FA;
	static const uint32_t GL_DEPTH24_STENCIL8 = 0x88F0;
	static const uint32_t GL_TEXTURE_STENCIL_SIZE = 0x88F1;
	static const uint32_t GL_TEXTURE_RED_TYPE = 0x8C10;
	static const uint32_t GL_TEXTURE_GREEN_TYPE = 0x8C11;
	static const uint32_t GL_TEXTURE_BLUE_TYPE = 0x8C12;
	static const uint32_t GL_TEXTURE_ALPHA_TYPE = 0x8C13;
	static const uint32_t GL_TEXTURE_DEPTH_TYPE = 0x8C16;
	static const uint32_t GL_UNSIGNED_NORMALIZED = 0x8C17;
	static const uint32_t GL_FRAMEBUFFER_BINDING = 0x8CA6;
	static const uint32_t GL_DRAW_FRAMEBUFFER_BINDING = 0x8CA6;
	static const uint32_t GL_RENDERBUFFER_BINDING = 0x8CA7;
	static const uint32_t GL_READ_FRAMEBUFFER = 0x8CA8;
	static const uint32_t GL_DRAW_FRAMEBUFFER = 0x8CA9;
	static const uint32_t GL_READ_FRAMEBUFFER_BINDING = 0x8CAA;
	static const uint32_t GL_RENDERBUFFER_SAMPLES = 0x8CAB;
	static const uint32_t GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE = 0x8CD0;
	static const uint32_t GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME = 0x8CD1;
	static const uint32_t GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_LEVEL = 0x8CD2;
	static const uint32_t GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_CUBE_MAP_FACE = 0x8CD3;
	static const uint32_t GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_LAYER = 0x8CD4;
	static const uint32_t GL_FRAMEBUFFER_COMPLETE = 0x8CD5;
	static const uint32_t GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT = 0x8CD6;
	static const uint32_t GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT = 0x8CD7;
	static const uint32_t GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER = 0x8CDB;
	static const uint32_t GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER = 0x8CDC;
	static const uint32_t GL_FRAMEBUFFER_UNSUPPORTED = 0x8CDD;
	static const uint32_t GL_MAX_COLOR_ATTACHMENTS = 0x8CDF;
	static const uint32_t GL_COLOR_ATTACHMENT0 = 0x8CE0;
	static const uint32_t GL_COLOR_ATTACHMENT1 = 0x8CE1;
	static const uint32_t GL_COLOR_ATTACHMENT2 = 0x8CE2;
	static const uint32_t GL_COLOR_ATTACHMENT3 = 0x8CE3;
	static const uint32_t GL_COLOR_ATTACHMENT4 = 0x8CE4;
	static const uint32_t GL_COLOR_ATTACHMENT5 = 0x8CE5;
	static const uint32_t GL_COLOR_ATTACHMENT6 = 0x8CE6;
	static const uint32_t GL_COLOR_ATTACHMENT7 = 0x8CE7;
	static const uint32_t GL_COLOR_ATTACHMENT8 = 0x8CE8;
	static const uint32_t GL_COLOR_ATTACHMENT9 = 0x8CE9;
	static const uint32_t GL_COLOR_ATTACHMENT10 = 0x8CEA;
	static const uint32_t GL_COLOR_ATTACHMENT11 = 0x8CEB;
	static const uint32_t GL_COLOR_ATTACHMENT12 = 0x8CEC;
	static const uint32_t GL_COLOR_ATTACHMENT13 = 0x8CED;
	static const uint32_t GL_COLOR_ATTACHMENT14 = 0x8CEE;
	static const uint32_t GL_COLOR_ATTACHMENT15 = 0x8CEF;
	static const uint32_t GL_COLOR_ATTACHMENT16 = 0x8CF0;
	static const uint32_t GL_COLOR_ATTACHMENT17 = 0x8CF1;
	static const uint32_t GL_COLOR_ATTACHMENT18 = 0x8CF2;
	static const uint32_t GL_COLOR_ATTACHMENT19 = 0x8CF3;
	static const uint32_t GL_COLOR_ATTACHMENT20 = 0x8CF4;
	static const uint32_t GL_COLOR_ATTACHMENT21 = 0x8CF5;
	static const uint32_t GL_COLOR_ATTACHMENT22 = 0x8CF6;
	static const uint32_t GL_COLOR_ATTACHMENT23 = 0x8CF7;
	static const uint32_t GL_COLOR_ATTACHMENT24 = 0x8CF8;
	static const uint32_t GL_COLOR_ATTACHMENT25 = 0x8CF9;
	static const uint32_t GL_COLOR_ATTACHMENT26 = 0x8CFA;
	static const uint32_t GL_COLOR_ATTACHMENT27 = 0x8CFB;
	static const uint32_t GL_COLOR_ATTACHMENT28 = 0x8CFC;
	static const uint32_t GL_COLOR_ATTACHMENT29 = 0x8CFD;
	static const uint32_t GL_COLOR_ATTACHMENT30 = 0x8CFE;
	static const uint32_t GL_COLOR_ATTACHMENT31 = 0x8CFF;
	static const uint32_t GL_DEPTH_ATTACHMENT = 0x8D00;
	static const uint32_t GL_STENCIL_ATTACHMENT = 0x8D20;
	static const uint32_t GL_FRAMEBUFFER = 0x8D40;
	static const uint32_t GL_RENDERBUFFER = 0x8D41;
	static const uint32_t GL_RENDERBUFFER_WIDTH = 0x8D42;
	static const uint32_t GL_RENDERBUFFER_HEIGHT = 0x8D43;
	static const uint32_t GL_RENDERBUFFER_INTERNAL_FORMAT = 0x8D44;
	static const uint32_t GL_STENCIL_INDEX1 = 0x8D46;
	static const uint32_t GL_STENCIL_INDEX4 = 0x8D47;
	static const uint32_t GL_STENCIL_INDEX8 = 0x8D48;
	static const uint32_t GL_STENCIL_INDEX16 = 0x8D49;
	static const uint32_t GL_RENDERBUFFER_RED_SIZE = 0x8D50;
	static const uint32_t GL_RENDERBUFFER_GREEN_SIZE = 0x8D51;
	static const uint32_t GL_RENDERBUFFER_BLUE_SIZE = 0x8D52;
	static const uint32_t GL_RENDERBUFFER_ALPHA_SIZE = 0x8D53;
	static const uint32_t GL_RENDERBUFFER_DEPTH_SIZE = 0x8D54;
	static const uint32_t GL_RENDERBUFFER_STENCIL_SIZE = 0x8D55;
	static const uint32_t GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE = 0x8D56;
	static const uint32_t GL_MAX_SAMPLES = 0x8D57;
	static const uint32_t GL_INDEX = 0x8222;
	static const uint32_t GL_TEXTURE_LUMINANCE_TYPE = 0x8C14;
	static const uint32_t GL_TEXTURE_INTENSITY_TYPE = 0x8C15;
	static const uint32_t GL_FRAMEBUFFER_SRGB = 0x8DB9;
	static const uint32_t GL_HALF_FLOAT = 0x140B;
	static const uint32_t GL_MAP_READ_BIT = 0x0001;
	static const uint32_t GL_MAP_WRITE_BIT = 0x0002;
	static const uint32_t GL_MAP_INVALIDATE_RANGE_BIT = 0x0004;
	static const uint32_t GL_MAP_INVALIDATE_BUFFER_BIT = 0x0008;
	static const uint32_t GL_MAP_FLUSH_EXPLICIT_BIT = 0x0010;
	static const uint32_t GL_MAP_UNSYNCHRONIZED_BIT = 0x0020;
	static const uint32_t GL_COMPRESSED_RED_RGTC1 = 0x8DBB;
	static const uint32_t GL_COMPRESSED_SIGNED_RED_RGTC1 = 0x8DBC;
	static const uint32_t GL_COMPRESSED_RG_RGTC2 = 0x8DBD;
	static const uint32_t GL_COMPRESSED_SIGNED_RG_RGTC2 = 0x8DBE;
	static const uint32_t GL_RG = 0x8227;
	static const uint32_t GL_RG_INTEGER = 0x8228;
	static const uint32_t GL_R8 = 0x8229;
	static const uint32_t GL_R16 = 0x822A;
	static const uint32_t GL_RG8 = 0x822B;
	static const uint32_t GL_RG16 = 0x822C;
	static const uint32_t GL_R16F = 0x822D;
	static const uint32_t GL_R32F = 0x822E;
	static const uint32_t GL_RG16F = 0x822F;
	static const uint32_t GL_RG32F = 0x8230;
	static const uint32_t GL_R8I = 0x8231;
	static const uint32_t GL_R8UI = 0x8232;
	static const uint32_t GL_R16I = 0x8233;
	static const uint32_t GL_R16UI = 0x8234;
	static const uint32_t GL_R32I = 0x8235;
	static const uint32_t GL_R32UI = 0x8236;
	static const uint32_t GL_RG8I = 0x8237;
	static const uint32_t GL_RG8UI = 0x8238;
	static const uint32_t GL_RG16I = 0x8239;
	static const uint32_t GL_RG16UI = 0x823A;
	static const uint32_t GL_RG32I = 0x823B;
	static const uint32_t GL_RG32UI = 0x823C;
	static const uint32_t GL_VERTEX_ARRAY_BINDING = 0x85B5;
	static const uint32_t GL_CLAMP_VERTEX_COLOR = 0x891A;
	static const uint32_t GL_CLAMP_FRAGMENT_COLOR = 0x891B;
	static const uint32_t GL_ALPHA_INTEGER = 0x8D97;
	typedef void (FDYNGL_APIENTRY gl_color_maski_func)(GLuint index, GLboolean r, GLboolean g, GLboolean b, GLboolean a);
	static gl_color_maski_func* glColorMaski;
	typedef void (FDYNGL_APIENTRY gl_get_booleani_v_func)(GLenum target, GLuint index, GLboolean *data);
	static gl_get_booleani_v_func* glGetBooleani_v;
	typedef void (FDYNGL_APIENTRY gl_get_integeri_v_func)(GLenum target, GLuint index, GLint *data);
	static gl_get_integeri_v_func* glGetIntegeri_v;
	typedef void (FDYNGL_APIENTRY gl_enablei_func)(GLenum target, GLuint index);
	static gl_enablei_func* glEnablei;
	typedef void (FDYNGL_APIENTRY gl_disablei_func)(GLenum target, GLuint index);
	static gl_disablei_func* glDisablei;
	typedef GLboolean(FDYNGL_APIENTRY gl_is_enabledi_func)(GLenum target, GLuint index);
	static gl_is_enabledi_func* glIsEnabledi;
	typedef void (FDYNGL_APIENTRY gl_begin_transform_feedback_func)(GLenum primitiveMode);
	static gl_begin_transform_feedback_func* glBeginTransformFeedback;
	typedef void (FDYNGL_APIENTRY gl_end_transform_feedback_func)(void);
	static gl_end_transform_feedback_func* glEndTransformFeedback;
	typedef void (FDYNGL_APIENTRY gl_bind_buffer_range_func)(GLenum target, GLuint index, GLuint buffer, GLintptr offset, GLsizeiptr size);
	static gl_bind_buffer_range_func* glBindBufferRange;
	typedef void (FDYNGL_APIENTRY gl_bind_buffer_base_func)(GLenum target, GLuint index, GLuint buffer);
	static gl_bind_buffer_base_func* glBindBufferBase;
	typedef void (FDYNGL_APIENTRY gl_transform_feedback_varyings_func)(GLuint program, GLsizei count, const GLchar *const*varyings, GLenum bufferMode);
	static gl_transform_feedback_varyings_func* glTransformFeedbackVaryings;
	typedef void (FDYNGL_APIENTRY gl_get_transform_feedback_varying_func)(GLuint program, GLuint index, GLsizei bufSize, GLsizei *length, GLsizei *size, GLenum *type, GLchar *name);
	static gl_get_transform_feedback_varying_func* glGetTransformFeedbackVarying;
	typedef void (FDYNGL_APIENTRY gl_clamp_color_func)(GLenum target, GLenum clamp);
	static gl_clamp_color_func* glClampColor;
	typedef void (FDYNGL_APIENTRY gl_begin_conditional_render_func)(GLuint id, GLenum mode);
	static gl_begin_conditional_render_func* glBeginConditionalRender;
	typedef void (FDYNGL_APIENTRY gl_end_conditional_render_func)(void);
	static gl_end_conditional_render_func* glEndConditionalRender;
	typedef void (FDYNGL_APIENTRY gl_vertex_attrib_i_pointer_func)(GLuint index, GLint size, GLenum type, GLsizei stride, const void *pointer);
	static gl_vertex_attrib_i_pointer_func* glVertexAttribIPointer;
	typedef void (FDYNGL_APIENTRY gl_get_vertex_attrib_iiv_func)(GLuint index, GLenum pname, GLint *params);
	static gl_get_vertex_attrib_iiv_func* glGetVertexAttribIiv;
	typedef void (FDYNGL_APIENTRY gl_get_vertex_attrib_iuiv_func)(GLuint index, GLenum pname, GLuint *params);
	static gl_get_vertex_attrib_iuiv_func* glGetVertexAttribIuiv;
	typedef void (FDYNGL_APIENTRY gl_vertex_attrib_i1i_func)(GLuint index, GLint x);
	static gl_vertex_attrib_i1i_func* glVertexAttribI1i;
	typedef void (FDYNGL_APIENTRY gl_vertex_attrib_i2i_func)(GLuint index, GLint x, GLint y);
	static gl_vertex_attrib_i2i_func* glVertexAttribI2i;
	typedef void (FDYNGL_APIENTRY gl_vertex_attrib_i3i_func)(GLuint index, GLint x, GLint y, GLint z);
	static gl_vertex_attrib_i3i_func* glVertexAttribI3i;
	typedef void (FDYNGL_APIENTRY gl_vertex_attrib_i4i_func)(GLuint index, GLint x, GLint y, GLint z, GLint w);
	static gl_vertex_attrib_i4i_func* glVertexAttribI4i;
	typedef void (FDYNGL_APIENTRY gl_vertex_attrib_i1ui_func)(GLuint index, GLuint x);
	static gl_vertex_attrib_i1ui_func* glVertexAttribI1ui;
	typedef void (FDYNGL_APIENTRY gl_vertex_attrib_i2ui_func)(GLuint index, GLuint x, GLuint y);
	static gl_vertex_attrib_i2ui_func* glVertexAttribI2ui;
	typedef void (FDYNGL_APIENTRY gl_vertex_attrib_i3ui_func)(GLuint index, GLuint x, GLuint y, GLuint z);
	static gl_vertex_attrib_i3ui_func* glVertexAttribI3ui;
	typedef void (FDYNGL_APIENTRY gl_vertex_attrib_i4ui_func)(GLuint index, GLuint x, GLuint y, GLuint z, GLuint w);
	static gl_vertex_attrib_i4ui_func* glVertexAttribI4ui;
	typedef void (FDYNGL_APIENTRY gl_vertex_attrib_i1iv_func)(GLuint index, const GLint *v);
	static gl_vertex_attrib_i1iv_func* glVertexAttribI1iv;
	typedef void (FDYNGL_APIENTRY gl_vertex_attrib_i2iv_func)(GLuint index, const GLint *v);
	static gl_vertex_attrib_i2iv_func* glVertexAttribI2iv;
	typedef void (FDYNGL_APIENTRY gl_vertex_attrib_i3iv_func)(GLuint index, const GLint *v);
	static gl_vertex_attrib_i3iv_func* glVertexAttribI3iv;
	typedef void (FDYNGL_APIENTRY gl_vertex_attrib_i4iv_func)(GLuint index, const GLint *v);
	static gl_vertex_attrib_i4iv_func* glVertexAttribI4iv;
	typedef void (FDYNGL_APIENTRY gl_vertex_attrib_i1uiv_func)(GLuint index, const GLuint *v);
	static gl_vertex_attrib_i1uiv_func* glVertexAttribI1uiv;
	typedef void (FDYNGL_APIENTRY gl_vertex_attrib_i2uiv_func)(GLuint index, const GLuint *v);
	static gl_vertex_attrib_i2uiv_func* glVertexAttribI2uiv;
	typedef void (FDYNGL_APIENTRY gl_vertex_attrib_i3uiv_func)(GLuint index, const GLuint *v);
	static gl_vertex_attrib_i3uiv_func* glVertexAttribI3uiv;
	typedef void (FDYNGL_APIENTRY gl_vertex_attrib_i4uiv_func)(GLuint index, const GLuint *v);
	static gl_vertex_attrib_i4uiv_func* glVertexAttribI4uiv;
	typedef void (FDYNGL_APIENTRY gl_vertex_attrib_i4bv_func)(GLuint index, const GLbyte *v);
	static gl_vertex_attrib_i4bv_func* glVertexAttribI4bv;
	typedef void (FDYNGL_APIENTRY gl_vertex_attrib_i4sv_func)(GLuint index, const GLshort *v);
	static gl_vertex_attrib_i4sv_func* glVertexAttribI4sv;
	typedef void (FDYNGL_APIENTRY gl_vertex_attrib_i4ubv_func)(GLuint index, const GLubyte *v);
	static gl_vertex_attrib_i4ubv_func* glVertexAttribI4ubv;
	typedef void (FDYNGL_APIENTRY gl_vertex_attrib_i4usv_func)(GLuint index, const GLushort *v);
	static gl_vertex_attrib_i4usv_func* glVertexAttribI4usv;
	typedef void (FDYNGL_APIENTRY gl_get_uniformuiv_func)(GLuint program, GLint location, GLuint *params);
	static gl_get_uniformuiv_func* glGetUniformuiv;
	typedef void (FDYNGL_APIENTRY gl_bind_frag_data_location_func)(GLuint program, GLuint color, const GLchar *name);
	static gl_bind_frag_data_location_func* glBindFragDataLocation;
	typedef GLint(FDYNGL_APIENTRY gl_get_frag_data_location_func)(GLuint program, const GLchar *name);
	static gl_get_frag_data_location_func* glGetFragDataLocation;
	typedef void (FDYNGL_APIENTRY gl_uniform1ui_func)(GLint location, GLuint v0);
	static gl_uniform1ui_func* glUniform1ui;
	typedef void (FDYNGL_APIENTRY gl_uniform2ui_func)(GLint location, GLuint v0, GLuint v1);
	static gl_uniform2ui_func* glUniform2ui;
	typedef void (FDYNGL_APIENTRY gl_uniform3ui_func)(GLint location, GLuint v0, GLuint v1, GLuint v2);
	static gl_uniform3ui_func* glUniform3ui;
	typedef void (FDYNGL_APIENTRY gl_uniform4ui_func)(GLint location, GLuint v0, GLuint v1, GLuint v2, GLuint v3);
	static gl_uniform4ui_func* glUniform4ui;
	typedef void (FDYNGL_APIENTRY gl_uniform1uiv_func)(GLint location, GLsizei count, const GLuint *value);
	static gl_uniform1uiv_func* glUniform1uiv;
	typedef void (FDYNGL_APIENTRY gl_uniform2uiv_func)(GLint location, GLsizei count, const GLuint *value);
	static gl_uniform2uiv_func* glUniform2uiv;
	typedef void (FDYNGL_APIENTRY gl_uniform3uiv_func)(GLint location, GLsizei count, const GLuint *value);
	static gl_uniform3uiv_func* glUniform3uiv;
	typedef void (FDYNGL_APIENTRY gl_uniform4uiv_func)(GLint location, GLsizei count, const GLuint *value);
	static gl_uniform4uiv_func* glUniform4uiv;
	typedef void (FDYNGL_APIENTRY gl_tex_parameter_iiv_func)(GLenum target, GLenum pname, const GLint *params);
	static gl_tex_parameter_iiv_func* glTexParameterIiv;
	typedef void (FDYNGL_APIENTRY gl_tex_parameter_iuiv_func)(GLenum target, GLenum pname, const GLuint *params);
	static gl_tex_parameter_iuiv_func* glTexParameterIuiv;
	typedef void (FDYNGL_APIENTRY gl_get_tex_parameter_iiv_func)(GLenum target, GLenum pname, GLint *params);
	static gl_get_tex_parameter_iiv_func* glGetTexParameterIiv;
	typedef void (FDYNGL_APIENTRY gl_get_tex_parameter_iuiv_func)(GLenum target, GLenum pname, GLuint *params);
	static gl_get_tex_parameter_iuiv_func* glGetTexParameterIuiv;
	typedef void (FDYNGL_APIENTRY gl_clear_bufferiv_func)(GLenum buffer, GLint drawbuffer, const GLint *value);
	static gl_clear_bufferiv_func* glClearBufferiv;
	typedef void (FDYNGL_APIENTRY gl_clear_bufferuiv_func)(GLenum buffer, GLint drawbuffer, const GLuint *value);
	static gl_clear_bufferuiv_func* glClearBufferuiv;
	typedef void (FDYNGL_APIENTRY gl_clear_bufferfv_func)(GLenum buffer, GLint drawbuffer, const GLfloat *value);
	static gl_clear_bufferfv_func* glClearBufferfv;
	typedef void (FDYNGL_APIENTRY gl_clear_bufferfi_func)(GLenum buffer, GLint drawbuffer, GLfloat depth, GLint stencil);
	static gl_clear_bufferfi_func* glClearBufferfi;
	typedef const GLubyte * (FDYNGL_APIENTRY gl_get_stringi_func)(GLenum name, GLuint index);
	static gl_get_stringi_func* glGetStringi;
	typedef GLboolean(FDYNGL_APIENTRY gl_is_renderbuffer_func)(GLuint renderbuffer);
	static gl_is_renderbuffer_func* glIsRenderbuffer;
	typedef void (FDYNGL_APIENTRY gl_bind_renderbuffer_func)(GLenum target, GLuint renderbuffer);
	static gl_bind_renderbuffer_func* glBindRenderbuffer;
	typedef void (FDYNGL_APIENTRY gl_delete_renderbuffers_func)(GLsizei n, const GLuint *renderbuffers);
	static gl_delete_renderbuffers_func* glDeleteRenderbuffers;
	typedef void (FDYNGL_APIENTRY gl_gen_renderbuffers_func)(GLsizei n, GLuint *renderbuffers);
	static gl_gen_renderbuffers_func* glGenRenderbuffers;
	typedef void (FDYNGL_APIENTRY gl_renderbuffer_storage_func)(GLenum target, GLenum internalformat, GLsizei width, GLsizei height);
	static gl_renderbuffer_storage_func* glRenderbufferStorage;
	typedef void (FDYNGL_APIENTRY gl_get_renderbuffer_parameteriv_func)(GLenum target, GLenum pname, GLint *params);
	static gl_get_renderbuffer_parameteriv_func* glGetRenderbufferParameteriv;
	typedef GLboolean(FDYNGL_APIENTRY gl_is_framebuffer_func)(GLuint framebuffer);
	static gl_is_framebuffer_func* glIsFramebuffer;
	typedef void (FDYNGL_APIENTRY gl_bind_framebuffer_func)(GLenum target, GLuint framebuffer);
	static gl_bind_framebuffer_func* glBindFramebuffer;
	typedef void (FDYNGL_APIENTRY gl_delete_framebuffers_func)(GLsizei n, const GLuint *framebuffers);
	static gl_delete_framebuffers_func* glDeleteFramebuffers;
	typedef void (FDYNGL_APIENTRY gl_gen_framebuffers_func)(GLsizei n, GLuint *framebuffers);
	static gl_gen_framebuffers_func* glGenFramebuffers;
	typedef GLenum(FDYNGL_APIENTRY gl_check_framebuffer_status_func)(GLenum target);
	static gl_check_framebuffer_status_func* glCheckFramebufferStatus;
	typedef void (FDYNGL_APIENTRY gl_framebuffer_texture1d_func)(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level);
	static gl_framebuffer_texture1d_func* glFramebufferTexture1D;
	typedef void (FDYNGL_APIENTRY gl_framebuffer_texture2d_func)(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level);
	static gl_framebuffer_texture2d_func* glFramebufferTexture2D;
	typedef void (FDYNGL_APIENTRY gl_framebuffer_texture3d_func)(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level, GLint zoffset);
	static gl_framebuffer_texture3d_func* glFramebufferTexture3D;
	typedef void (FDYNGL_APIENTRY gl_framebuffer_renderbuffer_func)(GLenum target, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer);
	static gl_framebuffer_renderbuffer_func* glFramebufferRenderbuffer;
	typedef void (FDYNGL_APIENTRY gl_get_framebuffer_attachment_parameteriv_func)(GLenum target, GLenum attachment, GLenum pname, GLint *params);
	static gl_get_framebuffer_attachment_parameteriv_func* glGetFramebufferAttachmentParameteriv;
	typedef void (FDYNGL_APIENTRY gl_generate_mipmap_func)(GLenum target);
	static gl_generate_mipmap_func* glGenerateMipmap;
	typedef void (FDYNGL_APIENTRY gl_blit_framebuffer_func)(GLint srcX0, GLint srcY0, GLint srcX1, GLint srcY1, GLint dstX0, GLint dstY0, GLint dstX1, GLint dstY1, GLbitfield mask, GLenum filter);
	static gl_blit_framebuffer_func* glBlitFramebuffer;
	typedef void (FDYNGL_APIENTRY gl_renderbuffer_storage_multisample_func)(GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height);
	static gl_renderbuffer_storage_multisample_func* glRenderbufferStorageMultisample;
	typedef void (FDYNGL_APIENTRY gl_framebuffer_texture_layer_func)(GLenum target, GLenum attachment, GLuint texture, GLint level, GLint layer);
	static gl_framebuffer_texture_layer_func* glFramebufferTextureLayer;
	typedef void * (FDYNGL_APIENTRY gl_map_buffer_range_func)(GLenum target, GLintptr offset, GLsizeiptr length, GLbitfield access);
	static gl_map_buffer_range_func* glMapBufferRange;
	typedef void (FDYNGL_APIENTRY gl_flush_mapped_buffer_range_func)(GLenum target, GLintptr offset, GLsizeiptr length);
	static gl_flush_mapped_buffer_range_func* glFlushMappedBufferRange;
	typedef void (FDYNGL_APIENTRY gl_bind_vertex_array_func)(GLuint array);
	static gl_bind_vertex_array_func* glBindVertexArray;
	typedef void (FDYNGL_APIENTRY gl_delete_vertex_arrays_func)(GLsizei n, const GLuint *arrays);
	static gl_delete_vertex_arrays_func* glDeleteVertexArrays;
	typedef void (FDYNGL_APIENTRY gl_gen_vertex_arrays_func)(GLsizei n, GLuint *arrays);
	static gl_gen_vertex_arrays_func* glGenVertexArrays;
	typedef GLboolean(FDYNGL_APIENTRY gl_is_vertex_array_func)(GLuint array);
	static gl_is_vertex_array_func* glIsVertexArray;
#	endif // GL_VERSION_3_0
#	ifndef GL_VERSION_3_1
#		define GL_VERSION_3_1 1
	static bool isGL_VERSION_3_1;
	static const uint32_t GL_SAMPLER_2D_RECT = 0x8B63;
	static const uint32_t GL_SAMPLER_2D_RECT_SHADOW = 0x8B64;
	static const uint32_t GL_SAMPLER_BUFFER = 0x8DC2;
	static const uint32_t GL_INT_SAMPLER_2D_RECT = 0x8DCD;
	static const uint32_t GL_INT_SAMPLER_BUFFER = 0x8DD0;
	static const uint32_t GL_UNSIGNED_INT_SAMPLER_2D_RECT = 0x8DD5;
	static const uint32_t GL_UNSIGNED_INT_SAMPLER_BUFFER = 0x8DD8;
	static const uint32_t GL_TEXTURE_BUFFER = 0x8C2A;
	static const uint32_t GL_MAX_TEXTURE_BUFFER_SIZE = 0x8C2B;
	static const uint32_t GL_TEXTURE_BINDING_BUFFER = 0x8C2C;
	static const uint32_t GL_TEXTURE_BUFFER_DATA_STORE_BINDING = 0x8C2D;
	static const uint32_t GL_TEXTURE_RECTANGLE = 0x84F5;
	static const uint32_t GL_TEXTURE_BINDING_RECTANGLE = 0x84F6;
	static const uint32_t GL_PROXY_TEXTURE_RECTANGLE = 0x84F7;
	static const uint32_t GL_MAX_RECTANGLE_TEXTURE_SIZE = 0x84F8;
	static const uint32_t GL_R8_SNORM = 0x8F94;
	static const uint32_t GL_RG8_SNORM = 0x8F95;
	static const uint32_t GL_RGB8_SNORM = 0x8F96;
	static const uint32_t GL_RGBA8_SNORM = 0x8F97;
	static const uint32_t GL_R16_SNORM = 0x8F98;
	static const uint32_t GL_RG16_SNORM = 0x8F99;
	static const uint32_t GL_RGB16_SNORM = 0x8F9A;
	static const uint32_t GL_RGBA16_SNORM = 0x8F9B;
	static const uint32_t GL_SIGNED_NORMALIZED = 0x8F9C;
	static const uint32_t GL_PRIMITIVE_RESTART = 0x8F9D;
	static const uint32_t GL_PRIMITIVE_RESTART_INDEX = 0x8F9E;
	static const uint32_t GL_COPY_READ_BUFFER = 0x8F36;
	static const uint32_t GL_COPY_WRITE_BUFFER = 0x8F37;
	static const uint32_t GL_UNIFORM_BUFFER = 0x8A11;
	static const uint32_t GL_UNIFORM_BUFFER_BINDING = 0x8A28;
	static const uint32_t GL_UNIFORM_BUFFER_START = 0x8A29;
	static const uint32_t GL_UNIFORM_BUFFER_SIZE = 0x8A2A;
	static const uint32_t GL_MAX_VERTEX_UNIFORM_BLOCKS = 0x8A2B;
	static const uint32_t GL_MAX_GEOMETRY_UNIFORM_BLOCKS = 0x8A2C;
	static const uint32_t GL_MAX_FRAGMENT_UNIFORM_BLOCKS = 0x8A2D;
	static const uint32_t GL_MAX_COMBINED_UNIFORM_BLOCKS = 0x8A2E;
	static const uint32_t GL_MAX_UNIFORM_BUFFER_BINDINGS = 0x8A2F;
	static const uint32_t GL_MAX_UNIFORM_BLOCK_SIZE = 0x8A30;
	static const uint32_t GL_MAX_COMBINED_VERTEX_UNIFORM_COMPONENTS = 0x8A31;
	static const uint32_t GL_MAX_COMBINED_GEOMETRY_UNIFORM_COMPONENTS = 0x8A32;
	static const uint32_t GL_MAX_COMBINED_FRAGMENT_UNIFORM_COMPONENTS = 0x8A33;
	static const uint32_t GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT = 0x8A34;
	static const uint32_t GL_ACTIVE_UNIFORM_BLOCK_MAX_NAME_LENGTH = 0x8A35;
	static const uint32_t GL_ACTIVE_UNIFORM_BLOCKS = 0x8A36;
	static const uint32_t GL_UNIFORM_TYPE = 0x8A37;
	static const uint32_t GL_UNIFORM_SIZE = 0x8A38;
	static const uint32_t GL_UNIFORM_NAME_LENGTH = 0x8A39;
	static const uint32_t GL_UNIFORM_BLOCK_INDEX = 0x8A3A;
	static const uint32_t GL_UNIFORM_OFFSET = 0x8A3B;
	static const uint32_t GL_UNIFORM_ARRAY_STRIDE = 0x8A3C;
	static const uint32_t GL_UNIFORM_MATRIX_STRIDE = 0x8A3D;
	static const uint32_t GL_UNIFORM_IS_ROW_MAJOR = 0x8A3E;
	static const uint32_t GL_UNIFORM_BLOCK_BINDING = 0x8A3F;
	static const uint32_t GL_UNIFORM_BLOCK_DATA_SIZE = 0x8A40;
	static const uint32_t GL_UNIFORM_BLOCK_NAME_LENGTH = 0x8A41;
	static const uint32_t GL_UNIFORM_BLOCK_ACTIVE_UNIFORMS = 0x8A42;
	static const uint32_t GL_UNIFORM_BLOCK_ACTIVE_UNIFORM_INDICES = 0x8A43;
	static const uint32_t GL_UNIFORM_BLOCK_REFERENCED_BY_VERTEX_SHADER = 0x8A44;
	static const uint32_t GL_UNIFORM_BLOCK_REFERENCED_BY_GEOMETRY_SHADER = 0x8A45;
	static const uint32_t GL_UNIFORM_BLOCK_REFERENCED_BY_FRAGMENT_SHADER = 0x8A46;
	static const uint32_t GL_INVALID_INDEX = 0xFFFFFFFFu;
	typedef void (FDYNGL_APIENTRY gl_draw_arrays_instanced_func)(GLenum mode, GLint first, GLsizei count, GLsizei instancecount);
	static gl_draw_arrays_instanced_func* glDrawArraysInstanced;
	typedef void (FDYNGL_APIENTRY gl_draw_elements_instanced_func)(GLenum mode, GLsizei count, GLenum type, const void *indices, GLsizei instancecount);
	static gl_draw_elements_instanced_func* glDrawElementsInstanced;
	typedef void (FDYNGL_APIENTRY gl_tex_buffer_func)(GLenum target, GLenum internalformat, GLuint buffer);
	static gl_tex_buffer_func* glTexBuffer;
	typedef void (FDYNGL_APIENTRY gl_primitive_restart_index_func)(GLuint index);
	static gl_primitive_restart_index_func* glPrimitiveRestartIndex;
	typedef void (FDYNGL_APIENTRY gl_copy_buffer_sub_data_func)(GLenum readTarget, GLenum writeTarget, GLintptr readOffset, GLintptr writeOffset, GLsizeiptr size);
	static gl_copy_buffer_sub_data_func* glCopyBufferSubData;
	typedef void (FDYNGL_APIENTRY gl_get_uniform_indices_func)(GLuint program, GLsizei uniformCount, const GLchar *const*uniformNames, GLuint *uniformIndices);
	static gl_get_uniform_indices_func* glGetUniformIndices;
	typedef void (FDYNGL_APIENTRY gl_get_active_uniformsiv_func)(GLuint program, GLsizei uniformCount, const GLuint *uniformIndices, GLenum pname, GLint *params);
	static gl_get_active_uniformsiv_func* glGetActiveUniformsiv;
	typedef void (FDYNGL_APIENTRY gl_get_active_uniform_name_func)(GLuint program, GLuint uniformIndex, GLsizei bufSize, GLsizei *length, GLchar *uniformName);
	static gl_get_active_uniform_name_func* glGetActiveUniformName;
	typedef GLuint(FDYNGL_APIENTRY gl_get_uniform_block_index_func)(GLuint program, const GLchar *uniformBlockName);
	static gl_get_uniform_block_index_func* glGetUniformBlockIndex;
	typedef void (FDYNGL_APIENTRY gl_get_active_uniform_blockiv_func)(GLuint program, GLuint uniformBlockIndex, GLenum pname, GLint *params);
	static gl_get_active_uniform_blockiv_func* glGetActiveUniformBlockiv;
	typedef void (FDYNGL_APIENTRY gl_get_active_uniform_block_name_func)(GLuint program, GLuint uniformBlockIndex, GLsizei bufSize, GLsizei *length, GLchar *uniformBlockName);
	static gl_get_active_uniform_block_name_func* glGetActiveUniformBlockName;
	typedef void (FDYNGL_APIENTRY gl_uniform_block_binding_func)(GLuint program, GLuint uniformBlockIndex, GLuint uniformBlockBinding);
	static gl_uniform_block_binding_func* glUniformBlockBinding;
#	endif // GL_VERSION_3_1
#	ifndef GL_VERSION_3_2
#		define GL_VERSION_3_2 1
	static bool isGL_VERSION_3_2;
	typedef struct __GLsync *GLsync;
	typedef uint64_t GLuint64;
	typedef int64_t GLint64;
	static const uint32_t GL_CONTEXT_CORE_PROFILE_BIT = 0x00000001;
	static const uint32_t GL_CONTEXT_COMPATIBILITY_PROFILE_BIT = 0x00000002;
	static const uint32_t GL_LINES_ADJACENCY = 0x000A;
	static const uint32_t GL_LINE_STRIP_ADJACENCY = 0x000B;
	static const uint32_t GL_TRIANGLES_ADJACENCY = 0x000C;
	static const uint32_t GL_TRIANGLE_STRIP_ADJACENCY = 0x000D;
	static const uint32_t GL_PROGRAM_POINT_SIZE = 0x8642;
	static const uint32_t GL_MAX_GEOMETRY_TEXTURE_IMAGE_UNITS = 0x8C29;
	static const uint32_t GL_FRAMEBUFFER_ATTACHMENT_LAYERED = 0x8DA7;
	static const uint32_t GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS = 0x8DA8;
	static const uint32_t GL_GEOMETRY_SHADER = 0x8DD9;
	static const uint32_t GL_GEOMETRY_VERTICES_OUT = 0x8916;
	static const uint32_t GL_GEOMETRY_INPUT_TYPE = 0x8917;
	static const uint32_t GL_GEOMETRY_OUTPUT_TYPE = 0x8918;
	static const uint32_t GL_MAX_GEOMETRY_UNIFORM_COMPONENTS = 0x8DDF;
	static const uint32_t GL_MAX_GEOMETRY_OUTPUT_VERTICES = 0x8DE0;
	static const uint32_t GL_MAX_GEOMETRY_TOTAL_OUTPUT_COMPONENTS = 0x8DE1;
	static const uint32_t GL_MAX_VERTEX_OUTPUT_COMPONENTS = 0x9122;
	static const uint32_t GL_MAX_GEOMETRY_INPUT_COMPONENTS = 0x9123;
	static const uint32_t GL_MAX_GEOMETRY_OUTPUT_COMPONENTS = 0x9124;
	static const uint32_t GL_MAX_FRAGMENT_INPUT_COMPONENTS = 0x9125;
	static const uint32_t GL_CONTEXT_PROFILE_MASK = 0x9126;
	static const uint32_t GL_DEPTH_CLAMP = 0x864F;
	static const uint32_t GL_QUADS_FOLLOW_PROVOKING_VERTEX_CONVENTION = 0x8E4C;
	static const uint32_t GL_FIRST_VERTEX_CONVENTION = 0x8E4D;
	static const uint32_t GL_LAST_VERTEX_CONVENTION = 0x8E4E;
	static const uint32_t GL_PROVOKING_VERTEX = 0x8E4F;
	static const uint32_t GL_TEXTURE_CUBE_MAP_SEAMLESS = 0x884F;
	static const uint32_t GL_MAX_SERVER_WAIT_TIMEOUT = 0x9111;
	static const uint32_t GL_OBJECT_TYPE = 0x9112;
	static const uint32_t GL_SYNC_CONDITION = 0x9113;
	static const uint32_t GL_SYNC_STATUS = 0x9114;
	static const uint32_t GL_SYNC_FLAGS = 0x9115;
	static const uint32_t GL_SYNC_FENCE = 0x9116;
	static const uint32_t GL_SYNC_GPU_COMMANDS_COMPLETE = 0x9117;
	static const uint32_t GL_UNSIGNALED = 0x9118;
	static const uint32_t GL_SIGNALED = 0x9119;
	static const uint32_t GL_ALREADY_SIGNALED = 0x911A;
	static const uint32_t GL_TIMEOUT_EXPIRED = 0x911B;
	static const uint32_t GL_CONDITION_SATISFIED = 0x911C;
	static const uint32_t GL_WAIT_FAILED = 0x911D;
	static const uint64_t GL_TIMEOUT_IGNORED = 0xFFFFFFFFFFFFFFFFull;
	static const uint32_t GL_SYNC_FLUSH_COMMANDS_BIT = 0x00000001;
	static const uint32_t GL_SAMPLE_POSITION = 0x8E50;
	static const uint32_t GL_SAMPLE_MASK = 0x8E51;
	static const uint32_t GL_SAMPLE_MASK_VALUE = 0x8E52;
	static const uint32_t GL_MAX_SAMPLE_MASK_WORDS = 0x8E59;
	static const uint32_t GL_TEXTURE_2D_MULTISAMPLE = 0x9100;
	static const uint32_t GL_PROXY_TEXTURE_2D_MULTISAMPLE = 0x9101;
	static const uint32_t GL_TEXTURE_2D_MULTISAMPLE_ARRAY = 0x9102;
	static const uint32_t GL_PROXY_TEXTURE_2D_MULTISAMPLE_ARRAY = 0x9103;
	static const uint32_t GL_TEXTURE_BINDING_2D_MULTISAMPLE = 0x9104;
	static const uint32_t GL_TEXTURE_BINDING_2D_MULTISAMPLE_ARRAY = 0x9105;
	static const uint32_t GL_TEXTURE_SAMPLES = 0x9106;
	static const uint32_t GL_TEXTURE_FIXED_SAMPLE_LOCATIONS = 0x9107;
	static const uint32_t GL_SAMPLER_2D_MULTISAMPLE = 0x9108;
	static const uint32_t GL_INT_SAMPLER_2D_MULTISAMPLE = 0x9109;
	static const uint32_t GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE = 0x910A;
	static const uint32_t GL_SAMPLER_2D_MULTISAMPLE_ARRAY = 0x910B;
	static const uint32_t GL_INT_SAMPLER_2D_MULTISAMPLE_ARRAY = 0x910C;
	static const uint32_t GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE_ARRAY = 0x910D;
	static const uint32_t GL_MAX_COLOR_TEXTURE_SAMPLES = 0x910E;
	static const uint32_t GL_MAX_DEPTH_TEXTURE_SAMPLES = 0x910F;
	static const uint32_t GL_MAX_INTEGER_SAMPLES = 0x9110;
	typedef void (FDYNGL_APIENTRY gl_draw_elements_base_vertex_func)(GLenum mode, GLsizei count, GLenum type, const void *indices, GLint basevertex);
	static gl_draw_elements_base_vertex_func* glDrawElementsBaseVertex;
	typedef void (FDYNGL_APIENTRY gl_draw_range_elements_base_vertex_func)(GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, const void *indices, GLint basevertex);
	static gl_draw_range_elements_base_vertex_func* glDrawRangeElementsBaseVertex;
	typedef void (FDYNGL_APIENTRY gl_draw_elements_instanced_base_vertex_func)(GLenum mode, GLsizei count, GLenum type, const void *indices, GLsizei instancecount, GLint basevertex);
	static gl_draw_elements_instanced_base_vertex_func* glDrawElementsInstancedBaseVertex;
	typedef void (FDYNGL_APIENTRY gl_multi_draw_elements_base_vertex_func)(GLenum mode, const GLsizei *count, GLenum type, const void *const*indices, GLsizei drawcount, const GLint *basevertex);
	static gl_multi_draw_elements_base_vertex_func* glMultiDrawElementsBaseVertex;
	typedef void (FDYNGL_APIENTRY gl_provoking_vertex_func)(GLenum mode);
	static gl_provoking_vertex_func* glProvokingVertex;
	typedef GLsync(FDYNGL_APIENTRY gl_fence_sync_func)(GLenum condition, GLbitfield flags);
	static gl_fence_sync_func* glFenceSync;
	typedef GLboolean(FDYNGL_APIENTRY gl_is_sync_func)(GLsync sync);
	static gl_is_sync_func* glIsSync;
	typedef void (FDYNGL_APIENTRY gl_delete_sync_func)(GLsync sync);
	static gl_delete_sync_func* glDeleteSync;
	typedef GLenum(FDYNGL_APIENTRY gl_client_wait_sync_func)(GLsync sync, GLbitfield flags, GLuint64 timeout);
	static gl_client_wait_sync_func* glClientWaitSync;
	typedef void (FDYNGL_APIENTRY gl_wait_sync_func)(GLsync sync, GLbitfield flags, GLuint64 timeout);
	static gl_wait_sync_func* glWaitSync;
	typedef void (FDYNGL_APIENTRY gl_get_integer64v_func)(GLenum pname, GLint64 *data);
	static gl_get_integer64v_func* glGetInteger64v;
	typedef void (FDYNGL_APIENTRY gl_get_synciv_func)(GLsync sync, GLenum pname, GLsizei bufSize, GLsizei *length, GLint *values);
	static gl_get_synciv_func* glGetSynciv;
	typedef void (FDYNGL_APIENTRY gl_get_integer64i_v_func)(GLenum target, GLuint index, GLint64 *data);
	static gl_get_integer64i_v_func* glGetInteger64i_v;
	typedef void (FDYNGL_APIENTRY gl_get_buffer_parameteri64v_func)(GLenum target, GLenum pname, GLint64 *params);
	static gl_get_buffer_parameteri64v_func* glGetBufferParameteri64v;
	typedef void (FDYNGL_APIENTRY gl_framebuffer_texture_func)(GLenum target, GLenum attachment, GLuint texture, GLint level);
	static gl_framebuffer_texture_func* glFramebufferTexture;
	typedef void (FDYNGL_APIENTRY gl_tex_image2_d_multisample_func)(GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height, GLboolean fixedsamplelocations);
	static gl_tex_image2_d_multisample_func* glTexImage2DMultisample;
	typedef void (FDYNGL_APIENTRY gl_tex_image3_d_multisample_func)(GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLboolean fixedsamplelocations);
	static gl_tex_image3_d_multisample_func* glTexImage3DMultisample;
	typedef void (FDYNGL_APIENTRY gl_get_multisamplefv_func)(GLenum pname, GLuint index, GLfloat *val);
	static gl_get_multisamplefv_func* glGetMultisamplefv;
	typedef void (FDYNGL_APIENTRY gl_sample_maski_func)(GLuint maskNumber, GLbitfield mask);
	static gl_sample_maski_func* glSampleMaski;
#	endif // GL_VERSION_3_2
#	ifndef GL_VERSION_3_3
#		define GL_VERSION_3_3 1
	static bool isGL_VERSION_3_3;
	static const uint32_t GL_VERTEX_ATTRIB_ARRAY_DIVISOR = 0x88FE;
	static const uint32_t GL_SRC1_COLOR = 0x88F9;
	static const uint32_t GL_ONE_MINUS_SRC1_COLOR = 0x88FA;
	static const uint32_t GL_ONE_MINUS_SRC1_ALPHA = 0x88FB;
	static const uint32_t GL_MAX_DUAL_SOURCE_DRAW_BUFFERS = 0x88FC;
	static const uint32_t GL_ANY_SAMPLES_PASSED = 0x8C2F;
	static const uint32_t GL_SAMPLER_BINDING = 0x8919;
	static const uint32_t GL_RGB10_A2UI = 0x906F;
	static const uint32_t GL_TEXTURE_SWIZZLE_R = 0x8E42;
	static const uint32_t GL_TEXTURE_SWIZZLE_G = 0x8E43;
	static const uint32_t GL_TEXTURE_SWIZZLE_B = 0x8E44;
	static const uint32_t GL_TEXTURE_SWIZZLE_A = 0x8E45;
	static const uint32_t GL_TEXTURE_SWIZZLE_RGBA = 0x8E46;
	static const uint32_t GL_TIME_ELAPSED = 0x88BF;
	static const uint32_t GL_TIMESTAMP = 0x8E28;
	static const uint32_t GL_INT_2_10_10_10_REV = 0x8D9F;
	typedef void (FDYNGL_APIENTRY gl_bind_frag_data_location_indexed_func)(GLuint program, GLuint colorNumber, GLuint index, const GLchar *name);
	static gl_bind_frag_data_location_indexed_func* glBindFragDataLocationIndexed;
	typedef GLint(FDYNGL_APIENTRY gl_get_frag_data_index_func)(GLuint program, const GLchar *name);
	static gl_get_frag_data_index_func* glGetFragDataIndex;
	typedef void (FDYNGL_APIENTRY gl_gen_samplers_func)(GLsizei count, GLuint *samplers);
	static gl_gen_samplers_func* glGenSamplers;
	typedef void (FDYNGL_APIENTRY gl_delete_samplers_func)(GLsizei count, const GLuint *samplers);
	static gl_delete_samplers_func* glDeleteSamplers;
	typedef GLboolean(FDYNGL_APIENTRY gl_is_sampler_func)(GLuint sampler);
	static gl_is_sampler_func* glIsSampler;
	typedef void (FDYNGL_APIENTRY gl_bind_sampler_func)(GLuint unit, GLuint sampler);
	static gl_bind_sampler_func* glBindSampler;
	typedef void (FDYNGL_APIENTRY gl_sampler_parameteri_func)(GLuint sampler, GLenum pname, GLint param);
	static gl_sampler_parameteri_func* glSamplerParameteri;
	typedef void (FDYNGL_APIENTRY gl_sampler_parameteriv_func)(GLuint sampler, GLenum pname, const GLint *param);
	static gl_sampler_parameteriv_func* glSamplerParameteriv;
	typedef void (FDYNGL_APIENTRY gl_sampler_parameterf_func)(GLuint sampler, GLenum pname, GLfloat param);
	static gl_sampler_parameterf_func* glSamplerParameterf;
	typedef void (FDYNGL_APIENTRY gl_sampler_parameterfv_func)(GLuint sampler, GLenum pname, const GLfloat *param);
	static gl_sampler_parameterfv_func* glSamplerParameterfv;
	typedef void (FDYNGL_APIENTRY gl_sampler_parameter_iiv_func)(GLuint sampler, GLenum pname, const GLint *param);
	static gl_sampler_parameter_iiv_func* glSamplerParameterIiv;
	typedef void (FDYNGL_APIENTRY gl_sampler_parameter_iuiv_func)(GLuint sampler, GLenum pname, const GLuint *param);
	static gl_sampler_parameter_iuiv_func* glSamplerParameterIuiv;
	typedef void (FDYNGL_APIENTRY gl_get_sampler_parameteriv_func)(GLuint sampler, GLenum pname, GLint *params);
	static gl_get_sampler_parameteriv_func* glGetSamplerParameteriv;
	typedef void (FDYNGL_APIENTRY gl_get_sampler_parameter_iiv_func)(GLuint sampler, GLenum pname, GLint *params);
	static gl_get_sampler_parameter_iiv_func* glGetSamplerParameterIiv;
	typedef void (FDYNGL_APIENTRY gl_get_sampler_parameterfv_func)(GLuint sampler, GLenum pname, GLfloat *params);
	static gl_get_sampler_parameterfv_func* glGetSamplerParameterfv;
	typedef void (FDYNGL_APIENTRY gl_get_sampler_parameter_iuiv_func)(GLuint sampler, GLenum pname, GLuint *params);
	static gl_get_sampler_parameter_iuiv_func* glGetSamplerParameterIuiv;
	typedef void (FDYNGL_APIENTRY gl_query_counter_func)(GLuint id, GLenum target);
	static gl_query_counter_func* glQueryCounter;
	typedef void (FDYNGL_APIENTRY gl_get_query_objecti64v_func)(GLuint id, GLenum pname, GLint64 *params);
	static gl_get_query_objecti64v_func* glGetQueryObjecti64v;
	typedef void (FDYNGL_APIENTRY gl_get_query_objectui64v_func)(GLuint id, GLenum pname, GLuint64 *params);
	static gl_get_query_objectui64v_func* glGetQueryObjectui64v;
	typedef void (FDYNGL_APIENTRY gl_vertex_attrib_divisor_func)(GLuint index, GLuint divisor);
	static gl_vertex_attrib_divisor_func* glVertexAttribDivisor;
	typedef void (FDYNGL_APIENTRY gl_vertex_attrib_p1ui_func)(GLuint index, GLenum type, GLboolean normalized, GLuint value);
	static gl_vertex_attrib_p1ui_func* glVertexAttribP1ui;
	typedef void (FDYNGL_APIENTRY gl_vertex_attrib_p1uiv_func)(GLuint index, GLenum type, GLboolean normalized, const GLuint *value);
	static gl_vertex_attrib_p1uiv_func* glVertexAttribP1uiv;
	typedef void (FDYNGL_APIENTRY gl_vertex_attrib_p2ui_func)(GLuint index, GLenum type, GLboolean normalized, GLuint value);
	static gl_vertex_attrib_p2ui_func* glVertexAttribP2ui;
	typedef void (FDYNGL_APIENTRY gl_vertex_attrib_p2uiv_func)(GLuint index, GLenum type, GLboolean normalized, const GLuint *value);
	static gl_vertex_attrib_p2uiv_func* glVertexAttribP2uiv;
	typedef void (FDYNGL_APIENTRY gl_vertex_attrib_p3ui_func)(GLuint index, GLenum type, GLboolean normalized, GLuint value);
	static gl_vertex_attrib_p3ui_func* glVertexAttribP3ui;
	typedef void (FDYNGL_APIENTRY gl_vertex_attrib_p3uiv_func)(GLuint index, GLenum type, GLboolean normalized, const GLuint *value);
	static gl_vertex_attrib_p3uiv_func* glVertexAttribP3uiv;
	typedef void (FDYNGL_APIENTRY gl_vertex_attrib_p4ui_func)(GLuint index, GLenum type, GLboolean normalized, GLuint value);
	static gl_vertex_attrib_p4ui_func* glVertexAttribP4ui;
	typedef void (FDYNGL_APIENTRY gl_vertex_attrib_p4uiv_func)(GLuint index, GLenum type, GLboolean normalized, const GLuint *value);
	static gl_vertex_attrib_p4uiv_func* glVertexAttribP4uiv;
	typedef void (FDYNGL_APIENTRY gl_vertex_p2ui_func)(GLenum type, GLuint value);
	static gl_vertex_p2ui_func* glVertexP2ui;
	typedef void (FDYNGL_APIENTRY gl_vertex_p2uiv_func)(GLenum type, const GLuint *value);
	static gl_vertex_p2uiv_func* glVertexP2uiv;
	typedef void (FDYNGL_APIENTRY gl_vertex_p3ui_func)(GLenum type, GLuint value);
	static gl_vertex_p3ui_func* glVertexP3ui;
	typedef void (FDYNGL_APIENTRY gl_vertex_p3uiv_func)(GLenum type, const GLuint *value);
	static gl_vertex_p3uiv_func* glVertexP3uiv;
	typedef void (FDYNGL_APIENTRY gl_vertex_p4ui_func)(GLenum type, GLuint value);
	static gl_vertex_p4ui_func* glVertexP4ui;
	typedef void (FDYNGL_APIENTRY gl_vertex_p4uiv_func)(GLenum type, const GLuint *value);
	static gl_vertex_p4uiv_func* glVertexP4uiv;
	typedef void (FDYNGL_APIENTRY gl_tex_coord_p1ui_func)(GLenum type, GLuint coords);
	static gl_tex_coord_p1ui_func* glTexCoordP1ui;
	typedef void (FDYNGL_APIENTRY gl_tex_coord_p1uiv_func)(GLenum type, const GLuint *coords);
	static gl_tex_coord_p1uiv_func* glTexCoordP1uiv;
	typedef void (FDYNGL_APIENTRY gl_tex_coord_p2ui_func)(GLenum type, GLuint coords);
	static gl_tex_coord_p2ui_func* glTexCoordP2ui;
	typedef void (FDYNGL_APIENTRY gl_tex_coord_p2uiv_func)(GLenum type, const GLuint *coords);
	static gl_tex_coord_p2uiv_func* glTexCoordP2uiv;
	typedef void (FDYNGL_APIENTRY gl_tex_coord_p3ui_func)(GLenum type, GLuint coords);
	static gl_tex_coord_p3ui_func* glTexCoordP3ui;
	typedef void (FDYNGL_APIENTRY gl_tex_coord_p3uiv_func)(GLenum type, const GLuint *coords);
	static gl_tex_coord_p3uiv_func* glTexCoordP3uiv;
	typedef void (FDYNGL_APIENTRY gl_tex_coord_p4ui_func)(GLenum type, GLuint coords);
	static gl_tex_coord_p4ui_func* glTexCoordP4ui;
	typedef void (FDYNGL_APIENTRY gl_tex_coord_p4uiv_func)(GLenum type, const GLuint *coords);
	static gl_tex_coord_p4uiv_func* glTexCoordP4uiv;
	typedef void (FDYNGL_APIENTRY gl_multi_tex_coord_p1ui_func)(GLenum texture, GLenum type, GLuint coords);
	static gl_multi_tex_coord_p1ui_func* glMultiTexCoordP1ui;
	typedef void (FDYNGL_APIENTRY gl_multi_tex_coord_p1uiv_func)(GLenum texture, GLenum type, const GLuint *coords);
	static gl_multi_tex_coord_p1uiv_func* glMultiTexCoordP1uiv;
	typedef void (FDYNGL_APIENTRY gl_multi_tex_coord_p2ui_func)(GLenum texture, GLenum type, GLuint coords);
	static gl_multi_tex_coord_p2ui_func* glMultiTexCoordP2ui;
	typedef void (FDYNGL_APIENTRY gl_multi_tex_coord_p2uiv_func)(GLenum texture, GLenum type, const GLuint *coords);
	static gl_multi_tex_coord_p2uiv_func* glMultiTexCoordP2uiv;
	typedef void (FDYNGL_APIENTRY gl_multi_tex_coord_p3ui_func)(GLenum texture, GLenum type, GLuint coords);
	static gl_multi_tex_coord_p3ui_func* glMultiTexCoordP3ui;
	typedef void (FDYNGL_APIENTRY gl_multi_tex_coord_p3uiv_func)(GLenum texture, GLenum type, const GLuint *coords);
	static gl_multi_tex_coord_p3uiv_func* glMultiTexCoordP3uiv;
	typedef void (FDYNGL_APIENTRY gl_multi_tex_coord_p4ui_func)(GLenum texture, GLenum type, GLuint coords);
	static gl_multi_tex_coord_p4ui_func* glMultiTexCoordP4ui;
	typedef void (FDYNGL_APIENTRY gl_multi_tex_coord_p4uiv_func)(GLenum texture, GLenum type, const GLuint *coords);
	static gl_multi_tex_coord_p4uiv_func* glMultiTexCoordP4uiv;
	typedef void (FDYNGL_APIENTRY gl_normal_p3ui_func)(GLenum type, GLuint coords);
	static gl_normal_p3ui_func* glNormalP3ui;
	typedef void (FDYNGL_APIENTRY gl_normal_p3uiv_func)(GLenum type, const GLuint *coords);
	static gl_normal_p3uiv_func* glNormalP3uiv;
	typedef void (FDYNGL_APIENTRY gl_color_p3ui_func)(GLenum type, GLuint color);
	static gl_color_p3ui_func* glColorP3ui;
	typedef void (FDYNGL_APIENTRY gl_color_p3uiv_func)(GLenum type, const GLuint *color);
	static gl_color_p3uiv_func* glColorP3uiv;
	typedef void (FDYNGL_APIENTRY gl_color_p4ui_func)(GLenum type, GLuint color);
	static gl_color_p4ui_func* glColorP4ui;
	typedef void (FDYNGL_APIENTRY gl_color_p4uiv_func)(GLenum type, const GLuint *color);
	static gl_color_p4uiv_func* glColorP4uiv;
	typedef void (FDYNGL_APIENTRY gl_secondary_color_p3ui_func)(GLenum type, GLuint color);
	static gl_secondary_color_p3ui_func* glSecondaryColorP3ui;
	typedef void (FDYNGL_APIENTRY gl_secondary_color_p3uiv_func)(GLenum type, const GLuint *color);
	static gl_secondary_color_p3uiv_func* glSecondaryColorP3uiv;
#	endif // GL_VERSION_3_3
#	ifndef GL_VERSION_4_0
#		define GL_VERSION_4_0 1
	static bool isGL_VERSION_4_0;
	static const uint32_t GL_SAMPLE_SHADING = 0x8C36;
	static const uint32_t GL_MIN_SAMPLE_SHADING_VALUE = 0x8C37;
	static const uint32_t GL_MIN_PROGRAM_TEXTURE_GATHER_OFFSET = 0x8E5E;
	static const uint32_t GL_MAX_PROGRAM_TEXTURE_GATHER_OFFSET = 0x8E5F;
	static const uint32_t GL_TEXTURE_CUBE_MAP_ARRAY = 0x9009;
	static const uint32_t GL_TEXTURE_BINDING_CUBE_MAP_ARRAY = 0x900A;
	static const uint32_t GL_PROXY_TEXTURE_CUBE_MAP_ARRAY = 0x900B;
	static const uint32_t GL_SAMPLER_CUBE_MAP_ARRAY = 0x900C;
	static const uint32_t GL_SAMPLER_CUBE_MAP_ARRAY_SHADOW = 0x900D;
	static const uint32_t GL_INT_SAMPLER_CUBE_MAP_ARRAY = 0x900E;
	static const uint32_t GL_UNSIGNED_INT_SAMPLER_CUBE_MAP_ARRAY = 0x900F;
	static const uint32_t GL_DRAW_INDIRECT_BUFFER = 0x8F3F;
	static const uint32_t GL_DRAW_INDIRECT_BUFFER_BINDING = 0x8F43;
	static const uint32_t GL_GEOMETRY_SHADER_INVOCATIONS = 0x887F;
	static const uint32_t GL_MAX_GEOMETRY_SHADER_INVOCATIONS = 0x8E5A;
	static const uint32_t GL_MIN_FRAGMENT_INTERPOLATION_OFFSET = 0x8E5B;
	static const uint32_t GL_MAX_FRAGMENT_INTERPOLATION_OFFSET = 0x8E5C;
	static const uint32_t GL_FRAGMENT_INTERPOLATION_OFFSET_BITS = 0x8E5D;
	static const uint32_t GL_MAX_VERTEX_STREAMS = 0x8E71;
	static const uint32_t GL_DOUBLE_VEC2 = 0x8FFC;
	static const uint32_t GL_DOUBLE_VEC3 = 0x8FFD;
	static const uint32_t GL_DOUBLE_VEC4 = 0x8FFE;
	static const uint32_t GL_DOUBLE_MAT2 = 0x8F46;
	static const uint32_t GL_DOUBLE_MAT3 = 0x8F47;
	static const uint32_t GL_DOUBLE_MAT4 = 0x8F48;
	static const uint32_t GL_DOUBLE_MAT2x3 = 0x8F49;
	static const uint32_t GL_DOUBLE_MAT2x4 = 0x8F4A;
	static const uint32_t GL_DOUBLE_MAT3x2 = 0x8F4B;
	static const uint32_t GL_DOUBLE_MAT3x4 = 0x8F4C;
	static const uint32_t GL_DOUBLE_MAT4x2 = 0x8F4D;
	static const uint32_t GL_DOUBLE_MAT4x3 = 0x8F4E;
	static const uint32_t GL_ACTIVE_SUBROUTINES = 0x8DE5;
	static const uint32_t GL_ACTIVE_SUBROUTINE_UNIFORMS = 0x8DE6;
	static const uint32_t GL_ACTIVE_SUBROUTINE_UNIFORM_LOCATIONS = 0x8E47;
	static const uint32_t GL_ACTIVE_SUBROUTINE_MAX_LENGTH = 0x8E48;
	static const uint32_t GL_ACTIVE_SUBROUTINE_UNIFORM_MAX_LENGTH = 0x8E49;
	static const uint32_t GL_MAX_SUBROUTINES = 0x8DE7;
	static const uint32_t GL_MAX_SUBROUTINE_UNIFORM_LOCATIONS = 0x8DE8;
	static const uint32_t GL_NUM_COMPATIBLE_SUBROUTINES = 0x8E4A;
	static const uint32_t GL_COMPATIBLE_SUBROUTINES = 0x8E4B;
	static const uint32_t GL_PATCHES = 0x000E;
	static const uint32_t GL_PATCH_VERTICES = 0x8E72;
	static const uint32_t GL_PATCH_DEFAULT_INNER_LEVEL = 0x8E73;
	static const uint32_t GL_PATCH_DEFAULT_OUTER_LEVEL = 0x8E74;
	static const uint32_t GL_TESS_CONTROL_OUTPUT_VERTICES = 0x8E75;
	static const uint32_t GL_TESS_GEN_MODE = 0x8E76;
	static const uint32_t GL_TESS_GEN_SPACING = 0x8E77;
	static const uint32_t GL_TESS_GEN_VERTEX_ORDER = 0x8E78;
	static const uint32_t GL_TESS_GEN_POINT_MODE = 0x8E79;
	static const uint32_t GL_ISOLINES = 0x8E7A;
	static const uint32_t GL_FRACTIONAL_ODD = 0x8E7B;
	static const uint32_t GL_FRACTIONAL_EVEN = 0x8E7C;
	static const uint32_t GL_MAX_PATCH_VERTICES = 0x8E7D;
	static const uint32_t GL_MAX_TESS_GEN_LEVEL = 0x8E7E;
	static const uint32_t GL_MAX_TESS_CONTROL_UNIFORM_COMPONENTS = 0x8E7F;
	static const uint32_t GL_MAX_TESS_EVALUATION_UNIFORM_COMPONENTS = 0x8E80;
	static const uint32_t GL_MAX_TESS_CONTROL_TEXTURE_IMAGE_UNITS = 0x8E81;
	static const uint32_t GL_MAX_TESS_EVALUATION_TEXTURE_IMAGE_UNITS = 0x8E82;
	static const uint32_t GL_MAX_TESS_CONTROL_OUTPUT_COMPONENTS = 0x8E83;
	static const uint32_t GL_MAX_TESS_PATCH_COMPONENTS = 0x8E84;
	static const uint32_t GL_MAX_TESS_CONTROL_TOTAL_OUTPUT_COMPONENTS = 0x8E85;
	static const uint32_t GL_MAX_TESS_EVALUATION_OUTPUT_COMPONENTS = 0x8E86;
	static const uint32_t GL_MAX_TESS_CONTROL_UNIFORM_BLOCKS = 0x8E89;
	static const uint32_t GL_MAX_TESS_EVALUATION_UNIFORM_BLOCKS = 0x8E8A;
	static const uint32_t GL_MAX_TESS_CONTROL_INPUT_COMPONENTS = 0x886C;
	static const uint32_t GL_MAX_TESS_EVALUATION_INPUT_COMPONENTS = 0x886D;
	static const uint32_t GL_MAX_COMBINED_TESS_CONTROL_UNIFORM_COMPONENTS = 0x8E1E;
	static const uint32_t GL_MAX_COMBINED_TESS_EVALUATION_UNIFORM_COMPONENTS = 0x8E1F;
	static const uint32_t GL_UNIFORM_BLOCK_REFERENCED_BY_TESS_CONTROL_SHADER = 0x84F0;
	static const uint32_t GL_UNIFORM_BLOCK_REFERENCED_BY_TESS_EVALUATION_SHADER = 0x84F1;
	static const uint32_t GL_TESS_EVALUATION_SHADER = 0x8E87;
	static const uint32_t GL_TESS_CONTROL_SHADER = 0x8E88;
	static const uint32_t GL_TRANSFORM_FEEDBACK = 0x8E22;
	static const uint32_t GL_TRANSFORM_FEEDBACK_BUFFER_PAUSED = 0x8E23;
	static const uint32_t GL_TRANSFORM_FEEDBACK_BUFFER_ACTIVE = 0x8E24;
	static const uint32_t GL_TRANSFORM_FEEDBACK_BINDING = 0x8E25;
	static const uint32_t GL_MAX_TRANSFORM_FEEDBACK_BUFFERS = 0x8E70;
	typedef void (FDYNGL_APIENTRY gl_min_sample_shading_func)(GLfloat value);
	static gl_min_sample_shading_func* glMinSampleShading;
	typedef void (FDYNGL_APIENTRY gl_blend_equationi_func)(GLuint buf, GLenum mode);
	static gl_blend_equationi_func* glBlendEquationi;
	typedef void (FDYNGL_APIENTRY gl_blend_equation_separatei_func)(GLuint buf, GLenum modeRGB, GLenum modeAlpha);
	static gl_blend_equation_separatei_func* glBlendEquationSeparatei;
	typedef void (FDYNGL_APIENTRY gl_blend_funci_func)(GLuint buf, GLenum src, GLenum dst);
	static gl_blend_funci_func* glBlendFunci;
	typedef void (FDYNGL_APIENTRY gl_blend_func_separatei_func)(GLuint buf, GLenum srcRGB, GLenum dstRGB, GLenum srcAlpha, GLenum dstAlpha);
	static gl_blend_func_separatei_func* glBlendFuncSeparatei;
	typedef void (FDYNGL_APIENTRY gl_draw_arrays_indirect_func)(GLenum mode, const void *indirect);
	static gl_draw_arrays_indirect_func* glDrawArraysIndirect;
	typedef void (FDYNGL_APIENTRY gl_draw_elements_indirect_func)(GLenum mode, GLenum type, const void *indirect);
	static gl_draw_elements_indirect_func* glDrawElementsIndirect;
	typedef void (FDYNGL_APIENTRY gl_uniform1d_func)(GLint location, GLdouble x);
	static gl_uniform1d_func* glUniform1d;
	typedef void (FDYNGL_APIENTRY gl_uniform2d_func)(GLint location, GLdouble x, GLdouble y);
	static gl_uniform2d_func* glUniform2d;
	typedef void (FDYNGL_APIENTRY gl_uniform3d_func)(GLint location, GLdouble x, GLdouble y, GLdouble z);
	static gl_uniform3d_func* glUniform3d;
	typedef void (FDYNGL_APIENTRY gl_uniform4d_func)(GLint location, GLdouble x, GLdouble y, GLdouble z, GLdouble w);
	static gl_uniform4d_func* glUniform4d;
	typedef void (FDYNGL_APIENTRY gl_uniform1dv_func)(GLint location, GLsizei count, const GLdouble *value);
	static gl_uniform1dv_func* glUniform1dv;
	typedef void (FDYNGL_APIENTRY gl_uniform2dv_func)(GLint location, GLsizei count, const GLdouble *value);
	static gl_uniform2dv_func* glUniform2dv;
	typedef void (FDYNGL_APIENTRY gl_uniform3dv_func)(GLint location, GLsizei count, const GLdouble *value);
	static gl_uniform3dv_func* glUniform3dv;
	typedef void (FDYNGL_APIENTRY gl_uniform4dv_func)(GLint location, GLsizei count, const GLdouble *value);
	static gl_uniform4dv_func* glUniform4dv;
	typedef void (FDYNGL_APIENTRY gl_uniform_matrix2dv_func)(GLint location, GLsizei count, GLboolean transpose, const GLdouble *value);
	static gl_uniform_matrix2dv_func* glUniformMatrix2dv;
	typedef void (FDYNGL_APIENTRY gl_uniform_matrix3dv_func)(GLint location, GLsizei count, GLboolean transpose, const GLdouble *value);
	static gl_uniform_matrix3dv_func* glUniformMatrix3dv;
	typedef void (FDYNGL_APIENTRY gl_uniform_matrix4dv_func)(GLint location, GLsizei count, GLboolean transpose, const GLdouble *value);
	static gl_uniform_matrix4dv_func* glUniformMatrix4dv;
	typedef void (FDYNGL_APIENTRY gl_uniform_matrix2x3dv_func)(GLint location, GLsizei count, GLboolean transpose, const GLdouble *value);
	static gl_uniform_matrix2x3dv_func* glUniformMatrix2x3dv;
	typedef void (FDYNGL_APIENTRY gl_uniform_matrix2x4dv_func)(GLint location, GLsizei count, GLboolean transpose, const GLdouble *value);
	static gl_uniform_matrix2x4dv_func* glUniformMatrix2x4dv;
	typedef void (FDYNGL_APIENTRY gl_uniform_matrix3x2dv_func)(GLint location, GLsizei count, GLboolean transpose, const GLdouble *value);
	static gl_uniform_matrix3x2dv_func* glUniformMatrix3x2dv;
	typedef void (FDYNGL_APIENTRY gl_uniform_matrix3x4dv_func)(GLint location, GLsizei count, GLboolean transpose, const GLdouble *value);
	static gl_uniform_matrix3x4dv_func* glUniformMatrix3x4dv;
	typedef void (FDYNGL_APIENTRY gl_uniform_matrix4x2dv_func)(GLint location, GLsizei count, GLboolean transpose, const GLdouble *value);
	static gl_uniform_matrix4x2dv_func* glUniformMatrix4x2dv;
	typedef void (FDYNGL_APIENTRY gl_uniform_matrix4x3dv_func)(GLint location, GLsizei count, GLboolean transpose, const GLdouble *value);
	static gl_uniform_matrix4x3dv_func* glUniformMatrix4x3dv;
	typedef void (FDYNGL_APIENTRY gl_get_uniformdv_func)(GLuint program, GLint location, GLdouble *params);
	static gl_get_uniformdv_func* glGetUniformdv;
	typedef GLint(FDYNGL_APIENTRY gl_get_subroutine_uniform_location_func)(GLuint program, GLenum shadertype, const GLchar *name);
	static gl_get_subroutine_uniform_location_func* glGetSubroutineUniformLocation;
	typedef GLuint(FDYNGL_APIENTRY gl_get_subroutine_index_func)(GLuint program, GLenum shadertype, const GLchar *name);
	static gl_get_subroutine_index_func* glGetSubroutineIndex;
	typedef void (FDYNGL_APIENTRY gl_get_active_subroutine_uniformiv_func)(GLuint program, GLenum shadertype, GLuint index, GLenum pname, GLint *values);
	static gl_get_active_subroutine_uniformiv_func* glGetActiveSubroutineUniformiv;
	typedef void (FDYNGL_APIENTRY gl_get_active_subroutine_uniform_name_func)(GLuint program, GLenum shadertype, GLuint index, GLsizei bufsize, GLsizei *length, GLchar *name);
	static gl_get_active_subroutine_uniform_name_func* glGetActiveSubroutineUniformName;
	typedef void (FDYNGL_APIENTRY gl_get_active_subroutine_name_func)(GLuint program, GLenum shadertype, GLuint index, GLsizei bufsize, GLsizei *length, GLchar *name);
	static gl_get_active_subroutine_name_func* glGetActiveSubroutineName;
	typedef void (FDYNGL_APIENTRY gl_uniform_subroutinesuiv_func)(GLenum shadertype, GLsizei count, const GLuint *indices);
	static gl_uniform_subroutinesuiv_func* glUniformSubroutinesuiv;
	typedef void (FDYNGL_APIENTRY gl_get_uniform_subroutineuiv_func)(GLenum shadertype, GLint location, GLuint *params);
	static gl_get_uniform_subroutineuiv_func* glGetUniformSubroutineuiv;
	typedef void (FDYNGL_APIENTRY gl_get_program_stageiv_func)(GLuint program, GLenum shadertype, GLenum pname, GLint *values);
	static gl_get_program_stageiv_func* glGetProgramStageiv;
	typedef void (FDYNGL_APIENTRY gl_patch_parameteri_func)(GLenum pname, GLint value);
	static gl_patch_parameteri_func* glPatchParameteri;
	typedef void (FDYNGL_APIENTRY gl_patch_parameterfv_func)(GLenum pname, const GLfloat *values);
	static gl_patch_parameterfv_func* glPatchParameterfv;
	typedef void (FDYNGL_APIENTRY gl_bind_transform_feedback_func)(GLenum target, GLuint id);
	static gl_bind_transform_feedback_func* glBindTransformFeedback;
	typedef void (FDYNGL_APIENTRY gl_delete_transform_feedbacks_func)(GLsizei n, const GLuint *ids);
	static gl_delete_transform_feedbacks_func* glDeleteTransformFeedbacks;
	typedef void (FDYNGL_APIENTRY gl_gen_transform_feedbacks_func)(GLsizei n, GLuint *ids);
	static gl_gen_transform_feedbacks_func* glGenTransformFeedbacks;
	typedef GLboolean(FDYNGL_APIENTRY gl_is_transform_feedback_func)(GLuint id);
	static gl_is_transform_feedback_func* glIsTransformFeedback;
	typedef void (FDYNGL_APIENTRY gl_pause_transform_feedback_func)(void);
	static gl_pause_transform_feedback_func* glPauseTransformFeedback;
	typedef void (FDYNGL_APIENTRY gl_resume_transform_feedback_func)(void);
	static gl_resume_transform_feedback_func* glResumeTransformFeedback;
	typedef void (FDYNGL_APIENTRY gl_draw_transform_feedback_func)(GLenum mode, GLuint id);
	static gl_draw_transform_feedback_func* glDrawTransformFeedback;
	typedef void (FDYNGL_APIENTRY gl_draw_transform_feedback_stream_func)(GLenum mode, GLuint id, GLuint stream);
	static gl_draw_transform_feedback_stream_func* glDrawTransformFeedbackStream;
	typedef void (FDYNGL_APIENTRY gl_begin_query_indexed_func)(GLenum target, GLuint index, GLuint id);
	static gl_begin_query_indexed_func* glBeginQueryIndexed;
	typedef void (FDYNGL_APIENTRY gl_end_query_indexed_func)(GLenum target, GLuint index);
	static gl_end_query_indexed_func* glEndQueryIndexed;
	typedef void (FDYNGL_APIENTRY gl_get_query_indexediv_func)(GLenum target, GLuint index, GLenum pname, GLint *params);
	static gl_get_query_indexediv_func* glGetQueryIndexediv;
#	endif // GL_VERSION_4_0
#	ifndef GL_VERSION_4_1
#		define GL_VERSION_4_1 1
	static bool isGL_VERSION_4_1;
	static const uint32_t GL_FIXED = 0x140C;
	static const uint32_t GL_IMPLEMENTATION_COLOR_READ_TYPE = 0x8B9A;
	static const uint32_t GL_IMPLEMENTATION_COLOR_READ_FORMAT = 0x8B9B;
	static const uint32_t GL_LOW_FLOAT = 0x8DF0;
	static const uint32_t GL_MEDIUM_FLOAT = 0x8DF1;
	static const uint32_t GL_HIGH_FLOAT = 0x8DF2;
	static const uint32_t GL_LOW_INT = 0x8DF3;
	static const uint32_t GL_MEDIUM_INT = 0x8DF4;
	static const uint32_t GL_HIGH_INT = 0x8DF5;
	static const uint32_t GL_SHADER_COMPILER = 0x8DFA;
	static const uint32_t GL_SHADER_BINARY_FORMATS = 0x8DF8;
	static const uint32_t GL_NUM_SHADER_BINARY_FORMATS = 0x8DF9;
	static const uint32_t GL_MAX_VERTEX_UNIFORM_VECTORS = 0x8DFB;
	static const uint32_t GL_MAX_VARYING_VECTORS = 0x8DFC;
	static const uint32_t GL_MAX_FRAGMENT_UNIFORM_VECTORS = 0x8DFD;
	static const uint32_t GL_RGB565 = 0x8D62;
	static const uint32_t GL_PROGRAM_BINARY_RETRIEVABLE_HINT = 0x8257;
	static const uint32_t GL_PROGRAM_BINARY_LENGTH = 0x8741;
	static const uint32_t GL_NUM_PROGRAM_BINARY_FORMATS = 0x87FE;
	static const uint32_t GL_PROGRAM_BINARY_FORMATS = 0x87FF;
	static const uint32_t GL_VERTEX_SHADER_BIT = 0x00000001;
	static const uint32_t GL_FRAGMENT_SHADER_BIT = 0x00000002;
	static const uint32_t GL_GEOMETRY_SHADER_BIT = 0x00000004;
	static const uint32_t GL_TESS_CONTROL_SHADER_BIT = 0x00000008;
	static const uint32_t GL_TESS_EVALUATION_SHADER_BIT = 0x00000010;
	static const uint32_t GL_ALL_SHADER_BITS = 0xFFFFFFFF;
	static const uint32_t GL_PROGRAM_SEPARABLE = 0x8258;
	static const uint32_t GL_ACTIVE_PROGRAM = 0x8259;
	static const uint32_t GL_PROGRAM_PIPELINE_BINDING = 0x825A;
	static const uint32_t GL_MAX_VIEWPORTS = 0x825B;
	static const uint32_t GL_VIEWPORT_SUBPIXEL_BITS = 0x825C;
	static const uint32_t GL_VIEWPORT_BOUNDS_RANGE = 0x825D;
	static const uint32_t GL_LAYER_PROVOKING_VERTEX = 0x825E;
	static const uint32_t GL_VIEWPORT_INDEX_PROVOKING_VERTEX = 0x825F;
	static const uint32_t GL_UNDEFINED_VERTEX = 0x8260;
	typedef void (FDYNGL_APIENTRY gl_release_shader_compiler_func)(void);
	static gl_release_shader_compiler_func* glReleaseShaderCompiler;
	typedef void (FDYNGL_APIENTRY gl_shader_binary_func)(GLsizei count, const GLuint *shaders, GLenum binaryformat, const void *binary, GLsizei length);
	static gl_shader_binary_func* glShaderBinary;
	typedef void (FDYNGL_APIENTRY gl_get_shader_precision_format_func)(GLenum shadertype, GLenum precisiontype, GLint *range, GLint *precision);
	static gl_get_shader_precision_format_func* glGetShaderPrecisionFormat;
	typedef void (FDYNGL_APIENTRY gl_depth_rangef_func)(GLfloat n, GLfloat f);
	static gl_depth_rangef_func* glDepthRangef;
	typedef void (FDYNGL_APIENTRY gl_clear_depthf_func)(GLfloat d);
	static gl_clear_depthf_func* glClearDepthf;
	typedef void (FDYNGL_APIENTRY gl_get_program_binary_func)(GLuint program, GLsizei bufSize, GLsizei *length, GLenum *binaryFormat, void *binary);
	static gl_get_program_binary_func* glGetProgramBinary;
	typedef void (FDYNGL_APIENTRY gl_program_binary_func)(GLuint program, GLenum binaryFormat, const void *binary, GLsizei length);
	static gl_program_binary_func* glProgramBinary;
	typedef void (FDYNGL_APIENTRY gl_program_parameteri_func)(GLuint program, GLenum pname, GLint value);
	static gl_program_parameteri_func* glProgramParameteri;
	typedef void (FDYNGL_APIENTRY gl_use_program_stages_func)(GLuint pipeline, GLbitfield stages, GLuint program);
	static gl_use_program_stages_func* glUseProgramStages;
	typedef void (FDYNGL_APIENTRY gl_active_shader_program_func)(GLuint pipeline, GLuint program);
	static gl_active_shader_program_func* glActiveShaderProgram;
	typedef GLuint(FDYNGL_APIENTRY gl_create_shader_programv_func)(GLenum type, GLsizei count, const GLchar *const*strings);
	static gl_create_shader_programv_func* glCreateShaderProgramv;
	typedef void (FDYNGL_APIENTRY gl_bind_program_pipeline_func)(GLuint pipeline);
	static gl_bind_program_pipeline_func* glBindProgramPipeline;
	typedef void (FDYNGL_APIENTRY gl_delete_program_pipelines_func)(GLsizei n, const GLuint *pipelines);
	static gl_delete_program_pipelines_func* glDeleteProgramPipelines;
	typedef void (FDYNGL_APIENTRY gl_gen_program_pipelines_func)(GLsizei n, GLuint *pipelines);
	static gl_gen_program_pipelines_func* glGenProgramPipelines;
	typedef GLboolean(FDYNGL_APIENTRY gl_is_program_pipeline_func)(GLuint pipeline);
	static gl_is_program_pipeline_func* glIsProgramPipeline;
	typedef void (FDYNGL_APIENTRY gl_get_program_pipelineiv_func)(GLuint pipeline, GLenum pname, GLint *params);
	static gl_get_program_pipelineiv_func* glGetProgramPipelineiv;
	typedef void (FDYNGL_APIENTRY gl_program_uniform1i_func)(GLuint program, GLint location, GLint v0);
	static gl_program_uniform1i_func* glProgramUniform1i;
	typedef void (FDYNGL_APIENTRY gl_program_uniform1iv_func)(GLuint program, GLint location, GLsizei count, const GLint *value);
	static gl_program_uniform1iv_func* glProgramUniform1iv;
	typedef void (FDYNGL_APIENTRY gl_program_uniform1f_func)(GLuint program, GLint location, GLfloat v0);
	static gl_program_uniform1f_func* glProgramUniform1f;
	typedef void (FDYNGL_APIENTRY gl_program_uniform1fv_func)(GLuint program, GLint location, GLsizei count, const GLfloat *value);
	static gl_program_uniform1fv_func* glProgramUniform1fv;
	typedef void (FDYNGL_APIENTRY gl_program_uniform1d_func)(GLuint program, GLint location, GLdouble v0);
	static gl_program_uniform1d_func* glProgramUniform1d;
	typedef void (FDYNGL_APIENTRY gl_program_uniform1dv_func)(GLuint program, GLint location, GLsizei count, const GLdouble *value);
	static gl_program_uniform1dv_func* glProgramUniform1dv;
	typedef void (FDYNGL_APIENTRY gl_program_uniform1ui_func)(GLuint program, GLint location, GLuint v0);
	static gl_program_uniform1ui_func* glProgramUniform1ui;
	typedef void (FDYNGL_APIENTRY gl_program_uniform1uiv_func)(GLuint program, GLint location, GLsizei count, const GLuint *value);
	static gl_program_uniform1uiv_func* glProgramUniform1uiv;
	typedef void (FDYNGL_APIENTRY gl_program_uniform2i_func)(GLuint program, GLint location, GLint v0, GLint v1);
	static gl_program_uniform2i_func* glProgramUniform2i;
	typedef void (FDYNGL_APIENTRY gl_program_uniform2iv_func)(GLuint program, GLint location, GLsizei count, const GLint *value);
	static gl_program_uniform2iv_func* glProgramUniform2iv;
	typedef void (FDYNGL_APIENTRY gl_program_uniform2f_func)(GLuint program, GLint location, GLfloat v0, GLfloat v1);
	static gl_program_uniform2f_func* glProgramUniform2f;
	typedef void (FDYNGL_APIENTRY gl_program_uniform2fv_func)(GLuint program, GLint location, GLsizei count, const GLfloat *value);
	static gl_program_uniform2fv_func* glProgramUniform2fv;
	typedef void (FDYNGL_APIENTRY gl_program_uniform2d_func)(GLuint program, GLint location, GLdouble v0, GLdouble v1);
	static gl_program_uniform2d_func* glProgramUniform2d;
	typedef void (FDYNGL_APIENTRY gl_program_uniform2dv_func)(GLuint program, GLint location, GLsizei count, const GLdouble *value);
	static gl_program_uniform2dv_func* glProgramUniform2dv;
	typedef void (FDYNGL_APIENTRY gl_program_uniform2ui_func)(GLuint program, GLint location, GLuint v0, GLuint v1);
	static gl_program_uniform2ui_func* glProgramUniform2ui;
	typedef void (FDYNGL_APIENTRY gl_program_uniform2uiv_func)(GLuint program, GLint location, GLsizei count, const GLuint *value);
	static gl_program_uniform2uiv_func* glProgramUniform2uiv;
	typedef void (FDYNGL_APIENTRY gl_program_uniform3i_func)(GLuint program, GLint location, GLint v0, GLint v1, GLint v2);
	static gl_program_uniform3i_func* glProgramUniform3i;
	typedef void (FDYNGL_APIENTRY gl_program_uniform3iv_func)(GLuint program, GLint location, GLsizei count, const GLint *value);
	static gl_program_uniform3iv_func* glProgramUniform3iv;
	typedef void (FDYNGL_APIENTRY gl_program_uniform3f_func)(GLuint program, GLint location, GLfloat v0, GLfloat v1, GLfloat v2);
	static gl_program_uniform3f_func* glProgramUniform3f;
	typedef void (FDYNGL_APIENTRY gl_program_uniform3fv_func)(GLuint program, GLint location, GLsizei count, const GLfloat *value);
	static gl_program_uniform3fv_func* glProgramUniform3fv;
	typedef void (FDYNGL_APIENTRY gl_program_uniform3d_func)(GLuint program, GLint location, GLdouble v0, GLdouble v1, GLdouble v2);
	static gl_program_uniform3d_func* glProgramUniform3d;
	typedef void (FDYNGL_APIENTRY gl_program_uniform3dv_func)(GLuint program, GLint location, GLsizei count, const GLdouble *value);
	static gl_program_uniform3dv_func* glProgramUniform3dv;
	typedef void (FDYNGL_APIENTRY gl_program_uniform3ui_func)(GLuint program, GLint location, GLuint v0, GLuint v1, GLuint v2);
	static gl_program_uniform3ui_func* glProgramUniform3ui;
	typedef void (FDYNGL_APIENTRY gl_program_uniform3uiv_func)(GLuint program, GLint location, GLsizei count, const GLuint *value);
	static gl_program_uniform3uiv_func* glProgramUniform3uiv;
	typedef void (FDYNGL_APIENTRY gl_program_uniform4i_func)(GLuint program, GLint location, GLint v0, GLint v1, GLint v2, GLint v3);
	static gl_program_uniform4i_func* glProgramUniform4i;
	typedef void (FDYNGL_APIENTRY gl_program_uniform4iv_func)(GLuint program, GLint location, GLsizei count, const GLint *value);
	static gl_program_uniform4iv_func* glProgramUniform4iv;
	typedef void (FDYNGL_APIENTRY gl_program_uniform4f_func)(GLuint program, GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3);
	static gl_program_uniform4f_func* glProgramUniform4f;
	typedef void (FDYNGL_APIENTRY gl_program_uniform4fv_func)(GLuint program, GLint location, GLsizei count, const GLfloat *value);
	static gl_program_uniform4fv_func* glProgramUniform4fv;
	typedef void (FDYNGL_APIENTRY gl_program_uniform4d_func)(GLuint program, GLint location, GLdouble v0, GLdouble v1, GLdouble v2, GLdouble v3);
	static gl_program_uniform4d_func* glProgramUniform4d;
	typedef void (FDYNGL_APIENTRY gl_program_uniform4dv_func)(GLuint program, GLint location, GLsizei count, const GLdouble *value);
	static gl_program_uniform4dv_func* glProgramUniform4dv;
	typedef void (FDYNGL_APIENTRY gl_program_uniform4ui_func)(GLuint program, GLint location, GLuint v0, GLuint v1, GLuint v2, GLuint v3);
	static gl_program_uniform4ui_func* glProgramUniform4ui;
	typedef void (FDYNGL_APIENTRY gl_program_uniform4uiv_func)(GLuint program, GLint location, GLsizei count, const GLuint *value);
	static gl_program_uniform4uiv_func* glProgramUniform4uiv;
	typedef void (FDYNGL_APIENTRY gl_program_uniform_matrix2fv_func)(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
	static gl_program_uniform_matrix2fv_func* glProgramUniformMatrix2fv;
	typedef void (FDYNGL_APIENTRY gl_program_uniform_matrix3fv_func)(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
	static gl_program_uniform_matrix3fv_func* glProgramUniformMatrix3fv;
	typedef void (FDYNGL_APIENTRY gl_program_uniform_matrix4fv_func)(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
	static gl_program_uniform_matrix4fv_func* glProgramUniformMatrix4fv;
	typedef void (FDYNGL_APIENTRY gl_program_uniform_matrix2dv_func)(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble *value);
	static gl_program_uniform_matrix2dv_func* glProgramUniformMatrix2dv;
	typedef void (FDYNGL_APIENTRY gl_program_uniform_matrix3dv_func)(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble *value);
	static gl_program_uniform_matrix3dv_func* glProgramUniformMatrix3dv;
	typedef void (FDYNGL_APIENTRY gl_program_uniform_matrix4dv_func)(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble *value);
	static gl_program_uniform_matrix4dv_func* glProgramUniformMatrix4dv;
	typedef void (FDYNGL_APIENTRY gl_program_uniform_matrix2x3fv_func)(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
	static gl_program_uniform_matrix2x3fv_func* glProgramUniformMatrix2x3fv;
	typedef void (FDYNGL_APIENTRY gl_program_uniform_matrix3x2fv_func)(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
	static gl_program_uniform_matrix3x2fv_func* glProgramUniformMatrix3x2fv;
	typedef void (FDYNGL_APIENTRY gl_program_uniform_matrix2x4fv_func)(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
	static gl_program_uniform_matrix2x4fv_func* glProgramUniformMatrix2x4fv;
	typedef void (FDYNGL_APIENTRY gl_program_uniform_matrix4x2fv_func)(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
	static gl_program_uniform_matrix4x2fv_func* glProgramUniformMatrix4x2fv;
	typedef void (FDYNGL_APIENTRY gl_program_uniform_matrix3x4fv_func)(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
	static gl_program_uniform_matrix3x4fv_func* glProgramUniformMatrix3x4fv;
	typedef void (FDYNGL_APIENTRY gl_program_uniform_matrix4x3fv_func)(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
	static gl_program_uniform_matrix4x3fv_func* glProgramUniformMatrix4x3fv;
	typedef void (FDYNGL_APIENTRY gl_program_uniform_matrix2x3dv_func)(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble *value);
	static gl_program_uniform_matrix2x3dv_func* glProgramUniformMatrix2x3dv;
	typedef void (FDYNGL_APIENTRY gl_program_uniform_matrix3x2dv_func)(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble *value);
	static gl_program_uniform_matrix3x2dv_func* glProgramUniformMatrix3x2dv;
	typedef void (FDYNGL_APIENTRY gl_program_uniform_matrix2x4dv_func)(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble *value);
	static gl_program_uniform_matrix2x4dv_func* glProgramUniformMatrix2x4dv;
	typedef void (FDYNGL_APIENTRY gl_program_uniform_matrix4x2dv_func)(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble *value);
	static gl_program_uniform_matrix4x2dv_func* glProgramUniformMatrix4x2dv;
	typedef void (FDYNGL_APIENTRY gl_program_uniform_matrix3x4dv_func)(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble *value);
	static gl_program_uniform_matrix3x4dv_func* glProgramUniformMatrix3x4dv;
	typedef void (FDYNGL_APIENTRY gl_program_uniform_matrix4x3dv_func)(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble *value);
	static gl_program_uniform_matrix4x3dv_func* glProgramUniformMatrix4x3dv;
	typedef void (FDYNGL_APIENTRY gl_validate_program_pipeline_func)(GLuint pipeline);
	static gl_validate_program_pipeline_func* glValidateProgramPipeline;
	typedef void (FDYNGL_APIENTRY gl_get_program_pipeline_info_log_func)(GLuint pipeline, GLsizei bufSize, GLsizei *length, GLchar *infoLog);
	static gl_get_program_pipeline_info_log_func* glGetProgramPipelineInfoLog;
	typedef void (FDYNGL_APIENTRY gl_vertex_attrib_l1d_func)(GLuint index, GLdouble x);
	static gl_vertex_attrib_l1d_func* glVertexAttribL1d;
	typedef void (FDYNGL_APIENTRY gl_vertex_attrib_l2d_func)(GLuint index, GLdouble x, GLdouble y);
	static gl_vertex_attrib_l2d_func* glVertexAttribL2d;
	typedef void (FDYNGL_APIENTRY gl_vertex_attrib_l3d_func)(GLuint index, GLdouble x, GLdouble y, GLdouble z);
	static gl_vertex_attrib_l3d_func* glVertexAttribL3d;
	typedef void (FDYNGL_APIENTRY gl_vertex_attrib_l4d_func)(GLuint index, GLdouble x, GLdouble y, GLdouble z, GLdouble w);
	static gl_vertex_attrib_l4d_func* glVertexAttribL4d;
	typedef void (FDYNGL_APIENTRY gl_vertex_attrib_l1dv_func)(GLuint index, const GLdouble *v);
	static gl_vertex_attrib_l1dv_func* glVertexAttribL1dv;
	typedef void (FDYNGL_APIENTRY gl_vertex_attrib_l2dv_func)(GLuint index, const GLdouble *v);
	static gl_vertex_attrib_l2dv_func* glVertexAttribL2dv;
	typedef void (FDYNGL_APIENTRY gl_vertex_attrib_l3dv_func)(GLuint index, const GLdouble *v);
	static gl_vertex_attrib_l3dv_func* glVertexAttribL3dv;
	typedef void (FDYNGL_APIENTRY gl_vertex_attrib_l4dv_func)(GLuint index, const GLdouble *v);
	static gl_vertex_attrib_l4dv_func* glVertexAttribL4dv;
	typedef void (FDYNGL_APIENTRY gl_vertex_attrib_l_pointer_func)(GLuint index, GLint size, GLenum type, GLsizei stride, const void *pointer);
	static gl_vertex_attrib_l_pointer_func* glVertexAttribLPointer;
	typedef void (FDYNGL_APIENTRY gl_get_vertex_attrib_ldv_func)(GLuint index, GLenum pname, GLdouble *params);
	static gl_get_vertex_attrib_ldv_func* glGetVertexAttribLdv;
	typedef void (FDYNGL_APIENTRY gl_viewport_arrayv_func)(GLuint first, GLsizei count, const GLfloat *v);
	static gl_viewport_arrayv_func* glViewportArrayv;
	typedef void (FDYNGL_APIENTRY gl_viewport_indexedf_func)(GLuint index, GLfloat x, GLfloat y, GLfloat w, GLfloat h);
	static gl_viewport_indexedf_func* glViewportIndexedf;
	typedef void (FDYNGL_APIENTRY gl_viewport_indexedfv_func)(GLuint index, const GLfloat *v);
	static gl_viewport_indexedfv_func* glViewportIndexedfv;
	typedef void (FDYNGL_APIENTRY gl_scissor_arrayv_func)(GLuint first, GLsizei count, const GLint *v);
	static gl_scissor_arrayv_func* glScissorArrayv;
	typedef void (FDYNGL_APIENTRY gl_scissor_indexed_func)(GLuint index, GLint left, GLint bottom, GLsizei width, GLsizei height);
	static gl_scissor_indexed_func* glScissorIndexed;
	typedef void (FDYNGL_APIENTRY gl_scissor_indexedv_func)(GLuint index, const GLint *v);
	static gl_scissor_indexedv_func* glScissorIndexedv;
	typedef void (FDYNGL_APIENTRY gl_depth_range_arrayv_func)(GLuint first, GLsizei count, const GLdouble *v);
	static gl_depth_range_arrayv_func* glDepthRangeArrayv;
	typedef void (FDYNGL_APIENTRY gl_depth_range_indexed_func)(GLuint index, GLdouble n, GLdouble f);
	static gl_depth_range_indexed_func* glDepthRangeIndexed;
	typedef void (FDYNGL_APIENTRY gl_get_floati_v_func)(GLenum target, GLuint index, GLfloat *data);
	static gl_get_floati_v_func* glGetFloati_v;
	typedef void (FDYNGL_APIENTRY gl_get_doublei_v_func)(GLenum target, GLuint index, GLdouble *data);
	static gl_get_doublei_v_func* glGetDoublei_v;
#	endif // GL_VERSION_4_1
#	ifndef GL_VERSION_4_2
#		define GL_VERSION_4_2 1
	static bool isGL_VERSION_4_2;
	static const uint32_t GL_COPY_READ_BUFFER_BINDING = 0x8F36;
	static const uint32_t GL_COPY_WRITE_BUFFER_BINDING = 0x8F37;
	static const uint32_t GL_TRANSFORM_FEEDBACK_ACTIVE = 0x8E24;
	static const uint32_t GL_TRANSFORM_FEEDBACK_PAUSED = 0x8E23;
	static const uint32_t GL_UNPACK_COMPRESSED_BLOCK_WIDTH = 0x9127;
	static const uint32_t GL_UNPACK_COMPRESSED_BLOCK_HEIGHT = 0x9128;
	static const uint32_t GL_UNPACK_COMPRESSED_BLOCK_DEPTH = 0x9129;
	static const uint32_t GL_UNPACK_COMPRESSED_BLOCK_SIZE = 0x912A;
	static const uint32_t GL_PACK_COMPRESSED_BLOCK_WIDTH = 0x912B;
	static const uint32_t GL_PACK_COMPRESSED_BLOCK_HEIGHT = 0x912C;
	static const uint32_t GL_PACK_COMPRESSED_BLOCK_DEPTH = 0x912D;
	static const uint32_t GL_PACK_COMPRESSED_BLOCK_SIZE = 0x912E;
	static const uint32_t GL_NUM_SAMPLE_COUNTS = 0x9380;
	static const uint32_t GL_MIN_MAP_BUFFER_ALIGNMENT = 0x90BC;
	static const uint32_t GL_ATOMIC_COUNTER_BUFFER = 0x92C0;
	static const uint32_t GL_ATOMIC_COUNTER_BUFFER_BINDING = 0x92C1;
	static const uint32_t GL_ATOMIC_COUNTER_BUFFER_START = 0x92C2;
	static const uint32_t GL_ATOMIC_COUNTER_BUFFER_SIZE = 0x92C3;
	static const uint32_t GL_ATOMIC_COUNTER_BUFFER_DATA_SIZE = 0x92C4;
	static const uint32_t GL_ATOMIC_COUNTER_BUFFER_ACTIVE_ATOMIC_COUNTERS = 0x92C5;
	static const uint32_t GL_ATOMIC_COUNTER_BUFFER_ACTIVE_ATOMIC_COUNTER_INDICES = 0x92C6;
	static const uint32_t GL_ATOMIC_COUNTER_BUFFER_REFERENCED_BY_VERTEX_SHADER = 0x92C7;
	static const uint32_t GL_ATOMIC_COUNTER_BUFFER_REFERENCED_BY_TESS_CONTROL_SHADER = 0x92C8;
	static const uint32_t GL_ATOMIC_COUNTER_BUFFER_REFERENCED_BY_TESS_EVALUATION_SHADER = 0x92C9;
	static const uint32_t GL_ATOMIC_COUNTER_BUFFER_REFERENCED_BY_GEOMETRY_SHADER = 0x92CA;
	static const uint32_t GL_ATOMIC_COUNTER_BUFFER_REFERENCED_BY_FRAGMENT_SHADER = 0x92CB;
	static const uint32_t GL_MAX_VERTEX_ATOMIC_COUNTER_BUFFERS = 0x92CC;
	static const uint32_t GL_MAX_TESS_CONTROL_ATOMIC_COUNTER_BUFFERS = 0x92CD;
	static const uint32_t GL_MAX_TESS_EVALUATION_ATOMIC_COUNTER_BUFFERS = 0x92CE;
	static const uint32_t GL_MAX_GEOMETRY_ATOMIC_COUNTER_BUFFERS = 0x92CF;
	static const uint32_t GL_MAX_FRAGMENT_ATOMIC_COUNTER_BUFFERS = 0x92D0;
	static const uint32_t GL_MAX_COMBINED_ATOMIC_COUNTER_BUFFERS = 0x92D1;
	static const uint32_t GL_MAX_VERTEX_ATOMIC_COUNTERS = 0x92D2;
	static const uint32_t GL_MAX_TESS_CONTROL_ATOMIC_COUNTERS = 0x92D3;
	static const uint32_t GL_MAX_TESS_EVALUATION_ATOMIC_COUNTERS = 0x92D4;
	static const uint32_t GL_MAX_GEOMETRY_ATOMIC_COUNTERS = 0x92D5;
	static const uint32_t GL_MAX_FRAGMENT_ATOMIC_COUNTERS = 0x92D6;
	static const uint32_t GL_MAX_COMBINED_ATOMIC_COUNTERS = 0x92D7;
	static const uint32_t GL_MAX_ATOMIC_COUNTER_BUFFER_SIZE = 0x92D8;
	static const uint32_t GL_MAX_ATOMIC_COUNTER_BUFFER_BINDINGS = 0x92DC;
	static const uint32_t GL_ACTIVE_ATOMIC_COUNTER_BUFFERS = 0x92D9;
	static const uint32_t GL_UNIFORM_ATOMIC_COUNTER_BUFFER_INDEX = 0x92DA;
	static const uint32_t GL_UNSIGNED_INT_ATOMIC_COUNTER = 0x92DB;
	static const uint32_t GL_VERTEX_ATTRIB_ARRAY_BARRIER_BIT = 0x00000001;
	static const uint32_t GL_ELEMENT_ARRAY_BARRIER_BIT = 0x00000002;
	static const uint32_t GL_UNIFORM_BARRIER_BIT = 0x00000004;
	static const uint32_t GL_TEXTURE_FETCH_BARRIER_BIT = 0x00000008;
	static const uint32_t GL_SHADER_IMAGE_ACCESS_BARRIER_BIT = 0x00000020;
	static const uint32_t GL_COMMAND_BARRIER_BIT = 0x00000040;
	static const uint32_t GL_PIXEL_BUFFER_BARRIER_BIT = 0x00000080;
	static const uint32_t GL_TEXTURE_UPDATE_BARRIER_BIT = 0x00000100;
	static const uint32_t GL_BUFFER_UPDATE_BARRIER_BIT = 0x00000200;
	static const uint32_t GL_FRAMEBUFFER_BARRIER_BIT = 0x00000400;
	static const uint32_t GL_TRANSFORM_FEEDBACK_BARRIER_BIT = 0x00000800;
	static const uint32_t GL_ATOMIC_COUNTER_BARRIER_BIT = 0x00001000;
	static const uint32_t GL_ALL_BARRIER_BITS = 0xFFFFFFFF;
	static const uint32_t GL_MAX_IMAGE_UNITS = 0x8F38;
	static const uint32_t GL_MAX_COMBINED_IMAGE_UNITS_AND_FRAGMENT_OUTPUTS = 0x8F39;
	static const uint32_t GL_IMAGE_BINDING_NAME = 0x8F3A;
	static const uint32_t GL_IMAGE_BINDING_LEVEL = 0x8F3B;
	static const uint32_t GL_IMAGE_BINDING_LAYERED = 0x8F3C;
	static const uint32_t GL_IMAGE_BINDING_LAYER = 0x8F3D;
	static const uint32_t GL_IMAGE_BINDING_ACCESS = 0x8F3E;
	static const uint32_t GL_IMAGE_1D = 0x904C;
	static const uint32_t GL_IMAGE_2D = 0x904D;
	static const uint32_t GL_IMAGE_3D = 0x904E;
	static const uint32_t GL_IMAGE_2D_RECT = 0x904F;
	static const uint32_t GL_IMAGE_CUBE = 0x9050;
	static const uint32_t GL_IMAGE_BUFFER = 0x9051;
	static const uint32_t GL_IMAGE_1D_ARRAY = 0x9052;
	static const uint32_t GL_IMAGE_2D_ARRAY = 0x9053;
	static const uint32_t GL_IMAGE_CUBE_MAP_ARRAY = 0x9054;
	static const uint32_t GL_IMAGE_2D_MULTISAMPLE = 0x9055;
	static const uint32_t GL_IMAGE_2D_MULTISAMPLE_ARRAY = 0x9056;
	static const uint32_t GL_INT_IMAGE_1D = 0x9057;
	static const uint32_t GL_INT_IMAGE_2D = 0x9058;
	static const uint32_t GL_INT_IMAGE_3D = 0x9059;
	static const uint32_t GL_INT_IMAGE_2D_RECT = 0x905A;
	static const uint32_t GL_INT_IMAGE_CUBE = 0x905B;
	static const uint32_t GL_INT_IMAGE_BUFFER = 0x905C;
	static const uint32_t GL_INT_IMAGE_1D_ARRAY = 0x905D;
	static const uint32_t GL_INT_IMAGE_2D_ARRAY = 0x905E;
	static const uint32_t GL_INT_IMAGE_CUBE_MAP_ARRAY = 0x905F;
	static const uint32_t GL_INT_IMAGE_2D_MULTISAMPLE = 0x9060;
	static const uint32_t GL_INT_IMAGE_2D_MULTISAMPLE_ARRAY = 0x9061;
	static const uint32_t GL_UNSIGNED_INT_IMAGE_1D = 0x9062;
	static const uint32_t GL_UNSIGNED_INT_IMAGE_2D = 0x9063;
	static const uint32_t GL_UNSIGNED_INT_IMAGE_3D = 0x9064;
	static const uint32_t GL_UNSIGNED_INT_IMAGE_2D_RECT = 0x9065;
	static const uint32_t GL_UNSIGNED_INT_IMAGE_CUBE = 0x9066;
	static const uint32_t GL_UNSIGNED_INT_IMAGE_BUFFER = 0x9067;
	static const uint32_t GL_UNSIGNED_INT_IMAGE_1D_ARRAY = 0x9068;
	static const uint32_t GL_UNSIGNED_INT_IMAGE_2D_ARRAY = 0x9069;
	static const uint32_t GL_UNSIGNED_INT_IMAGE_CUBE_MAP_ARRAY = 0x906A;
	static const uint32_t GL_UNSIGNED_INT_IMAGE_2D_MULTISAMPLE = 0x906B;
	static const uint32_t GL_UNSIGNED_INT_IMAGE_2D_MULTISAMPLE_ARRAY = 0x906C;
	static const uint32_t GL_MAX_IMAGE_SAMPLES = 0x906D;
	static const uint32_t GL_IMAGE_BINDING_FORMAT = 0x906E;
	static const uint32_t GL_IMAGE_FORMAT_COMPATIBILITY_TYPE = 0x90C7;
	static const uint32_t GL_IMAGE_FORMAT_COMPATIBILITY_BY_SIZE = 0x90C8;
	static const uint32_t GL_IMAGE_FORMAT_COMPATIBILITY_BY_CLASS = 0x90C9;
	static const uint32_t GL_MAX_VERTEX_IMAGE_UNIFORMS = 0x90CA;
	static const uint32_t GL_MAX_TESS_CONTROL_IMAGE_UNIFORMS = 0x90CB;
	static const uint32_t GL_MAX_TESS_EVALUATION_IMAGE_UNIFORMS = 0x90CC;
	static const uint32_t GL_MAX_GEOMETRY_IMAGE_UNIFORMS = 0x90CD;
	static const uint32_t GL_MAX_FRAGMENT_IMAGE_UNIFORMS = 0x90CE;
	static const uint32_t GL_MAX_COMBINED_IMAGE_UNIFORMS = 0x90CF;
	static const uint32_t GL_COMPRESSED_RGBA_BPTC_UNORM = 0x8E8C;
	static const uint32_t GL_COMPRESSED_SRGB_ALPHA_BPTC_UNORM = 0x8E8D;
	static const uint32_t GL_COMPRESSED_RGB_BPTC_SIGNED_FLOAT = 0x8E8E;
	static const uint32_t GL_COMPRESSED_RGB_BPTC_UNSIGNED_FLOAT = 0x8E8F;
	static const uint32_t GL_TEXTURE_IMMUTABLE_FORMAT = 0x912F;
	typedef void (FDYNGL_APIENTRY gl_draw_arrays_instanced_base_instance_func)(GLenum mode, GLint first, GLsizei count, GLsizei instancecount, GLuint baseinstance);
	static gl_draw_arrays_instanced_base_instance_func* glDrawArraysInstancedBaseInstance;
	typedef void (FDYNGL_APIENTRY gl_draw_elements_instanced_base_instance_func)(GLenum mode, GLsizei count, GLenum type, const void *indices, GLsizei instancecount, GLuint baseinstance);
	static gl_draw_elements_instanced_base_instance_func* glDrawElementsInstancedBaseInstance;
	typedef void (FDYNGL_APIENTRY gl_draw_elements_instanced_base_vertex_base_instance_func)(GLenum mode, GLsizei count, GLenum type, const void *indices, GLsizei instancecount, GLint basevertex, GLuint baseinstance);
	static gl_draw_elements_instanced_base_vertex_base_instance_func* glDrawElementsInstancedBaseVertexBaseInstance;
	typedef void (FDYNGL_APIENTRY gl_get_internalformativ_func)(GLenum target, GLenum internalformat, GLenum pname, GLsizei bufSize, GLint *params);
	static gl_get_internalformativ_func* glGetInternalformativ;
	typedef void (FDYNGL_APIENTRY gl_get_active_atomic_counter_bufferiv_func)(GLuint program, GLuint bufferIndex, GLenum pname, GLint *params);
	static gl_get_active_atomic_counter_bufferiv_func* glGetActiveAtomicCounterBufferiv;
	typedef void (FDYNGL_APIENTRY gl_bind_image_texture_func)(GLuint unit, GLuint texture, GLint level, GLboolean layered, GLint layer, GLenum access, GLenum format);
	static gl_bind_image_texture_func* glBindImageTexture;
	typedef void (FDYNGL_APIENTRY gl_memory_barrier_func)(GLbitfield barriers);
	static gl_memory_barrier_func* glMemoryBarrier;
	typedef void (FDYNGL_APIENTRY gl_tex_storage1d_func)(GLenum target, GLsizei levels, GLenum internalformat, GLsizei width);
	static gl_tex_storage1d_func* glTexStorage1D;
	typedef void (FDYNGL_APIENTRY gl_tex_storage2d_func)(GLenum target, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height);
	static gl_tex_storage2d_func* glTexStorage2D;
	typedef void (FDYNGL_APIENTRY gl_tex_storage3d_func)(GLenum target, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth);
	static gl_tex_storage3d_func* glTexStorage3D;
	typedef void (FDYNGL_APIENTRY gl_draw_transform_feedback_instanced_func)(GLenum mode, GLuint id, GLsizei instancecount);
	static gl_draw_transform_feedback_instanced_func* glDrawTransformFeedbackInstanced;
	typedef void (FDYNGL_APIENTRY gl_draw_transform_feedback_stream_instanced_func)(GLenum mode, GLuint id, GLuint stream, GLsizei instancecount);
	static gl_draw_transform_feedback_stream_instanced_func* glDrawTransformFeedbackStreamInstanced;
#	endif // GL_VERSION_4_2
#	ifndef GL_VERSION_4_3
#		define GL_VERSION_4_3 1
	static bool isGL_VERSION_4_3;
	typedef void (FDYNGL_APIENTRY *GLDEBUGPROC)(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar *message, const void *userParam);
	static const uint32_t GL_NUM_SHADING_LANGUAGE_VERSIONS = 0x82E9;
	static const uint32_t GL_VERTEX_ATTRIB_ARRAY_LONG = 0x874E;
	static const uint32_t GL_COMPRESSED_RGB8_ETC2 = 0x9274;
	static const uint32_t GL_COMPRESSED_SRGB8_ETC2 = 0x9275;
	static const uint32_t GL_COMPRESSED_RGB8_PUNCHTHROUGH_ALPHA1_ETC2 = 0x9276;
	static const uint32_t GL_COMPRESSED_SRGB8_PUNCHTHROUGH_ALPHA1_ETC2 = 0x9277;
	static const uint32_t GL_COMPRESSED_RGBA8_ETC2_EAC = 0x9278;
	static const uint32_t GL_COMPRESSED_SRGB8_ALPHA8_ETC2_EAC = 0x9279;
	static const uint32_t GL_COMPRESSED_R11_EAC = 0x9270;
	static const uint32_t GL_COMPRESSED_SIGNED_R11_EAC = 0x9271;
	static const uint32_t GL_COMPRESSED_RG11_EAC = 0x9272;
	static const uint32_t GL_COMPRESSED_SIGNED_RG11_EAC = 0x9273;
	static const uint32_t GL_PRIMITIVE_RESTART_FIXED_INDEX = 0x8D69;
	static const uint32_t GL_ANY_SAMPLES_PASSED_CONSERVATIVE = 0x8D6A;
	static const uint32_t GL_MAX_ELEMENT_INDEX = 0x8D6B;
	static const uint32_t GL_COMPUTE_SHADER = 0x91B9;
	static const uint32_t GL_MAX_COMPUTE_UNIFORM_BLOCKS = 0x91BB;
	static const uint32_t GL_MAX_COMPUTE_TEXTURE_IMAGE_UNITS = 0x91BC;
	static const uint32_t GL_MAX_COMPUTE_IMAGE_UNIFORMS = 0x91BD;
	static const uint32_t GL_MAX_COMPUTE_SHARED_MEMORY_SIZE = 0x8262;
	static const uint32_t GL_MAX_COMPUTE_UNIFORM_COMPONENTS = 0x8263;
	static const uint32_t GL_MAX_COMPUTE_ATOMIC_COUNTER_BUFFERS = 0x8264;
	static const uint32_t GL_MAX_COMPUTE_ATOMIC_COUNTERS = 0x8265;
	static const uint32_t GL_MAX_COMBINED_COMPUTE_UNIFORM_COMPONENTS = 0x8266;
	static const uint32_t GL_MAX_COMPUTE_WORK_GROUP_INVOCATIONS = 0x90EB;
	static const uint32_t GL_MAX_COMPUTE_WORK_GROUP_COUNT = 0x91BE;
	static const uint32_t GL_MAX_COMPUTE_WORK_GROUP_SIZE = 0x91BF;
	static const uint32_t GL_COMPUTE_WORK_GROUP_SIZE = 0x8267;
	static const uint32_t GL_UNIFORM_BLOCK_REFERENCED_BY_COMPUTE_SHADER = 0x90EC;
	static const uint32_t GL_ATOMIC_COUNTER_BUFFER_REFERENCED_BY_COMPUTE_SHADER = 0x90ED;
	static const uint32_t GL_DISPATCH_INDIRECT_BUFFER = 0x90EE;
	static const uint32_t GL_DISPATCH_INDIRECT_BUFFER_BINDING = 0x90EF;
	static const uint32_t GL_COMPUTE_SHADER_BIT = 0x00000020;
	static const uint32_t GL_DEBUG_OUTPUT_SYNCHRONOUS = 0x8242;
	static const uint32_t GL_DEBUG_NEXT_LOGGED_MESSAGE_LENGTH = 0x8243;
	static const uint32_t GL_DEBUG_CALLBACK_FUNCTION = 0x8244;
	static const uint32_t GL_DEBUG_CALLBACK_USER_PARAM = 0x8245;
	static const uint32_t GL_DEBUG_SOURCE_API = 0x8246;
	static const uint32_t GL_DEBUG_SOURCE_WINDOW_SYSTEM = 0x8247;
	static const uint32_t GL_DEBUG_SOURCE_SHADER_COMPILER = 0x8248;
	static const uint32_t GL_DEBUG_SOURCE_THIRD_PARTY = 0x8249;
	static const uint32_t GL_DEBUG_SOURCE_APPLICATION = 0x824A;
	static const uint32_t GL_DEBUG_SOURCE_OTHER = 0x824B;
	static const uint32_t GL_DEBUG_TYPE_ERROR = 0x824C;
	static const uint32_t GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR = 0x824D;
	static const uint32_t GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR = 0x824E;
	static const uint32_t GL_DEBUG_TYPE_PORTABILITY = 0x824F;
	static const uint32_t GL_DEBUG_TYPE_PERFORMANCE = 0x8250;
	static const uint32_t GL_DEBUG_TYPE_OTHER = 0x8251;
	static const uint32_t GL_MAX_DEBUG_MESSAGE_LENGTH = 0x9143;
	static const uint32_t GL_MAX_DEBUG_LOGGED_MESSAGES = 0x9144;
	static const uint32_t GL_DEBUG_LOGGED_MESSAGES = 0x9145;
	static const uint32_t GL_DEBUG_SEVERITY_HIGH = 0x9146;
	static const uint32_t GL_DEBUG_SEVERITY_MEDIUM = 0x9147;
	static const uint32_t GL_DEBUG_SEVERITY_LOW = 0x9148;
	static const uint32_t GL_DEBUG_TYPE_MARKER = 0x8268;
	static const uint32_t GL_DEBUG_TYPE_PUSH_GROUP = 0x8269;
	static const uint32_t GL_DEBUG_TYPE_POP_GROUP = 0x826A;
	static const uint32_t GL_DEBUG_SEVERITY_NOTIFICATION = 0x826B;
	static const uint32_t GL_MAX_DEBUG_GROUP_STACK_DEPTH = 0x826C;
	static const uint32_t GL_DEBUG_GROUP_STACK_DEPTH = 0x826D;
	static const uint32_t GL_BUFFER = 0x82E0;
	static const uint32_t GL_SHADER = 0x82E1;
	static const uint32_t GL_PROGRAM = 0x82E2;
	static const uint32_t GL_QUERY = 0x82E3;
	static const uint32_t GL_PROGRAM_PIPELINE = 0x82E4;
	static const uint32_t GL_SAMPLER = 0x82E6;
	static const uint32_t GL_MAX_LABEL_LENGTH = 0x82E8;
	static const uint32_t GL_DEBUG_OUTPUT = 0x92E0;
	static const uint32_t GL_CONTEXT_FLAG_DEBUG_BIT = 0x00000002;
	static const uint32_t GL_MAX_UNIFORM_LOCATIONS = 0x826E;
	static const uint32_t GL_FRAMEBUFFER_DEFAULT_WIDTH = 0x9310;
	static const uint32_t GL_FRAMEBUFFER_DEFAULT_HEIGHT = 0x9311;
	static const uint32_t GL_FRAMEBUFFER_DEFAULT_LAYERS = 0x9312;
	static const uint32_t GL_FRAMEBUFFER_DEFAULT_SAMPLES = 0x9313;
	static const uint32_t GL_FRAMEBUFFER_DEFAULT_FIXED_SAMPLE_LOCATIONS = 0x9314;
	static const uint32_t GL_MAX_FRAMEBUFFER_WIDTH = 0x9315;
	static const uint32_t GL_MAX_FRAMEBUFFER_HEIGHT = 0x9316;
	static const uint32_t GL_MAX_FRAMEBUFFER_LAYERS = 0x9317;
	static const uint32_t GL_MAX_FRAMEBUFFER_SAMPLES = 0x9318;
	static const uint32_t GL_INTERNALFORMAT_SUPPORTED = 0x826F;
	static const uint32_t GL_INTERNALFORMAT_PREFERRED = 0x8270;
	static const uint32_t GL_INTERNALFORMAT_RED_SIZE = 0x8271;
	static const uint32_t GL_INTERNALFORMAT_GREEN_SIZE = 0x8272;
	static const uint32_t GL_INTERNALFORMAT_BLUE_SIZE = 0x8273;
	static const uint32_t GL_INTERNALFORMAT_ALPHA_SIZE = 0x8274;
	static const uint32_t GL_INTERNALFORMAT_DEPTH_SIZE = 0x8275;
	static const uint32_t GL_INTERNALFORMAT_STENCIL_SIZE = 0x8276;
	static const uint32_t GL_INTERNALFORMAT_SHARED_SIZE = 0x8277;
	static const uint32_t GL_INTERNALFORMAT_RED_TYPE = 0x8278;
	static const uint32_t GL_INTERNALFORMAT_GREEN_TYPE = 0x8279;
	static const uint32_t GL_INTERNALFORMAT_BLUE_TYPE = 0x827A;
	static const uint32_t GL_INTERNALFORMAT_ALPHA_TYPE = 0x827B;
	static const uint32_t GL_INTERNALFORMAT_DEPTH_TYPE = 0x827C;
	static const uint32_t GL_INTERNALFORMAT_STENCIL_TYPE = 0x827D;
	static const uint32_t GL_MAX_WIDTH = 0x827E;
	static const uint32_t GL_MAX_HEIGHT = 0x827F;
	static const uint32_t GL_MAX_DEPTH = 0x8280;
	static const uint32_t GL_MAX_LAYERS = 0x8281;
	static const uint32_t GL_MAX_COMBINED_DIMENSIONS = 0x8282;
	static const uint32_t GL_COLOR_COMPONENTS = 0x8283;
	static const uint32_t GL_DEPTH_COMPONENTS = 0x8284;
	static const uint32_t GL_STENCIL_COMPONENTS = 0x8285;
	static const uint32_t GL_COLOR_RENDERABLE = 0x8286;
	static const uint32_t GL_DEPTH_RENDERABLE = 0x8287;
	static const uint32_t GL_STENCIL_RENDERABLE = 0x8288;
	static const uint32_t GL_FRAMEBUFFER_RENDERABLE = 0x8289;
	static const uint32_t GL_FRAMEBUFFER_RENDERABLE_LAYERED = 0x828A;
	static const uint32_t GL_FRAMEBUFFER_BLEND = 0x828B;
	static const uint32_t GL_READ_PIXELS = 0x828C;
	static const uint32_t GL_READ_PIXELS_FORMAT = 0x828D;
	static const uint32_t GL_READ_PIXELS_TYPE = 0x828E;
	static const uint32_t GL_TEXTURE_IMAGE_FORMAT = 0x828F;
	static const uint32_t GL_TEXTURE_IMAGE_TYPE = 0x8290;
	static const uint32_t GL_GET_TEXTURE_IMAGE_FORMAT = 0x8291;
	static const uint32_t GL_GET_TEXTURE_IMAGE_TYPE = 0x8292;
	static const uint32_t GL_MIPMAP = 0x8293;
	static const uint32_t GL_MANUAL_GENERATE_MIPMAP = 0x8294;
	static const uint32_t GL_AUTO_GENERATE_MIPMAP = 0x8295;
	static const uint32_t GL_COLOR_ENCODING = 0x8296;
	static const uint32_t GL_SRGB_READ = 0x8297;
	static const uint32_t GL_SRGB_WRITE = 0x8298;
	static const uint32_t GL_FILTER = 0x829A;
	static const uint32_t GL_VERTEX_TEXTURE = 0x829B;
	static const uint32_t GL_TESS_CONTROL_TEXTURE = 0x829C;
	static const uint32_t GL_TESS_EVALUATION_TEXTURE = 0x829D;
	static const uint32_t GL_GEOMETRY_TEXTURE = 0x829E;
	static const uint32_t GL_FRAGMENT_TEXTURE = 0x829F;
	static const uint32_t GL_COMPUTE_TEXTURE = 0x82A0;
	static const uint32_t GL_TEXTURE_SHADOW = 0x82A1;
	static const uint32_t GL_TEXTURE_GATHER = 0x82A2;
	static const uint32_t GL_TEXTURE_GATHER_SHADOW = 0x82A3;
	static const uint32_t GL_SHADER_IMAGE_LOAD = 0x82A4;
	static const uint32_t GL_SHADER_IMAGE_STORE = 0x82A5;
	static const uint32_t GL_SHADER_IMAGE_ATOMIC = 0x82A6;
	static const uint32_t GL_IMAGE_TEXEL_SIZE = 0x82A7;
	static const uint32_t GL_IMAGE_COMPATIBILITY_CLASS = 0x82A8;
	static const uint32_t GL_IMAGE_PIXEL_FORMAT = 0x82A9;
	static const uint32_t GL_IMAGE_PIXEL_TYPE = 0x82AA;
	static const uint32_t GL_SIMULTANEOUS_TEXTURE_AND_DEPTH_TEST = 0x82AC;
	static const uint32_t GL_SIMULTANEOUS_TEXTURE_AND_STENCIL_TEST = 0x82AD;
	static const uint32_t GL_SIMULTANEOUS_TEXTURE_AND_DEPTH_WRITE = 0x82AE;
	static const uint32_t GL_SIMULTANEOUS_TEXTURE_AND_STENCIL_WRITE = 0x82AF;
	static const uint32_t GL_TEXTURE_COMPRESSED_BLOCK_WIDTH = 0x82B1;
	static const uint32_t GL_TEXTURE_COMPRESSED_BLOCK_HEIGHT = 0x82B2;
	static const uint32_t GL_TEXTURE_COMPRESSED_BLOCK_SIZE = 0x82B3;
	static const uint32_t GL_CLEAR_BUFFER = 0x82B4;
	static const uint32_t GL_TEXTURE_VIEW = 0x82B5;
	static const uint32_t GL_VIEW_COMPATIBILITY_CLASS = 0x82B6;
	static const uint32_t GL_FULL_SUPPORT = 0x82B7;
	static const uint32_t GL_CAVEAT_SUPPORT = 0x82B8;
	static const uint32_t GL_IMAGE_CLASS_4_X_32 = 0x82B9;
	static const uint32_t GL_IMAGE_CLASS_2_X_32 = 0x82BA;
	static const uint32_t GL_IMAGE_CLASS_1_X_32 = 0x82BB;
	static const uint32_t GL_IMAGE_CLASS_4_X_16 = 0x82BC;
	static const uint32_t GL_IMAGE_CLASS_2_X_16 = 0x82BD;
	static const uint32_t GL_IMAGE_CLASS_1_X_16 = 0x82BE;
	static const uint32_t GL_IMAGE_CLASS_4_X_8 = 0x82BF;
	static const uint32_t GL_IMAGE_CLASS_2_X_8 = 0x82C0;
	static const uint32_t GL_IMAGE_CLASS_1_X_8 = 0x82C1;
	static const uint32_t GL_IMAGE_CLASS_11_11_10 = 0x82C2;
	static const uint32_t GL_IMAGE_CLASS_10_10_10_2 = 0x82C3;
	static const uint32_t GL_VIEW_CLASS_128_BITS = 0x82C4;
	static const uint32_t GL_VIEW_CLASS_96_BITS = 0x82C5;
	static const uint32_t GL_VIEW_CLASS_64_BITS = 0x82C6;
	static const uint32_t GL_VIEW_CLASS_48_BITS = 0x82C7;
	static const uint32_t GL_VIEW_CLASS_32_BITS = 0x82C8;
	static const uint32_t GL_VIEW_CLASS_24_BITS = 0x82C9;
	static const uint32_t GL_VIEW_CLASS_16_BITS = 0x82CA;
	static const uint32_t GL_VIEW_CLASS_8_BITS = 0x82CB;
	static const uint32_t GL_VIEW_CLASS_S3TC_DXT1_RGB = 0x82CC;
	static const uint32_t GL_VIEW_CLASS_S3TC_DXT1_RGBA = 0x82CD;
	static const uint32_t GL_VIEW_CLASS_S3TC_DXT3_RGBA = 0x82CE;
	static const uint32_t GL_VIEW_CLASS_S3TC_DXT5_RGBA = 0x82CF;
	static const uint32_t GL_VIEW_CLASS_RGTC1_RED = 0x82D0;
	static const uint32_t GL_VIEW_CLASS_RGTC2_RG = 0x82D1;
	static const uint32_t GL_VIEW_CLASS_BPTC_UNORM = 0x82D2;
	static const uint32_t GL_VIEW_CLASS_BPTC_FLOAT = 0x82D3;
	static const uint32_t GL_UNIFORM = 0x92E1;
	static const uint32_t GL_UNIFORM_BLOCK = 0x92E2;
	static const uint32_t GL_PROGRAM_INPUT = 0x92E3;
	static const uint32_t GL_PROGRAM_OUTPUT = 0x92E4;
	static const uint32_t GL_BUFFER_VARIABLE = 0x92E5;
	static const uint32_t GL_SHADER_STORAGE_BLOCK = 0x92E6;
	static const uint32_t GL_VERTEX_SUBROUTINE = 0x92E8;
	static const uint32_t GL_TESS_CONTROL_SUBROUTINE = 0x92E9;
	static const uint32_t GL_TESS_EVALUATION_SUBROUTINE = 0x92EA;
	static const uint32_t GL_GEOMETRY_SUBROUTINE = 0x92EB;
	static const uint32_t GL_FRAGMENT_SUBROUTINE = 0x92EC;
	static const uint32_t GL_COMPUTE_SUBROUTINE = 0x92ED;
	static const uint32_t GL_VERTEX_SUBROUTINE_UNIFORM = 0x92EE;
	static const uint32_t GL_TESS_CONTROL_SUBROUTINE_UNIFORM = 0x92EF;
	static const uint32_t GL_TESS_EVALUATION_SUBROUTINE_UNIFORM = 0x92F0;
	static const uint32_t GL_GEOMETRY_SUBROUTINE_UNIFORM = 0x92F1;
	static const uint32_t GL_FRAGMENT_SUBROUTINE_UNIFORM = 0x92F2;
	static const uint32_t GL_COMPUTE_SUBROUTINE_UNIFORM = 0x92F3;
	static const uint32_t GL_TRANSFORM_FEEDBACK_VARYING = 0x92F4;
	static const uint32_t GL_ACTIVE_RESOURCES = 0x92F5;
	static const uint32_t GL_MAX_NAME_LENGTH = 0x92F6;
	static const uint32_t GL_MAX_NUM_ACTIVE_VARIABLES = 0x92F7;
	static const uint32_t GL_MAX_NUM_COMPATIBLE_SUBROUTINES = 0x92F8;
	static const uint32_t GL_NAME_LENGTH = 0x92F9;
	static const uint32_t GL_TYPE = 0x92FA;
	static const uint32_t GL_ARRAY_SIZE = 0x92FB;
	static const uint32_t GL_OFFSET = 0x92FC;
	static const uint32_t GL_BLOCK_INDEX = 0x92FD;
	static const uint32_t GL_ARRAY_STRIDE = 0x92FE;
	static const uint32_t GL_MATRIX_STRIDE = 0x92FF;
	static const uint32_t GL_IS_ROW_MAJOR = 0x9300;
	static const uint32_t GL_ATOMIC_COUNTER_BUFFER_INDEX = 0x9301;
	static const uint32_t GL_BUFFER_BINDING = 0x9302;
	static const uint32_t GL_BUFFER_DATA_SIZE = 0x9303;
	static const uint32_t GL_NUM_ACTIVE_VARIABLES = 0x9304;
	static const uint32_t GL_ACTIVE_VARIABLES = 0x9305;
	static const uint32_t GL_REFERENCED_BY_VERTEX_SHADER = 0x9306;
	static const uint32_t GL_REFERENCED_BY_TESS_CONTROL_SHADER = 0x9307;
	static const uint32_t GL_REFERENCED_BY_TESS_EVALUATION_SHADER = 0x9308;
	static const uint32_t GL_REFERENCED_BY_GEOMETRY_SHADER = 0x9309;
	static const uint32_t GL_REFERENCED_BY_FRAGMENT_SHADER = 0x930A;
	static const uint32_t GL_REFERENCED_BY_COMPUTE_SHADER = 0x930B;
	static const uint32_t GL_TOP_LEVEL_ARRAY_SIZE = 0x930C;
	static const uint32_t GL_TOP_LEVEL_ARRAY_STRIDE = 0x930D;
	static const uint32_t GL_LOCATION = 0x930E;
	static const uint32_t GL_LOCATION_INDEX = 0x930F;
	static const uint32_t GL_IS_PER_PATCH = 0x92E7;
	static const uint32_t GL_SHADER_STORAGE_BUFFER = 0x90D2;
	static const uint32_t GL_SHADER_STORAGE_BUFFER_BINDING = 0x90D3;
	static const uint32_t GL_SHADER_STORAGE_BUFFER_START = 0x90D4;
	static const uint32_t GL_SHADER_STORAGE_BUFFER_SIZE = 0x90D5;
	static const uint32_t GL_MAX_VERTEX_SHADER_STORAGE_BLOCKS = 0x90D6;
	static const uint32_t GL_MAX_GEOMETRY_SHADER_STORAGE_BLOCKS = 0x90D7;
	static const uint32_t GL_MAX_TESS_CONTROL_SHADER_STORAGE_BLOCKS = 0x90D8;
	static const uint32_t GL_MAX_TESS_EVALUATION_SHADER_STORAGE_BLOCKS = 0x90D9;
	static const uint32_t GL_MAX_FRAGMENT_SHADER_STORAGE_BLOCKS = 0x90DA;
	static const uint32_t GL_MAX_COMPUTE_SHADER_STORAGE_BLOCKS = 0x90DB;
	static const uint32_t GL_MAX_COMBINED_SHADER_STORAGE_BLOCKS = 0x90DC;
	static const uint32_t GL_MAX_SHADER_STORAGE_BUFFER_BINDINGS = 0x90DD;
	static const uint32_t GL_MAX_SHADER_STORAGE_BLOCK_SIZE = 0x90DE;
	static const uint32_t GL_SHADER_STORAGE_BUFFER_OFFSET_ALIGNMENT = 0x90DF;
	static const uint32_t GL_SHADER_STORAGE_BARRIER_BIT = 0x00002000;
	static const uint32_t GL_MAX_COMBINED_SHADER_OUTPUT_RESOURCES = 0x8F39;
	static const uint32_t GL_DEPTH_STENCIL_TEXTURE_MODE = 0x90EA;
	static const uint32_t GL_TEXTURE_BUFFER_OFFSET = 0x919D;
	static const uint32_t GL_TEXTURE_BUFFER_SIZE = 0x919E;
	static const uint32_t GL_TEXTURE_BUFFER_OFFSET_ALIGNMENT = 0x919F;
	static const uint32_t GL_TEXTURE_VIEW_MIN_LEVEL = 0x82DB;
	static const uint32_t GL_TEXTURE_VIEW_NUM_LEVELS = 0x82DC;
	static const uint32_t GL_TEXTURE_VIEW_MIN_LAYER = 0x82DD;
	static const uint32_t GL_TEXTURE_VIEW_NUM_LAYERS = 0x82DE;
	static const uint32_t GL_TEXTURE_IMMUTABLE_LEVELS = 0x82DF;
	static const uint32_t GL_VERTEX_ATTRIB_BINDING = 0x82D4;
	static const uint32_t GL_VERTEX_ATTRIB_RELATIVE_OFFSET = 0x82D5;
	static const uint32_t GL_VERTEX_BINDING_DIVISOR = 0x82D6;
	static const uint32_t GL_VERTEX_BINDING_OFFSET = 0x82D7;
	static const uint32_t GL_VERTEX_BINDING_STRIDE = 0x82D8;
	static const uint32_t GL_MAX_VERTEX_ATTRIB_RELATIVE_OFFSET = 0x82D9;
	static const uint32_t GL_MAX_VERTEX_ATTRIB_BINDINGS = 0x82DA;
	static const uint32_t GL_VERTEX_BINDING_BUFFER = 0x8F4F;
	static const uint32_t GL_DISPLAY_LIST = 0x82E7;
	typedef void (FDYNGL_APIENTRY gl_clear_buffer_data_func)(GLenum target, GLenum internalformat, GLenum format, GLenum type, const void *data);
	static gl_clear_buffer_data_func* glClearBufferData;
	typedef void (FDYNGL_APIENTRY gl_clear_buffer_sub_data_func)(GLenum target, GLenum internalformat, GLintptr offset, GLsizeiptr size, GLenum format, GLenum type, const void *data);
	static gl_clear_buffer_sub_data_func* glClearBufferSubData;
	typedef void (FDYNGL_APIENTRY gl_dispatch_compute_func)(GLuint num_groups_x, GLuint num_groups_y, GLuint num_groups_z);
	static gl_dispatch_compute_func* glDispatchCompute;
	typedef void (FDYNGL_APIENTRY gl_dispatch_compute_indirect_func)(GLintptr indirect);
	static gl_dispatch_compute_indirect_func* glDispatchComputeIndirect;
	typedef void (FDYNGL_APIENTRY gl_copy_image_sub_data_func)(GLuint srcName, GLenum srcTarget, GLint srcLevel, GLint srcX, GLint srcY, GLint srcZ, GLuint dstName, GLenum dstTarget, GLint dstLevel, GLint dstX, GLint dstY, GLint dstZ, GLsizei srcWidth, GLsizei srcHeight, GLsizei srcDepth);
	static gl_copy_image_sub_data_func* glCopyImageSubData;
	typedef void (FDYNGL_APIENTRY gl_framebuffer_parameteri_func)(GLenum target, GLenum pname, GLint param);
	static gl_framebuffer_parameteri_func* glFramebufferParameteri;
	typedef void (FDYNGL_APIENTRY gl_get_framebuffer_parameteriv_func)(GLenum target, GLenum pname, GLint *params);
	static gl_get_framebuffer_parameteriv_func* glGetFramebufferParameteriv;
	typedef void (FDYNGL_APIENTRY gl_get_internalformati64v_func)(GLenum target, GLenum internalformat, GLenum pname, GLsizei bufSize, GLint64 *params);
	static gl_get_internalformati64v_func* glGetInternalformati64v;
	typedef void (FDYNGL_APIENTRY gl_invalidate_tex_sub_image_func)(GLuint texture, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth);
	static gl_invalidate_tex_sub_image_func* glInvalidateTexSubImage;
	typedef void (FDYNGL_APIENTRY gl_invalidate_tex_image_func)(GLuint texture, GLint level);
	static gl_invalidate_tex_image_func* glInvalidateTexImage;
	typedef void (FDYNGL_APIENTRY gl_invalidate_buffer_sub_data_func)(GLuint buffer, GLintptr offset, GLsizeiptr length);
	static gl_invalidate_buffer_sub_data_func* glInvalidateBufferSubData;
	typedef void (FDYNGL_APIENTRY gl_invalidate_buffer_data_func)(GLuint buffer);
	static gl_invalidate_buffer_data_func* glInvalidateBufferData;
	typedef void (FDYNGL_APIENTRY gl_invalidate_framebuffer_func)(GLenum target, GLsizei numAttachments, const GLenum *attachments);
	static gl_invalidate_framebuffer_func* glInvalidateFramebuffer;
	typedef void (FDYNGL_APIENTRY gl_invalidate_sub_framebuffer_func)(GLenum target, GLsizei numAttachments, const GLenum *attachments, GLint x, GLint y, GLsizei width, GLsizei height);
	static gl_invalidate_sub_framebuffer_func* glInvalidateSubFramebuffer;
	typedef void (FDYNGL_APIENTRY gl_multi_draw_arrays_indirect_func)(GLenum mode, const void *indirect, GLsizei drawcount, GLsizei stride);
	static gl_multi_draw_arrays_indirect_func* glMultiDrawArraysIndirect;
	typedef void (FDYNGL_APIENTRY gl_multi_draw_elements_indirect_func)(GLenum mode, GLenum type, const void *indirect, GLsizei drawcount, GLsizei stride);
	static gl_multi_draw_elements_indirect_func* glMultiDrawElementsIndirect;
	typedef void (FDYNGL_APIENTRY gl_get_program_interfaceiv_func)(GLuint program, GLenum programInterface, GLenum pname, GLint *params);
	static gl_get_program_interfaceiv_func* glGetProgramInterfaceiv;
	typedef GLuint(FDYNGL_APIENTRY gl_get_program_resource_index_func)(GLuint program, GLenum programInterface, const GLchar *name);
	static gl_get_program_resource_index_func* glGetProgramResourceIndex;
	typedef void (FDYNGL_APIENTRY gl_get_program_resource_name_func)(GLuint program, GLenum programInterface, GLuint index, GLsizei bufSize, GLsizei *length, GLchar *name);
	static gl_get_program_resource_name_func* glGetProgramResourceName;
	typedef void (FDYNGL_APIENTRY gl_get_program_resourceiv_func)(GLuint program, GLenum programInterface, GLuint index, GLsizei propCount, const GLenum *props, GLsizei bufSize, GLsizei *length, GLint *params);
	static gl_get_program_resourceiv_func* glGetProgramResourceiv;
	typedef GLint(FDYNGL_APIENTRY gl_get_program_resource_location_func)(GLuint program, GLenum programInterface, const GLchar *name);
	static gl_get_program_resource_location_func* glGetProgramResourceLocation;
	typedef GLint(FDYNGL_APIENTRY gl_get_program_resource_location_index_func)(GLuint program, GLenum programInterface, const GLchar *name);
	static gl_get_program_resource_location_index_func* glGetProgramResourceLocationIndex;
	typedef void (FDYNGL_APIENTRY gl_shader_storage_block_binding_func)(GLuint program, GLuint storageBlockIndex, GLuint storageBlockBinding);
	static gl_shader_storage_block_binding_func* glShaderStorageBlockBinding;
	typedef void (FDYNGL_APIENTRY gl_tex_buffer_range_func)(GLenum target, GLenum internalformat, GLuint buffer, GLintptr offset, GLsizeiptr size);
	static gl_tex_buffer_range_func* glTexBufferRange;
	typedef void (FDYNGL_APIENTRY gl_tex_storage2_d_multisample_func)(GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height, GLboolean fixedsamplelocations);
	static gl_tex_storage2_d_multisample_func* glTexStorage2DMultisample;
	typedef void (FDYNGL_APIENTRY gl_tex_storage3_d_multisample_func)(GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLboolean fixedsamplelocations);
	static gl_tex_storage3_d_multisample_func* glTexStorage3DMultisample;
	typedef void (FDYNGL_APIENTRY gl_texture_view_func)(GLuint texture, GLenum target, GLuint origtexture, GLenum internalformat, GLuint minlevel, GLuint numlevels, GLuint minlayer, GLuint numlayers);
	static gl_texture_view_func* glTextureView;
	typedef void (FDYNGL_APIENTRY gl_bind_vertex_buffer_func)(GLuint bindingindex, GLuint buffer, GLintptr offset, GLsizei stride);
	static gl_bind_vertex_buffer_func* glBindVertexBuffer;
	typedef void (FDYNGL_APIENTRY gl_vertex_attrib_format_func)(GLuint attribindex, GLint size, GLenum type, GLboolean normalized, GLuint relativeoffset);
	static gl_vertex_attrib_format_func* glVertexAttribFormat;
	typedef void (FDYNGL_APIENTRY gl_vertex_attrib_i_format_func)(GLuint attribindex, GLint size, GLenum type, GLuint relativeoffset);
	static gl_vertex_attrib_i_format_func* glVertexAttribIFormat;
	typedef void (FDYNGL_APIENTRY gl_vertex_attrib_l_format_func)(GLuint attribindex, GLint size, GLenum type, GLuint relativeoffset);
	static gl_vertex_attrib_l_format_func* glVertexAttribLFormat;
	typedef void (FDYNGL_APIENTRY gl_vertex_attrib_binding_func)(GLuint attribindex, GLuint bindingindex);
	static gl_vertex_attrib_binding_func* glVertexAttribBinding;
	typedef void (FDYNGL_APIENTRY gl_vertex_binding_divisor_func)(GLuint bindingindex, GLuint divisor);
	static gl_vertex_binding_divisor_func* glVertexBindingDivisor;
	typedef void (FDYNGL_APIENTRY gl_debug_message_control_func)(GLenum source, GLenum type, GLenum severity, GLsizei count, const GLuint *ids, GLboolean enabled);
	static gl_debug_message_control_func* glDebugMessageControl;
	typedef void (FDYNGL_APIENTRY gl_debug_message_insert_func)(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar *buf);
	static gl_debug_message_insert_func* glDebugMessageInsert;
	typedef void (FDYNGL_APIENTRY gl_debug_message_callback_func)(GLDEBUGPROC callback, const void *userParam);
	static gl_debug_message_callback_func* glDebugMessageCallback;
	typedef GLuint(FDYNGL_APIENTRY gl_get_debug_message_log_func)(GLuint count, GLsizei bufSize, GLenum *sources, GLenum *types, GLuint *ids, GLenum *severities, GLsizei *lengths, GLchar *messageLog);
	static gl_get_debug_message_log_func* glGetDebugMessageLog;
	typedef void (FDYNGL_APIENTRY gl_push_debug_group_func)(GLenum source, GLuint id, GLsizei length, const GLchar *message);
	static gl_push_debug_group_func* glPushDebugGroup;
	typedef void (FDYNGL_APIENTRY gl_pop_debug_group_func)(void);
	static gl_pop_debug_group_func* glPopDebugGroup;
	typedef void (FDYNGL_APIENTRY gl_object_label_func)(GLenum identifier, GLuint name, GLsizei length, const GLchar *label);
	static gl_object_label_func* glObjectLabel;
	typedef void (FDYNGL_APIENTRY gl_get_object_label_func)(GLenum identifier, GLuint name, GLsizei bufSize, GLsizei *length, GLchar *label);
	static gl_get_object_label_func* glGetObjectLabel;
	typedef void (FDYNGL_APIENTRY gl_object_ptr_label_func)(const void *ptr, GLsizei length, const GLchar *label);
	static gl_object_ptr_label_func* glObjectPtrLabel;
	typedef void (FDYNGL_APIENTRY gl_get_object_ptr_label_func)(const void *ptr, GLsizei bufSize, GLsizei *length, GLchar *label);
	static gl_get_object_ptr_label_func* glGetObjectPtrLabel;
#	endif // GL_VERSION_4_3
#	ifndef GL_VERSION_4_4
#		define GL_VERSION_4_4 1
	static bool isGL_VERSION_4_4;
	static const uint32_t GL_MAX_VERTEX_ATTRIB_STRIDE = 0x82E5;
	static const uint32_t GL_PRIMITIVE_RESTART_FOR_PATCHES_SUPPORTED = 0x8221;
	static const uint32_t GL_TEXTURE_BUFFER_BINDING = 0x8C2A;
	static const uint32_t GL_MAP_PERSISTENT_BIT = 0x0040;
	static const uint32_t GL_MAP_COHERENT_BIT = 0x0080;
	static const uint32_t GL_DYNAMIC_STORAGE_BIT = 0x0100;
	static const uint32_t GL_CLIENT_STORAGE_BIT = 0x0200;
	static const uint32_t GL_CLIENT_MAPPED_BUFFER_BARRIER_BIT = 0x00004000;
	static const uint32_t GL_BUFFER_IMMUTABLE_STORAGE = 0x821F;
	static const uint32_t GL_BUFFER_STORAGE_FLAGS = 0x8220;
	static const uint32_t GL_CLEAR_TEXTURE = 0x9365;
	static const uint32_t GL_LOCATION_COMPONENT = 0x934A;
	static const uint32_t GL_TRANSFORM_FEEDBACK_BUFFER_INDEX = 0x934B;
	static const uint32_t GL_TRANSFORM_FEEDBACK_BUFFER_STRIDE = 0x934C;
	static const uint32_t GL_QUERY_BUFFER = 0x9192;
	static const uint32_t GL_QUERY_BUFFER_BARRIER_BIT = 0x00008000;
	static const uint32_t GL_QUERY_BUFFER_BINDING = 0x9193;
	static const uint32_t GL_QUERY_RESULT_NO_WAIT = 0x9194;
	static const uint32_t GL_MIRROR_CLAMP_TO_EDGE = 0x8743;
	typedef void (FDYNGL_APIENTRY gl_buffer_storage_func)(GLenum target, GLsizeiptr size, const void *data, GLbitfield flags);
	static gl_buffer_storage_func* glBufferStorage;
	typedef void (FDYNGL_APIENTRY gl_clear_tex_image_func)(GLuint texture, GLint level, GLenum format, GLenum type, const void *data);
	static gl_clear_tex_image_func* glClearTexImage;
	typedef void (FDYNGL_APIENTRY gl_clear_tex_sub_image_func)(GLuint texture, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const void *data);
	static gl_clear_tex_sub_image_func* glClearTexSubImage;
	typedef void (FDYNGL_APIENTRY gl_bind_buffers_base_func)(GLenum target, GLuint first, GLsizei count, const GLuint *buffers);
	static gl_bind_buffers_base_func* glBindBuffersBase;
	typedef void (FDYNGL_APIENTRY gl_bind_buffers_range_func)(GLenum target, GLuint first, GLsizei count, const GLuint *buffers, const GLintptr *offsets, const GLsizeiptr *sizes);
	static gl_bind_buffers_range_func* glBindBuffersRange;
	typedef void (FDYNGL_APIENTRY gl_bind_textures_func)(GLuint first, GLsizei count, const GLuint *textures);
	static gl_bind_textures_func* glBindTextures;
	typedef void (FDYNGL_APIENTRY gl_bind_samplers_func)(GLuint first, GLsizei count, const GLuint *samplers);
	static gl_bind_samplers_func* glBindSamplers;
	typedef void (FDYNGL_APIENTRY gl_bind_image_textures_func)(GLuint first, GLsizei count, const GLuint *textures);
	static gl_bind_image_textures_func* glBindImageTextures;
	typedef void (FDYNGL_APIENTRY gl_bind_vertex_buffers_func)(GLuint first, GLsizei count, const GLuint *buffers, const GLintptr *offsets, const GLsizei *strides);
	static gl_bind_vertex_buffers_func* glBindVertexBuffers;
#	endif // GL_VERSION_4_4
#	ifndef GL_VERSION_4_5
#		define GL_VERSION_4_5 1
	static bool isGL_VERSION_4_5;
	static const uint32_t GL_CONTEXT_LOST = 0x0507;
	static const uint32_t GL_NEGATIVE_ONE_TO_ONE = 0x935E;
	static const uint32_t GL_ZERO_TO_ONE = 0x935F;
	static const uint32_t GL_CLIP_ORIGIN = 0x935C;
	static const uint32_t GL_CLIP_DEPTH_MODE = 0x935D;
	static const uint32_t GL_QUERY_WAIT_INVERTED = 0x8E17;
	static const uint32_t GL_QUERY_NO_WAIT_INVERTED = 0x8E18;
	static const uint32_t GL_QUERY_BY_REGION_WAIT_INVERTED = 0x8E19;
	static const uint32_t GL_QUERY_BY_REGION_NO_WAIT_INVERTED = 0x8E1A;
	static const uint32_t GL_MAX_CULL_DISTANCES = 0x82F9;
	static const uint32_t GL_MAX_COMBINED_CLIP_AND_CULL_DISTANCES = 0x82FA;
	static const uint32_t GL_TEXTURE_TARGET = 0x1006;
	static const uint32_t GL_QUERY_TARGET = 0x82EA;
	static const uint32_t GL_GUILTY_CONTEXT_RESET = 0x8253;
	static const uint32_t GL_INNOCENT_CONTEXT_RESET = 0x8254;
	static const uint32_t GL_UNKNOWN_CONTEXT_RESET = 0x8255;
	static const uint32_t GL_RESET_NOTIFICATION_STRATEGY = 0x8256;
	static const uint32_t GL_LOSE_CONTEXT_ON_RESET = 0x8252;
	static const uint32_t GL_NO_RESET_NOTIFICATION = 0x8261;
	static const uint32_t GL_CONTEXT_FLAG_ROBUST_ACCESS_BIT = 0x00000004;
	static const uint32_t GL_CONTEXT_RELEASE_BEHAVIOR = 0x82FB;
	static const uint32_t GL_CONTEXT_RELEASE_BEHAVIOR_FLUSH = 0x82FC;
	typedef void (FDYNGL_APIENTRY gl_clip_control_func)(GLenum origin, GLenum depth);
	static gl_clip_control_func* glClipControl;
	typedef void (FDYNGL_APIENTRY gl_create_transform_feedbacks_func)(GLsizei n, GLuint *ids);
	static gl_create_transform_feedbacks_func* glCreateTransformFeedbacks;
	typedef void (FDYNGL_APIENTRY gl_transform_feedback_buffer_base_func)(GLuint xfb, GLuint index, GLuint buffer);
	static gl_transform_feedback_buffer_base_func* glTransformFeedbackBufferBase;
	typedef void (FDYNGL_APIENTRY gl_transform_feedback_buffer_range_func)(GLuint xfb, GLuint index, GLuint buffer, GLintptr offset, GLsizeiptr size);
	static gl_transform_feedback_buffer_range_func* glTransformFeedbackBufferRange;
	typedef void (FDYNGL_APIENTRY gl_get_transform_feedbackiv_func)(GLuint xfb, GLenum pname, GLint *param);
	static gl_get_transform_feedbackiv_func* glGetTransformFeedbackiv;
	typedef void (FDYNGL_APIENTRY gl_get_transform_feedbacki_v_func)(GLuint xfb, GLenum pname, GLuint index, GLint *param);
	static gl_get_transform_feedbacki_v_func* glGetTransformFeedbacki_v;
	typedef void (FDYNGL_APIENTRY gl_get_transform_feedbacki64_v_func)(GLuint xfb, GLenum pname, GLuint index, GLint64 *param);
	static gl_get_transform_feedbacki64_v_func* glGetTransformFeedbacki64_v;
	typedef void (FDYNGL_APIENTRY gl_create_buffers_func)(GLsizei n, GLuint *buffers);
	static gl_create_buffers_func* glCreateBuffers;
	typedef void (FDYNGL_APIENTRY gl_named_buffer_storage_func)(GLuint buffer, GLsizeiptr size, const void *data, GLbitfield flags);
	static gl_named_buffer_storage_func* glNamedBufferStorage;
	typedef void (FDYNGL_APIENTRY gl_named_buffer_data_func)(GLuint buffer, GLsizeiptr size, const void *data, GLenum usage);
	static gl_named_buffer_data_func* glNamedBufferData;
	typedef void (FDYNGL_APIENTRY gl_named_buffer_sub_data_func)(GLuint buffer, GLintptr offset, GLsizeiptr size, const void *data);
	static gl_named_buffer_sub_data_func* glNamedBufferSubData;
	typedef void (FDYNGL_APIENTRY gl_copy_named_buffer_sub_data_func)(GLuint readBuffer, GLuint writeBuffer, GLintptr readOffset, GLintptr writeOffset, GLsizeiptr size);
	static gl_copy_named_buffer_sub_data_func* glCopyNamedBufferSubData;
	typedef void (FDYNGL_APIENTRY gl_clear_named_buffer_data_func)(GLuint buffer, GLenum internalformat, GLenum format, GLenum type, const void *data);
	static gl_clear_named_buffer_data_func* glClearNamedBufferData;
	typedef void (FDYNGL_APIENTRY gl_clear_named_buffer_sub_data_func)(GLuint buffer, GLenum internalformat, GLintptr offset, GLsizeiptr size, GLenum format, GLenum type, const void *data);
	static gl_clear_named_buffer_sub_data_func* glClearNamedBufferSubData;
	typedef void * (FDYNGL_APIENTRY gl_map_named_buffer_func)(GLuint buffer, GLenum access);
	static gl_map_named_buffer_func* glMapNamedBuffer;
	typedef void * (FDYNGL_APIENTRY gl_map_named_buffer_range_func)(GLuint buffer, GLintptr offset, GLsizeiptr length, GLbitfield access);
	static gl_map_named_buffer_range_func* glMapNamedBufferRange;
	typedef GLboolean(FDYNGL_APIENTRY gl_unmap_named_buffer_func)(GLuint buffer);
	static gl_unmap_named_buffer_func* glUnmapNamedBuffer;
	typedef void (FDYNGL_APIENTRY gl_flush_mapped_named_buffer_range_func)(GLuint buffer, GLintptr offset, GLsizeiptr length);
	static gl_flush_mapped_named_buffer_range_func* glFlushMappedNamedBufferRange;
	typedef void (FDYNGL_APIENTRY gl_get_named_buffer_parameteriv_func)(GLuint buffer, GLenum pname, GLint *params);
	static gl_get_named_buffer_parameteriv_func* glGetNamedBufferParameteriv;
	typedef void (FDYNGL_APIENTRY gl_get_named_buffer_parameteri64v_func)(GLuint buffer, GLenum pname, GLint64 *params);
	static gl_get_named_buffer_parameteri64v_func* glGetNamedBufferParameteri64v;
	typedef void (FDYNGL_APIENTRY gl_get_named_buffer_pointerv_func)(GLuint buffer, GLenum pname, void **params);
	static gl_get_named_buffer_pointerv_func* glGetNamedBufferPointerv;
	typedef void (FDYNGL_APIENTRY gl_get_named_buffer_sub_data_func)(GLuint buffer, GLintptr offset, GLsizeiptr size, void *data);
	static gl_get_named_buffer_sub_data_func* glGetNamedBufferSubData;
	typedef void (FDYNGL_APIENTRY gl_create_framebuffers_func)(GLsizei n, GLuint *framebuffers);
	static gl_create_framebuffers_func* glCreateFramebuffers;
	typedef void (FDYNGL_APIENTRY gl_named_framebuffer_renderbuffer_func)(GLuint framebuffer, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer);
	static gl_named_framebuffer_renderbuffer_func* glNamedFramebufferRenderbuffer;
	typedef void (FDYNGL_APIENTRY gl_named_framebuffer_parameteri_func)(GLuint framebuffer, GLenum pname, GLint param);
	static gl_named_framebuffer_parameteri_func* glNamedFramebufferParameteri;
	typedef void (FDYNGL_APIENTRY gl_named_framebuffer_texture_func)(GLuint framebuffer, GLenum attachment, GLuint texture, GLint level);
	static gl_named_framebuffer_texture_func* glNamedFramebufferTexture;
	typedef void (FDYNGL_APIENTRY gl_named_framebuffer_texture_layer_func)(GLuint framebuffer, GLenum attachment, GLuint texture, GLint level, GLint layer);
	static gl_named_framebuffer_texture_layer_func* glNamedFramebufferTextureLayer;
	typedef void (FDYNGL_APIENTRY gl_named_framebuffer_draw_buffer_func)(GLuint framebuffer, GLenum buf);
	static gl_named_framebuffer_draw_buffer_func* glNamedFramebufferDrawBuffer;
	typedef void (FDYNGL_APIENTRY gl_named_framebuffer_draw_buffers_func)(GLuint framebuffer, GLsizei n, const GLenum *bufs);
	static gl_named_framebuffer_draw_buffers_func* glNamedFramebufferDrawBuffers;
	typedef void (FDYNGL_APIENTRY gl_named_framebuffer_read_buffer_func)(GLuint framebuffer, GLenum src);
	static gl_named_framebuffer_read_buffer_func* glNamedFramebufferReadBuffer;
	typedef void (FDYNGL_APIENTRY gl_invalidate_named_framebuffer_data_func)(GLuint framebuffer, GLsizei numAttachments, const GLenum *attachments);
	static gl_invalidate_named_framebuffer_data_func* glInvalidateNamedFramebufferData;
	typedef void (FDYNGL_APIENTRY gl_invalidate_named_framebuffer_sub_data_func)(GLuint framebuffer, GLsizei numAttachments, const GLenum *attachments, GLint x, GLint y, GLsizei width, GLsizei height);
	static gl_invalidate_named_framebuffer_sub_data_func* glInvalidateNamedFramebufferSubData;
	typedef void (FDYNGL_APIENTRY gl_clear_named_framebufferiv_func)(GLuint framebuffer, GLenum buffer, GLint drawbuffer, const GLint *value);
	static gl_clear_named_framebufferiv_func* glClearNamedFramebufferiv;
	typedef void (FDYNGL_APIENTRY gl_clear_named_framebufferuiv_func)(GLuint framebuffer, GLenum buffer, GLint drawbuffer, const GLuint *value);
	static gl_clear_named_framebufferuiv_func* glClearNamedFramebufferuiv;
	typedef void (FDYNGL_APIENTRY gl_clear_named_framebufferfv_func)(GLuint framebuffer, GLenum buffer, GLint drawbuffer, const GLfloat *value);
	static gl_clear_named_framebufferfv_func* glClearNamedFramebufferfv;
	typedef void (FDYNGL_APIENTRY gl_clear_named_framebufferfi_func)(GLuint framebuffer, GLenum buffer, GLint drawbuffer, GLfloat depth, GLint stencil);
	static gl_clear_named_framebufferfi_func* glClearNamedFramebufferfi;
	typedef void (FDYNGL_APIENTRY gl_blit_named_framebuffer_func)(GLuint readFramebuffer, GLuint drawFramebuffer, GLint srcX0, GLint srcY0, GLint srcX1, GLint srcY1, GLint dstX0, GLint dstY0, GLint dstX1, GLint dstY1, GLbitfield mask, GLenum filter);
	static gl_blit_named_framebuffer_func* glBlitNamedFramebuffer;
	typedef GLenum(FDYNGL_APIENTRY gl_check_named_framebuffer_status_func)(GLuint framebuffer, GLenum target);
	static gl_check_named_framebuffer_status_func* glCheckNamedFramebufferStatus;
	typedef void (FDYNGL_APIENTRY gl_get_named_framebuffer_parameteriv_func)(GLuint framebuffer, GLenum pname, GLint *param);
	static gl_get_named_framebuffer_parameteriv_func* glGetNamedFramebufferParameteriv;
	typedef void (FDYNGL_APIENTRY gl_get_named_framebuffer_attachment_parameteriv_func)(GLuint framebuffer, GLenum attachment, GLenum pname, GLint *params);
	static gl_get_named_framebuffer_attachment_parameteriv_func* glGetNamedFramebufferAttachmentParameteriv;
	typedef void (FDYNGL_APIENTRY gl_create_renderbuffers_func)(GLsizei n, GLuint *renderbuffers);
	static gl_create_renderbuffers_func* glCreateRenderbuffers;
	typedef void (FDYNGL_APIENTRY gl_named_renderbuffer_storage_func)(GLuint renderbuffer, GLenum internalformat, GLsizei width, GLsizei height);
	static gl_named_renderbuffer_storage_func* glNamedRenderbufferStorage;
	typedef void (FDYNGL_APIENTRY gl_named_renderbuffer_storage_multisample_func)(GLuint renderbuffer, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height);
	static gl_named_renderbuffer_storage_multisample_func* glNamedRenderbufferStorageMultisample;
	typedef void (FDYNGL_APIENTRY gl_get_named_renderbuffer_parameteriv_func)(GLuint renderbuffer, GLenum pname, GLint *params);
	static gl_get_named_renderbuffer_parameteriv_func* glGetNamedRenderbufferParameteriv;
	typedef void (FDYNGL_APIENTRY gl_create_textures_func)(GLenum target, GLsizei n, GLuint *textures);
	static gl_create_textures_func* glCreateTextures;
	typedef void (FDYNGL_APIENTRY gl_texture_buffer_func)(GLuint texture, GLenum internalformat, GLuint buffer);
	static gl_texture_buffer_func* glTextureBuffer;
	typedef void (FDYNGL_APIENTRY gl_texture_buffer_range_func)(GLuint texture, GLenum internalformat, GLuint buffer, GLintptr offset, GLsizeiptr size);
	static gl_texture_buffer_range_func* glTextureBufferRange;
	typedef void (FDYNGL_APIENTRY gl_texture_storage1d_func)(GLuint texture, GLsizei levels, GLenum internalformat, GLsizei width);
	static gl_texture_storage1d_func* glTextureStorage1D;
	typedef void (FDYNGL_APIENTRY gl_texture_storage2d_func)(GLuint texture, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height);
	static gl_texture_storage2d_func* glTextureStorage2D;
	typedef void (FDYNGL_APIENTRY gl_texture_storage3d_func)(GLuint texture, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth);
	static gl_texture_storage3d_func* glTextureStorage3D;
	typedef void (FDYNGL_APIENTRY gl_texture_storage2_d_multisample_func)(GLuint texture, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height, GLboolean fixedsamplelocations);
	static gl_texture_storage2_d_multisample_func* glTextureStorage2DMultisample;
	typedef void (FDYNGL_APIENTRY gl_texture_storage3_d_multisample_func)(GLuint texture, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLboolean fixedsamplelocations);
	static gl_texture_storage3_d_multisample_func* glTextureStorage3DMultisample;
	typedef void (FDYNGL_APIENTRY gl_texture_sub_image1d_func)(GLuint texture, GLint level, GLint xoffset, GLsizei width, GLenum format, GLenum type, const void *pixels);
	static gl_texture_sub_image1d_func* glTextureSubImage1D;
	typedef void (FDYNGL_APIENTRY gl_texture_sub_image2d_func)(GLuint texture, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const void *pixels);
	static gl_texture_sub_image2d_func* glTextureSubImage2D;
	typedef void (FDYNGL_APIENTRY gl_texture_sub_image3d_func)(GLuint texture, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const void *pixels);
	static gl_texture_sub_image3d_func* glTextureSubImage3D;
	typedef void (FDYNGL_APIENTRY gl_compressed_texture_sub_image1d_func)(GLuint texture, GLint level, GLint xoffset, GLsizei width, GLenum format, GLsizei imageSize, const void *data);
	static gl_compressed_texture_sub_image1d_func* glCompressedTextureSubImage1D;
	typedef void (FDYNGL_APIENTRY gl_compressed_texture_sub_image2d_func)(GLuint texture, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLsizei imageSize, const void *data);
	static gl_compressed_texture_sub_image2d_func* glCompressedTextureSubImage2D;
	typedef void (FDYNGL_APIENTRY gl_compressed_texture_sub_image3d_func)(GLuint texture, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLsizei imageSize, const void *data);
	static gl_compressed_texture_sub_image3d_func* glCompressedTextureSubImage3D;
	typedef void (FDYNGL_APIENTRY gl_copy_texture_sub_image1d_func)(GLuint texture, GLint level, GLint xoffset, GLint x, GLint y, GLsizei width);
	static gl_copy_texture_sub_image1d_func* glCopyTextureSubImage1D;
	typedef void (FDYNGL_APIENTRY gl_copy_texture_sub_image2d_func)(GLuint texture, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height);
	static gl_copy_texture_sub_image2d_func* glCopyTextureSubImage2D;
	typedef void (FDYNGL_APIENTRY gl_copy_texture_sub_image3d_func)(GLuint texture, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLint x, GLint y, GLsizei width, GLsizei height);
	static gl_copy_texture_sub_image3d_func* glCopyTextureSubImage3D;
	typedef void (FDYNGL_APIENTRY gl_texture_parameterf_func)(GLuint texture, GLenum pname, GLfloat param);
	static gl_texture_parameterf_func* glTextureParameterf;
	typedef void (FDYNGL_APIENTRY gl_texture_parameterfv_func)(GLuint texture, GLenum pname, const GLfloat *param);
	static gl_texture_parameterfv_func* glTextureParameterfv;
	typedef void (FDYNGL_APIENTRY gl_texture_parameteri_func)(GLuint texture, GLenum pname, GLint param);
	static gl_texture_parameteri_func* glTextureParameteri;
	typedef void (FDYNGL_APIENTRY gl_texture_parameter_iiv_func)(GLuint texture, GLenum pname, const GLint *params);
	static gl_texture_parameter_iiv_func* glTextureParameterIiv;
	typedef void (FDYNGL_APIENTRY gl_texture_parameter_iuiv_func)(GLuint texture, GLenum pname, const GLuint *params);
	static gl_texture_parameter_iuiv_func* glTextureParameterIuiv;
	typedef void (FDYNGL_APIENTRY gl_texture_parameteriv_func)(GLuint texture, GLenum pname, const GLint *param);
	static gl_texture_parameteriv_func* glTextureParameteriv;
	typedef void (FDYNGL_APIENTRY gl_generate_texture_mipmap_func)(GLuint texture);
	static gl_generate_texture_mipmap_func* glGenerateTextureMipmap;
	typedef void (FDYNGL_APIENTRY gl_bind_texture_unit_func)(GLuint unit, GLuint texture);
	static gl_bind_texture_unit_func* glBindTextureUnit;
	typedef void (FDYNGL_APIENTRY gl_get_texture_image_func)(GLuint texture, GLint level, GLenum format, GLenum type, GLsizei bufSize, void *pixels);
	static gl_get_texture_image_func* glGetTextureImage;
	typedef void (FDYNGL_APIENTRY gl_get_compressed_texture_image_func)(GLuint texture, GLint level, GLsizei bufSize, void *pixels);
	static gl_get_compressed_texture_image_func* glGetCompressedTextureImage;
	typedef void (FDYNGL_APIENTRY gl_get_texture_level_parameterfv_func)(GLuint texture, GLint level, GLenum pname, GLfloat *params);
	static gl_get_texture_level_parameterfv_func* glGetTextureLevelParameterfv;
	typedef void (FDYNGL_APIENTRY gl_get_texture_level_parameteriv_func)(GLuint texture, GLint level, GLenum pname, GLint *params);
	static gl_get_texture_level_parameteriv_func* glGetTextureLevelParameteriv;
	typedef void (FDYNGL_APIENTRY gl_get_texture_parameterfv_func)(GLuint texture, GLenum pname, GLfloat *params);
	static gl_get_texture_parameterfv_func* glGetTextureParameterfv;
	typedef void (FDYNGL_APIENTRY gl_get_texture_parameter_iiv_func)(GLuint texture, GLenum pname, GLint *params);
	static gl_get_texture_parameter_iiv_func* glGetTextureParameterIiv;
	typedef void (FDYNGL_APIENTRY gl_get_texture_parameter_iuiv_func)(GLuint texture, GLenum pname, GLuint *params);
	static gl_get_texture_parameter_iuiv_func* glGetTextureParameterIuiv;
	typedef void (FDYNGL_APIENTRY gl_get_texture_parameteriv_func)(GLuint texture, GLenum pname, GLint *params);
	static gl_get_texture_parameteriv_func* glGetTextureParameteriv;
	typedef void (FDYNGL_APIENTRY gl_create_vertex_arrays_func)(GLsizei n, GLuint *arrays);
	static gl_create_vertex_arrays_func* glCreateVertexArrays;
	typedef void (FDYNGL_APIENTRY gl_disable_vertex_array_attrib_func)(GLuint vaobj, GLuint index);
	static gl_disable_vertex_array_attrib_func* glDisableVertexArrayAttrib;
	typedef void (FDYNGL_APIENTRY gl_enable_vertex_array_attrib_func)(GLuint vaobj, GLuint index);
	static gl_enable_vertex_array_attrib_func* glEnableVertexArrayAttrib;
	typedef void (FDYNGL_APIENTRY gl_vertex_array_element_buffer_func)(GLuint vaobj, GLuint buffer);
	static gl_vertex_array_element_buffer_func* glVertexArrayElementBuffer;
	typedef void (FDYNGL_APIENTRY gl_vertex_array_vertex_buffer_func)(GLuint vaobj, GLuint bindingindex, GLuint buffer, GLintptr offset, GLsizei stride);
	static gl_vertex_array_vertex_buffer_func* glVertexArrayVertexBuffer;
	typedef void (FDYNGL_APIENTRY gl_vertex_array_vertex_buffers_func)(GLuint vaobj, GLuint first, GLsizei count, const GLuint *buffers, const GLintptr *offsets, const GLsizei *strides);
	static gl_vertex_array_vertex_buffers_func* glVertexArrayVertexBuffers;
	typedef void (FDYNGL_APIENTRY gl_vertex_array_attrib_binding_func)(GLuint vaobj, GLuint attribindex, GLuint bindingindex);
	static gl_vertex_array_attrib_binding_func* glVertexArrayAttribBinding;
	typedef void (FDYNGL_APIENTRY gl_vertex_array_attrib_format_func)(GLuint vaobj, GLuint attribindex, GLint size, GLenum type, GLboolean normalized, GLuint relativeoffset);
	static gl_vertex_array_attrib_format_func* glVertexArrayAttribFormat;
	typedef void (FDYNGL_APIENTRY gl_vertex_array_attrib_i_format_func)(GLuint vaobj, GLuint attribindex, GLint size, GLenum type, GLuint relativeoffset);
	static gl_vertex_array_attrib_i_format_func* glVertexArrayAttribIFormat;
	typedef void (FDYNGL_APIENTRY gl_vertex_array_attrib_l_format_func)(GLuint vaobj, GLuint attribindex, GLint size, GLenum type, GLuint relativeoffset);
	static gl_vertex_array_attrib_l_format_func* glVertexArrayAttribLFormat;
	typedef void (FDYNGL_APIENTRY gl_vertex_array_binding_divisor_func)(GLuint vaobj, GLuint bindingindex, GLuint divisor);
	static gl_vertex_array_binding_divisor_func* glVertexArrayBindingDivisor;
	typedef void (FDYNGL_APIENTRY gl_get_vertex_arrayiv_func)(GLuint vaobj, GLenum pname, GLint *param);
	static gl_get_vertex_arrayiv_func* glGetVertexArrayiv;
	typedef void (FDYNGL_APIENTRY gl_get_vertex_array_indexediv_func)(GLuint vaobj, GLuint index, GLenum pname, GLint *param);
	static gl_get_vertex_array_indexediv_func* glGetVertexArrayIndexediv;
	typedef void (FDYNGL_APIENTRY gl_get_vertex_array_indexed64iv_func)(GLuint vaobj, GLuint index, GLenum pname, GLint64 *param);
	static gl_get_vertex_array_indexed64iv_func* glGetVertexArrayIndexed64iv;
	typedef void (FDYNGL_APIENTRY gl_create_samplers_func)(GLsizei n, GLuint *samplers);
	static gl_create_samplers_func* glCreateSamplers;
	typedef void (FDYNGL_APIENTRY gl_create_program_pipelines_func)(GLsizei n, GLuint *pipelines);
	static gl_create_program_pipelines_func* glCreateProgramPipelines;
	typedef void (FDYNGL_APIENTRY gl_create_queries_func)(GLenum target, GLsizei n, GLuint *ids);
	static gl_create_queries_func* glCreateQueries;
	typedef void (FDYNGL_APIENTRY gl_get_query_buffer_objecti64v_func)(GLuint id, GLuint buffer, GLenum pname, GLintptr offset);
	static gl_get_query_buffer_objecti64v_func* glGetQueryBufferObjecti64v;
	typedef void (FDYNGL_APIENTRY gl_get_query_buffer_objectiv_func)(GLuint id, GLuint buffer, GLenum pname, GLintptr offset);
	static gl_get_query_buffer_objectiv_func* glGetQueryBufferObjectiv;
	typedef void (FDYNGL_APIENTRY gl_get_query_buffer_objectui64v_func)(GLuint id, GLuint buffer, GLenum pname, GLintptr offset);
	static gl_get_query_buffer_objectui64v_func* glGetQueryBufferObjectui64v;
	typedef void (FDYNGL_APIENTRY gl_get_query_buffer_objectuiv_func)(GLuint id, GLuint buffer, GLenum pname, GLintptr offset);
	static gl_get_query_buffer_objectuiv_func* glGetQueryBufferObjectuiv;
	typedef void (FDYNGL_APIENTRY gl_memory_barrier_by_region_func)(GLbitfield barriers);
	static gl_memory_barrier_by_region_func* glMemoryBarrierByRegion;
	typedef void (FDYNGL_APIENTRY gl_get_texture_sub_image_func)(GLuint texture, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, GLsizei bufSize, void *pixels);
	static gl_get_texture_sub_image_func* glGetTextureSubImage;
	typedef void (FDYNGL_APIENTRY gl_get_compressed_texture_sub_image_func)(GLuint texture, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLsizei bufSize, void *pixels);
	static gl_get_compressed_texture_sub_image_func* glGetCompressedTextureSubImage;
	typedef GLenum(FDYNGL_APIENTRY gl_get_graphics_reset_status_func)(void);
	static gl_get_graphics_reset_status_func* glGetGraphicsResetStatus;
	typedef void (FDYNGL_APIENTRY gl_getn_compressed_tex_image_func)(GLenum target, GLint lod, GLsizei bufSize, void *pixels);
	static gl_getn_compressed_tex_image_func* glGetnCompressedTexImage;
	typedef void (FDYNGL_APIENTRY gl_getn_tex_image_func)(GLenum target, GLint level, GLenum format, GLenum type, GLsizei bufSize, void *pixels);
	static gl_getn_tex_image_func* glGetnTexImage;
	typedef void (FDYNGL_APIENTRY gl_getn_uniformdv_func)(GLuint program, GLint location, GLsizei bufSize, GLdouble *params);
	static gl_getn_uniformdv_func* glGetnUniformdv;
	typedef void (FDYNGL_APIENTRY gl_getn_uniformfv_func)(GLuint program, GLint location, GLsizei bufSize, GLfloat *params);
	static gl_getn_uniformfv_func* glGetnUniformfv;
	typedef void (FDYNGL_APIENTRY gl_getn_uniformiv_func)(GLuint program, GLint location, GLsizei bufSize, GLint *params);
	static gl_getn_uniformiv_func* glGetnUniformiv;
	typedef void (FDYNGL_APIENTRY gl_getn_uniformuiv_func)(GLuint program, GLint location, GLsizei bufSize, GLuint *params);
	static gl_getn_uniformuiv_func* glGetnUniformuiv;
	typedef void (FDYNGL_APIENTRY gl_readn_pixels_func)(GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, GLsizei bufSize, void *data);
	static gl_readn_pixels_func* glReadnPixels;
	typedef void (FDYNGL_APIENTRY gl_getn_mapdv_func)(GLenum target, GLenum query, GLsizei bufSize, GLdouble *v);
	static gl_getn_mapdv_func* glGetnMapdv;
	typedef void (FDYNGL_APIENTRY gl_getn_mapfv_func)(GLenum target, GLenum query, GLsizei bufSize, GLfloat *v);
	static gl_getn_mapfv_func* glGetnMapfv;
	typedef void (FDYNGL_APIENTRY gl_getn_mapiv_func)(GLenum target, GLenum query, GLsizei bufSize, GLint *v);
	static gl_getn_mapiv_func* glGetnMapiv;
	typedef void (FDYNGL_APIENTRY gl_getn_pixel_mapfv_func)(GLenum map, GLsizei bufSize, GLfloat *values);
	static gl_getn_pixel_mapfv_func* glGetnPixelMapfv;
	typedef void (FDYNGL_APIENTRY gl_getn_pixel_mapuiv_func)(GLenum map, GLsizei bufSize, GLuint *values);
	static gl_getn_pixel_mapuiv_func* glGetnPixelMapuiv;
	typedef void (FDYNGL_APIENTRY gl_getn_pixel_mapusv_func)(GLenum map, GLsizei bufSize, GLushort *values);
	static gl_getn_pixel_mapusv_func* glGetnPixelMapusv;
	typedef void (FDYNGL_APIENTRY gl_getn_polygon_stipple_func)(GLsizei bufSize, GLubyte *pattern);
	static gl_getn_polygon_stipple_func* glGetnPolygonStipple;
	typedef void (FDYNGL_APIENTRY gl_getn_color_table_func)(GLenum target, GLenum format, GLenum type, GLsizei bufSize, void *table);
	static gl_getn_color_table_func* glGetnColorTable;
	typedef void (FDYNGL_APIENTRY gl_getn_convolution_filter_func)(GLenum target, GLenum format, GLenum type, GLsizei bufSize, void *image);
	static gl_getn_convolution_filter_func* glGetnConvolutionFilter;
	typedef void (FDYNGL_APIENTRY gl_getn_separable_filter_func)(GLenum target, GLenum format, GLenum type, GLsizei rowBufSize, void *row, GLsizei columnBufSize, void *column, void *span);
	static gl_getn_separable_filter_func* glGetnSeparableFilter;
	typedef void (FDYNGL_APIENTRY gl_getn_histogram_func)(GLenum target, GLboolean reset, GLenum format, GLenum type, GLsizei bufSize, void *values);
	static gl_getn_histogram_func* glGetnHistogram;
	typedef void (FDYNGL_APIENTRY gl_getn_minmax_func)(GLenum target, GLboolean reset, GLenum format, GLenum type, GLsizei bufSize, void *values);
	static gl_getn_minmax_func* glGetnMinmax;
	typedef void (FDYNGL_APIENTRY gl_texture_barrier_func)(void);
	static gl_texture_barrier_func* glTextureBarrier;
#	endif // GL_VERSION_4_5
#	ifndef GL_VERSION_4_6
#		define GL_VERSION_4_6 1
	static bool isGL_VERSION_4_6;
	static const uint32_t GL_SHADER_BINARY_FORMAT_SPIR_V = 0x9551;
	static const uint32_t GL_SPIR_V_BINARY = 0x9552;
	static const uint32_t GL_PARAMETER_BUFFER = 0x80EE;
	static const uint32_t GL_PARAMETER_BUFFER_BINDING = 0x80EF;
	static const uint32_t GL_CONTEXT_FLAG_NO_ERROR_BIT = 0x00000008;
	static const uint32_t GL_VERTICES_SUBMITTED = 0x82EE;
	static const uint32_t GL_PRIMITIVES_SUBMITTED = 0x82EF;
	static const uint32_t GL_VERTEX_SHADER_INVOCATIONS = 0x82F0;
	static const uint32_t GL_TESS_CONTROL_SHADER_PATCHES = 0x82F1;
	static const uint32_t GL_TESS_EVALUATION_SHADER_INVOCATIONS = 0x82F2;
	static const uint32_t GL_GEOMETRY_SHADER_PRIMITIVES_EMITTED = 0x82F3;
	static const uint32_t GL_FRAGMENT_SHADER_INVOCATIONS = 0x82F4;
	static const uint32_t GL_COMPUTE_SHADER_INVOCATIONS = 0x82F5;
	static const uint32_t GL_CLIPPING_INPUT_PRIMITIVES = 0x82F6;
	static const uint32_t GL_CLIPPING_OUTPUT_PRIMITIVES = 0x82F7;
	static const uint32_t GL_POLYGON_OFFSET_CLAMP = 0x8E1B;
	static const uint32_t GL_SPIR_V_EXTENSIONS = 0x9553;
	static const uint32_t GL_NUM_SPIR_V_EXTENSIONS = 0x9554;
	static const uint32_t GL_TEXTURE_MAX_ANISOTROPY = 0x84FE;
	static const uint32_t GL_MAX_TEXTURE_MAX_ANISOTROPY = 0x84FF;
	static const uint32_t GL_TRANSFORM_FEEDBACK_OVERFLOW = 0x82EC;
	static const uint32_t GL_TRANSFORM_FEEDBACK_STREAM_OVERFLOW = 0x82ED;
	typedef void (FDYNGL_APIENTRY gl_specialize_shader_func)(GLuint shader, const GLchar *pEntryPoint, GLuint numSpecializationConstants, const GLuint *pConstantIndex, const GLuint *pConstantValue);
	static gl_specialize_shader_func* glSpecializeShader;
	typedef void (FDYNGL_APIENTRY gl_multi_draw_arrays_indirect_count_func)(GLenum mode, const void *indirect, GLintptr drawcount, GLsizei maxdrawcount, GLsizei stride);
	static gl_multi_draw_arrays_indirect_count_func* glMultiDrawArraysIndirectCount;
	typedef void (FDYNGL_APIENTRY gl_multi_draw_elements_indirect_count_func)(GLenum mode, GLenum type, const void *indirect, GLintptr drawcount, GLsizei maxdrawcount, GLsizei stride);
	static gl_multi_draw_elements_indirect_count_func* glMultiDrawElementsIndirectCount;
	typedef void (FDYNGL_APIENTRY gl_polygon_offset_clamp_func)(GLfloat factor, GLfloat units, GLfloat clamp);
	static gl_polygon_offset_clamp_func* glPolygonOffsetClamp;
#	endif // GL_VERSION_4_6

#ifdef __cplusplus
}
#endif

#endif // FDYNGL_INCLUDE_HPP

// ****************************************************************************
//
// Implementation
//
// ****************************************************************************
#if defined(FDYNGL_IMPLEMENTATION) && !defined(FDYNGL_IMPLEMENTED)
#	define FDYNGL_IMPLEMENTED

#define FDYNGL_ARRAYCOUNT(arr) (sizeof(arr) / sizeof((arr)[0]))

#include <string.h> // strcpy, strlen
#include <assert.h> // assert
#include <stdarg.h> // va_start, va_end
#include <stdio.h> // vsnprintf

namespace fdyngl {
#if defined(FDYNGL_PLATFORM_WIN32)
	// User32.dll
#	define FDYNGL_FUNC_RELEASE_DC(name) int WINAPI name(HWND hWnd, HDC hDC)
	typedef FDYNGL_FUNC_RELEASE_DC(win32_func_ReleaseDC);
#	define FDYNGL_FUNC_GET_DC(name) HDC WINAPI name(HWND hWnd)
	typedef FDYNGL_FUNC_GET_DC(win32_func_GetDC);

	// Gdi32.dll
#	define FDYNGL_FUNC_CHOOSE_PIXEL_FORMAT(name) int WINAPI name(HDC hdc, CONST PIXELFORMATDESCRIPTOR *ppfd)
	typedef FDYNGL_FUNC_CHOOSE_PIXEL_FORMAT(win32_func_ChoosePixelFormat);
#	define FDYNGL_FUNC_SET_PIXEL_FORMAT(name) BOOL WINAPI name(HDC hdc, int format, CONST PIXELFORMATDESCRIPTOR *ppfd)
	typedef FDYNGL_FUNC_SET_PIXEL_FORMAT(win32_func_SetPixelFormat);
#	define FDYNGL_FUNC_DESCRIPE_PIXEL_FORMAT(name) int WINAPI name(HDC hdc, int iPixelFormat, UINT nBytes, LPPIXELFORMATDESCRIPTOR ppfd)
	typedef FDYNGL_FUNC_DESCRIPE_PIXEL_FORMAT(win32_func_DescribePixelFormat);
#	define FDYNGL_FUNC_SWAP_BUFFERS(name) BOOL WINAPI name(HDC)
	typedef FDYNGL_FUNC_SWAP_BUFFERS(win32_func_SwapBuffers);

	// OpenGL32.dll
#	define FDYNGL_GL_CONTEXT_FLAG_FORWARD_COMPATIBLE_BIT 0x0001
#	define FDYNGL_GL_CONTEXT_FLAG_DEBUG_BIT 0x00000002
#	define FDYNGL_GL_CONTEXT_FLAG_ROBUST_ACCESS_BIT 0x00000004
#	define FDYNGL_GL_CONTEXT_FLAG_NO_ERROR_BIT 0x00000008
#	define FDYNGL_GL_CONTEXT_CORE_PROFILE_BIT 0x00000001
#	define FDYNGL_GL_CONTEXT_COMPATIBILITY_PROFILE_BIT 0x00000002

#	define FDYNGL_WGL_CONTEXT_DEBUG_BIT_ARB 0x0001
#	define FDYNGL_WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB 0x0002
#	define FDYNGL_WGL_CONTEXT_CORE_PROFILE_BIT_ARB 0x00000001
#	define FDYNGL_WGL_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB 0x00000002
#	define FDYNGL_WGL_CONTEXT_PROFILE_MASK_ARB 0x9126
#	define FDYNGL_WGL_CONTEXT_MAJOR_VERSION_ARB 0x2091
#	define FDYNGL_WGL_CONTEXT_MINOR_VERSION_ARB 0x2092
#	define FDYNGL_WGL_CONTEXT_LAYER_PLANE_ARB 0x2093
#	define FDYNGL_WGL_CONTEXT_FLAGS_ARB 0x2094

#	define FDYNGL_WGL_DRAW_TO_WINDOW_ARB 0x2001
#	define FDYNGL_WGL_ACCELERATION_ARB 0x2003
#	define FDYNGL_WGL_SWAP_METHOD_ARB 0x2007
#	define FDYNGL_WGL_SUPPORT_OPENGL_ARB 0x2010
#	define FDYNGL_WGL_DOUBLE_BUFFER_ARB 0x2011
#	define FDYNGL_WGL_PIXEL_TYPE_ARB 0x2013
#	define FDYNGL_WGL_COLOR_BITS_ARB 0x2014
#	define FDYNGL_WGL_DEPTH_BITS_ARB 0x2022
#	define FDYNGL_WGL_STENCIL_BITS_ARB 0x2023
#	define FDYNGL_WGL_FULL_ACCELERATION_ARB 0x2027
#	define FDYNGL_WGL_SWAP_EXCHANGE_ARB 0x2028
#	define FDYNGL_WGL_TYPE_RGBA_ARB 0x202B

#	define FDYNGL_FUNC_WGL_MAKE_CURRENT(name) BOOL WINAPI name(HDC deviceContext, HGLRC renderingContext)
	typedef FDYNGL_FUNC_WGL_MAKE_CURRENT(win32_func_wglMakeCurrent);
#	define FDYNGL_FUNC_WGL_GET_PROC_ADDRESS(name) PROC WINAPI name(LPCSTR procedure)
	typedef FDYNGL_FUNC_WGL_GET_PROC_ADDRESS(win32_func_wglGetProcAddress);
#	define FDYNGL_FUNC_WGL_DELETE_CONTEXT(name) BOOL WINAPI name(HGLRC renderingContext)
	typedef FDYNGL_FUNC_WGL_DELETE_CONTEXT(win32_func_wglDeleteContext);
#	define FDYNGL_FUNC_WGL_CREATE_CONTEXT(name) HGLRC WINAPI name(HDC deviceContext)
	typedef FDYNGL_FUNC_WGL_CREATE_CONTEXT(win32_func_wglCreateContext);

#	define FDYNGL_FUNC_WGL_CHOOSE_PIXEL_FORMAT_ARB(name) BOOL WINAPI name(HDC hdc, const int *piAttribIList, const FLOAT *pfAttribFList, UINT nMaxFormats, int *piFormats, UINT *nNumFormats)
	typedef FDYNGL_FUNC_WGL_CHOOSE_PIXEL_FORMAT_ARB(win32_func_wglChoosePixelFormatARB);
#	define FDYNGL_FUNC_WGL_CREATE_CONTEXT_ATTRIBS_ARB(name) HGLRC WINAPI name(HDC hDC, HGLRC hShareContext, const int *attribList)
	typedef FDYNGL_FUNC_WGL_CREATE_CONTEXT_ATTRIBS_ARB(win32_func_wglCreateContextAttribsARB);
#	define FDYNGL_FUNC_WGL_SWAP_INTERVAL_EXT(name) BOOL WINAPI name(int interval)
	typedef FDYNGL_FUNC_WGL_SWAP_INTERVAL_EXT(win32_func_wglSwapIntervalEXT);

#elif defined(FDYNGL_SUBPLATFORM_GLX)
#	define FDYNGL_FUNC_GLX_GET_PROC_ADDRESS(name) void *name(const char *name)
	typedef FDYNGL_FUNC_GLX_GET_PROC_ADDRESS(glx_func_glXGetProcAddress);
#endif

	struct OpenGLState {
		union {
#		if defined(FDYNGL_PLATFORM_WIN32)
			struct {
				struct {
					HMODULE libraryHandle;
					win32_func_GetDC *GetDC;
					win32_func_ReleaseDC *ReleaseDC;
				} user32;

				struct {
					HMODULE libraryHandle;
					win32_func_ChoosePixelFormat *ChoosePixelFormat;
					win32_func_SetPixelFormat *SetPixelFormat;
					win32_func_DescribePixelFormat *DescribePixelFormat;
					win32_func_SwapBuffers *SwapBuffers;
				} gdi32;

				struct {
					HMODULE libraryHandle;
					win32_func_wglMakeCurrent *wglMakeCurrent;
					win32_func_wglGetProcAddress *wglGetProcAddress;
					win32_func_wglDeleteContext *wglDeleteContext;
					win32_func_wglCreateContext *wglCreateContext;
					win32_func_wglChoosePixelFormatARB *wglChoosePixelFormatArb;
					win32_func_wglCreateContextAttribsARB *wglCreateContextAttribsArb;
					win32_func_wglSwapIntervalEXT *wglSwapIntervalExt;
				} opengl32;
			} win32;
#		elif defined(FDYNGL_PLATFORM_POSIX) && defined(FDYNGL_PLATFORM_GLX)
			struct {
				void *libraryHandle;
				glx_func_glXGetProcAddress *glXGetProcAddress;
			} posix_glx;
#		endif
		};
		char lastError[256];
		bool isLoaded;
	};

	namespace platform {
		static void SetLastError_Internal(OpenGLState &state, const char *format, ...) {
			if (format != nullptr) {
				va_list argList;
				va_start(argList, format);
				vsnprintf(state.lastError, FDYNGL_ARRAYCOUNT(state.lastError), format, argList);
				va_end(argList);
			} else {
				memset(state.lastError, 0, sizeof(state.lastError));
			}
		}

		static void *GetOpenGLProcAddress_Internal(const OpenGLState &state, const char *name) {
			void *result = nullptr;
#		if defined(FDYNGL_PLATFORM_WIN32)
			result = GetProcAddress(state.win32.opengl32.libraryHandle, name);
			if (result == nullptr) {
				result = state.win32.opengl32.wglGetProcAddress(name);
			}
#		elif defined(FDYNGL_PLATFORM_POSIX) && defined(FDYNGL_PLATFORM_GLX)
			result = dlsym(state.posix_glx.libraryHandle, name);
			if (result == nullptr) {
				result = state.posix_glx.glXGetProcAddress(name);
			}
#		endif
			return(result);
		}

		static void LoadOpenGLExtensions_Internal(const OpenGLState &state) {
#	if GL_VERSION_1_1
			glAccum = (gl_accum_func *)GetOpenGLProcAddress_Internal(state, "glAccum");
			glAlphaFunc = (gl_alpha_func_func *)GetOpenGLProcAddress_Internal(state, "glAlphaFunc");
			glAreTexturesResident = (gl_are_textures_resident_func *)GetOpenGLProcAddress_Internal(state, "glAreTexturesResident");
			glArrayElement = (gl_array_element_func *)GetOpenGLProcAddress_Internal(state, "glArrayElement");
			glBegin = (gl_begin_func *)GetOpenGLProcAddress_Internal(state, "glBegin");
			glBindTexture = (gl_bind_texture_func *)GetOpenGLProcAddress_Internal(state, "glBindTexture");
			glBitmap = (gl_bitmap_func *)GetOpenGLProcAddress_Internal(state, "glBitmap");
			glBlendFunc = (gl_blend_func_func *)GetOpenGLProcAddress_Internal(state, "glBlendFunc");
			glCallList = (gl_call_list_func *)GetOpenGLProcAddress_Internal(state, "glCallList");
			glCallLists = (gl_call_lists_func *)GetOpenGLProcAddress_Internal(state, "glCallLists");
			glClear = (gl_clear_func *)GetOpenGLProcAddress_Internal(state, "glClear");
			glClearAccum = (gl_clear_accum_func *)GetOpenGLProcAddress_Internal(state, "glClearAccum");
			glClearColor = (gl_clear_color_func *)GetOpenGLProcAddress_Internal(state, "glClearColor");
			glClearDepth = (gl_clear_depth_func *)GetOpenGLProcAddress_Internal(state, "glClearDepth");
			glClearIndex = (gl_clear_index_func *)GetOpenGLProcAddress_Internal(state, "glClearIndex");
			glClearStencil = (gl_clear_stencil_func *)GetOpenGLProcAddress_Internal(state, "glClearStencil");
			glClipPlane = (gl_clip_plane_func *)GetOpenGLProcAddress_Internal(state, "glClipPlane");
			glColor3b = (gl_color3b_func *)GetOpenGLProcAddress_Internal(state, "glColor3b");
			glColor3bv = (gl_color3bv_func *)GetOpenGLProcAddress_Internal(state, "glColor3bv");
			glColor3d = (gl_color3d_func *)GetOpenGLProcAddress_Internal(state, "glColor3d");
			glColor3dv = (gl_color3dv_func *)GetOpenGLProcAddress_Internal(state, "glColor3dv");
			glColor3f = (gl_color3f_func *)GetOpenGLProcAddress_Internal(state, "glColor3f");
			glColor3fv = (gl_color3fv_func *)GetOpenGLProcAddress_Internal(state, "glColor3fv");
			glColor3i = (gl_color3i_func *)GetOpenGLProcAddress_Internal(state, "glColor3i");
			glColor3iv = (gl_color3iv_func *)GetOpenGLProcAddress_Internal(state, "glColor3iv");
			glColor3s = (gl_color3s_func *)GetOpenGLProcAddress_Internal(state, "glColor3s");
			glColor3sv = (gl_color3sv_func *)GetOpenGLProcAddress_Internal(state, "glColor3sv");
			glColor3ub = (gl_color3ub_func *)GetOpenGLProcAddress_Internal(state, "glColor3ub");
			glColor3ubv = (gl_color3ubv_func *)GetOpenGLProcAddress_Internal(state, "glColor3ubv");
			glColor3ui = (gl_color3ui_func *)GetOpenGLProcAddress_Internal(state, "glColor3ui");
			glColor3uiv = (gl_color3uiv_func *)GetOpenGLProcAddress_Internal(state, "glColor3uiv");
			glColor3us = (gl_color3us_func *)GetOpenGLProcAddress_Internal(state, "glColor3us");
			glColor3usv = (gl_color3usv_func *)GetOpenGLProcAddress_Internal(state, "glColor3usv");
			glColor4b = (gl_color4b_func *)GetOpenGLProcAddress_Internal(state, "glColor4b");
			glColor4bv = (gl_color4bv_func *)GetOpenGLProcAddress_Internal(state, "glColor4bv");
			glColor4d = (gl_color4d_func *)GetOpenGLProcAddress_Internal(state, "glColor4d");
			glColor4dv = (gl_color4dv_func *)GetOpenGLProcAddress_Internal(state, "glColor4dv");
			glColor4f = (gl_color4f_func *)GetOpenGLProcAddress_Internal(state, "glColor4f");
			glColor4fv = (gl_color4fv_func *)GetOpenGLProcAddress_Internal(state, "glColor4fv");
			glColor4i = (gl_color4i_func *)GetOpenGLProcAddress_Internal(state, "glColor4i");
			glColor4iv = (gl_color4iv_func *)GetOpenGLProcAddress_Internal(state, "glColor4iv");
			glColor4s = (gl_color4s_func *)GetOpenGLProcAddress_Internal(state, "glColor4s");
			glColor4sv = (gl_color4sv_func *)GetOpenGLProcAddress_Internal(state, "glColor4sv");
			glColor4ub = (gl_color4ub_func *)GetOpenGLProcAddress_Internal(state, "glColor4ub");
			glColor4ubv = (gl_color4ubv_func *)GetOpenGLProcAddress_Internal(state, "glColor4ubv");
			glColor4ui = (gl_color4ui_func *)GetOpenGLProcAddress_Internal(state, "glColor4ui");
			glColor4uiv = (gl_color4uiv_func *)GetOpenGLProcAddress_Internal(state, "glColor4uiv");
			glColor4us = (gl_color4us_func *)GetOpenGLProcAddress_Internal(state, "glColor4us");
			glColor4usv = (gl_color4usv_func *)GetOpenGLProcAddress_Internal(state, "glColor4usv");
			glColorMask = (gl_color_mask_func *)GetOpenGLProcAddress_Internal(state, "glColorMask");
			glColorMaterial = (gl_color_material_func *)GetOpenGLProcAddress_Internal(state, "glColorMaterial");
			glColorPointer = (gl_color_pointer_func *)GetOpenGLProcAddress_Internal(state, "glColorPointer");
			glCopyPixels = (gl_copy_pixels_func *)GetOpenGLProcAddress_Internal(state, "glCopyPixels");
			glCopyTexImage1D = (gl_copy_tex_image1d_func *)GetOpenGLProcAddress_Internal(state, "glCopyTexImage1D");
			glCopyTexImage2D = (gl_copy_tex_image2d_func *)GetOpenGLProcAddress_Internal(state, "glCopyTexImage2D");
			glCopyTexSubImage1D = (gl_copy_tex_sub_image1d_func *)GetOpenGLProcAddress_Internal(state, "glCopyTexSubImage1D");
			glCopyTexSubImage2D = (gl_copy_tex_sub_image2d_func *)GetOpenGLProcAddress_Internal(state, "glCopyTexSubImage2D");
			glCullFace = (gl_cull_face_func *)GetOpenGLProcAddress_Internal(state, "glCullFace");
			glDeleteLists = (gl_delete_lists_func *)GetOpenGLProcAddress_Internal(state, "glDeleteLists");
			glDeleteTextures = (gl_delete_textures_func *)GetOpenGLProcAddress_Internal(state, "glDeleteTextures");
			glDepthFunc = (gl_depth_func_func *)GetOpenGLProcAddress_Internal(state, "glDepthFunc");
			glDepthMask = (gl_depth_mask_func *)GetOpenGLProcAddress_Internal(state, "glDepthMask");
			glDepthRange = (gl_depth_range_func *)GetOpenGLProcAddress_Internal(state, "glDepthRange");
			glDisable = (gl_disable_func *)GetOpenGLProcAddress_Internal(state, "glDisable");
			glDisableClientState = (gl_disable_client_state_func *)GetOpenGLProcAddress_Internal(state, "glDisableClientState");
			glDrawArrays = (gl_draw_arrays_func *)GetOpenGLProcAddress_Internal(state, "glDrawArrays");
			glDrawBuffer = (gl_draw_buffer_func *)GetOpenGLProcAddress_Internal(state, "glDrawBuffer");
			glDrawElements = (gl_draw_elements_func *)GetOpenGLProcAddress_Internal(state, "glDrawElements");
			glDrawPixels = (gl_draw_pixels_func *)GetOpenGLProcAddress_Internal(state, "glDrawPixels");
			glEdgeFlag = (gl_edge_flag_func *)GetOpenGLProcAddress_Internal(state, "glEdgeFlag");
			glEdgeFlagPointer = (gl_edge_flag_pointer_func *)GetOpenGLProcAddress_Internal(state, "glEdgeFlagPointer");
			glEdgeFlagv = (gl_edge_flagv_func *)GetOpenGLProcAddress_Internal(state, "glEdgeFlagv");
			glEnable = (gl_enable_func *)GetOpenGLProcAddress_Internal(state, "glEnable");
			glEnableClientState = (gl_enable_client_state_func *)GetOpenGLProcAddress_Internal(state, "glEnableClientState");
			glEnd = (gl_end_func *)GetOpenGLProcAddress_Internal(state, "glEnd");
			glEndList = (gl_end_list_func *)GetOpenGLProcAddress_Internal(state, "glEndList");
			glEvalCoord1d = (gl_eval_coord1d_func *)GetOpenGLProcAddress_Internal(state, "glEvalCoord1d");
			glEvalCoord1dv = (gl_eval_coord1dv_func *)GetOpenGLProcAddress_Internal(state, "glEvalCoord1dv");
			glEvalCoord1f = (gl_eval_coord1f_func *)GetOpenGLProcAddress_Internal(state, "glEvalCoord1f");
			glEvalCoord1fv = (gl_eval_coord1fv_func *)GetOpenGLProcAddress_Internal(state, "glEvalCoord1fv");
			glEvalCoord2d = (gl_eval_coord2d_func *)GetOpenGLProcAddress_Internal(state, "glEvalCoord2d");
			glEvalCoord2dv = (gl_eval_coord2dv_func *)GetOpenGLProcAddress_Internal(state, "glEvalCoord2dv");
			glEvalCoord2f = (gl_eval_coord2f_func *)GetOpenGLProcAddress_Internal(state, "glEvalCoord2f");
			glEvalCoord2fv = (gl_eval_coord2fv_func *)GetOpenGLProcAddress_Internal(state, "glEvalCoord2fv");
			glEvalMesh1 = (gl_eval_mesh1_func *)GetOpenGLProcAddress_Internal(state, "glEvalMesh1");
			glEvalMesh2 = (gl_eval_mesh2_func *)GetOpenGLProcAddress_Internal(state, "glEvalMesh2");
			glEvalPoint1 = (gl_eval_point1_func *)GetOpenGLProcAddress_Internal(state, "glEvalPoint1");
			glEvalPoint2 = (gl_eval_point2_func *)GetOpenGLProcAddress_Internal(state, "glEvalPoint2");
			glFeedbackBuffer = (gl_feedback_buffer_func *)GetOpenGLProcAddress_Internal(state, "glFeedbackBuffer");
			glFinish = (gl_finish_func *)GetOpenGLProcAddress_Internal(state, "glFinish");
			glFlush = (gl_flush_func *)GetOpenGLProcAddress_Internal(state, "glFlush");
			glFogf = (gl_fogf_func *)GetOpenGLProcAddress_Internal(state, "glFogf");
			glFogfv = (gl_fogfv_func *)GetOpenGLProcAddress_Internal(state, "glFogfv");
			glFogi = (gl_fogi_func *)GetOpenGLProcAddress_Internal(state, "glFogi");
			glFogiv = (gl_fogiv_func *)GetOpenGLProcAddress_Internal(state, "glFogiv");
			glFrontFace = (gl_front_face_func *)GetOpenGLProcAddress_Internal(state, "glFrontFace");
			glFrustum = (gl_frustum_func *)GetOpenGLProcAddress_Internal(state, "glFrustum");
			glGenLists = (gl_gen_lists_func *)GetOpenGLProcAddress_Internal(state, "glGenLists");
			glGenTextures = (gl_gen_textures_func *)GetOpenGLProcAddress_Internal(state, "glGenTextures");
			glGetBooleanv = (gl_get_booleanv_func *)GetOpenGLProcAddress_Internal(state, "glGetBooleanv");
			glGetClipPlane = (gl_get_clip_plane_func *)GetOpenGLProcAddress_Internal(state, "glGetClipPlane");
			glGetDoublev = (gl_get_doublev_func *)GetOpenGLProcAddress_Internal(state, "glGetDoublev");
			glGetError = (gl_get_error_func *)GetOpenGLProcAddress_Internal(state, "glGetError");
			glGetFloatv = (gl_get_floatv_func *)GetOpenGLProcAddress_Internal(state, "glGetFloatv");
			glGetIntegerv = (gl_get_integerv_func *)GetOpenGLProcAddress_Internal(state, "glGetIntegerv");
			glGetLightfv = (gl_get_lightfv_func *)GetOpenGLProcAddress_Internal(state, "glGetLightfv");
			glGetLightiv = (gl_get_lightiv_func *)GetOpenGLProcAddress_Internal(state, "glGetLightiv");
			glGetMapdv = (gl_get_mapdv_func *)GetOpenGLProcAddress_Internal(state, "glGetMapdv");
			glGetMapfv = (gl_get_mapfv_func *)GetOpenGLProcAddress_Internal(state, "glGetMapfv");
			glGetMapiv = (gl_get_mapiv_func *)GetOpenGLProcAddress_Internal(state, "glGetMapiv");
			glGetMaterialfv = (gl_get_materialfv_func *)GetOpenGLProcAddress_Internal(state, "glGetMaterialfv");
			glGetMaterialiv = (gl_get_materialiv_func *)GetOpenGLProcAddress_Internal(state, "glGetMaterialiv");
			glGetPixelMapfv = (gl_get_pixel_mapfv_func *)GetOpenGLProcAddress_Internal(state, "glGetPixelMapfv");
			glGetPixelMapuiv = (gl_get_pixel_mapuiv_func *)GetOpenGLProcAddress_Internal(state, "glGetPixelMapuiv");
			glGetPixelMapusv = (gl_get_pixel_mapusv_func *)GetOpenGLProcAddress_Internal(state, "glGetPixelMapusv");
			glGetPointerv = (gl_get_pointerv_func *)GetOpenGLProcAddress_Internal(state, "glGetPointerv");
			glGetPolygonStipple = (gl_get_polygon_stipple_func *)GetOpenGLProcAddress_Internal(state, "glGetPolygonStipple");
			glGetString = (gl_get_string_func *)GetOpenGLProcAddress_Internal(state, "glGetString");
			glGetTexEnvfv = (gl_get_tex_envfv_func *)GetOpenGLProcAddress_Internal(state, "glGetTexEnvfv");
			glGetTexEnviv = (gl_get_tex_enviv_func *)GetOpenGLProcAddress_Internal(state, "glGetTexEnviv");
			glGetTexGendv = (gl_get_tex_gendv_func *)GetOpenGLProcAddress_Internal(state, "glGetTexGendv");
			glGetTexGenfv = (gl_get_tex_genfv_func *)GetOpenGLProcAddress_Internal(state, "glGetTexGenfv");
			glGetTexGeniv = (gl_get_tex_geniv_func *)GetOpenGLProcAddress_Internal(state, "glGetTexGeniv");
			glGetTexImage = (gl_get_tex_image_func *)GetOpenGLProcAddress_Internal(state, "glGetTexImage");
			glGetTexLevelParameterfv = (gl_get_tex_level_parameterfv_func *)GetOpenGLProcAddress_Internal(state, "glGetTexLevelParameterfv");
			glGetTexLevelParameteriv = (gl_get_tex_level_parameteriv_func *)GetOpenGLProcAddress_Internal(state, "glGetTexLevelParameteriv");
			glGetTexParameterfv = (gl_get_tex_parameterfv_func *)GetOpenGLProcAddress_Internal(state, "glGetTexParameterfv");
			glGetTexParameteriv = (gl_get_tex_parameteriv_func *)GetOpenGLProcAddress_Internal(state, "glGetTexParameteriv");
			glHint = (gl_hint_func *)GetOpenGLProcAddress_Internal(state, "glHint");
			glIndexMask = (gl_index_mask_func *)GetOpenGLProcAddress_Internal(state, "glIndexMask");
			glIndexPointer = (gl_index_pointer_func *)GetOpenGLProcAddress_Internal(state, "glIndexPointer");
			glIndexd = (gl_indexd_func *)GetOpenGLProcAddress_Internal(state, "glIndexd");
			glIndexdv = (gl_indexdv_func *)GetOpenGLProcAddress_Internal(state, "glIndexdv");
			glIndexf = (gl_indexf_func *)GetOpenGLProcAddress_Internal(state, "glIndexf");
			glIndexfv = (gl_indexfv_func *)GetOpenGLProcAddress_Internal(state, "glIndexfv");
			glIndexi = (gl_indexi_func *)GetOpenGLProcAddress_Internal(state, "glIndexi");
			glIndexiv = (gl_indexiv_func *)GetOpenGLProcAddress_Internal(state, "glIndexiv");
			glIndexs = (gl_indexs_func *)GetOpenGLProcAddress_Internal(state, "glIndexs");
			glIndexsv = (gl_indexsv_func *)GetOpenGLProcAddress_Internal(state, "glIndexsv");
			glIndexub = (gl_indexub_func *)GetOpenGLProcAddress_Internal(state, "glIndexub");
			glIndexubv = (gl_indexubv_func *)GetOpenGLProcAddress_Internal(state, "glIndexubv");
			glInitNames = (gl_init_names_func *)GetOpenGLProcAddress_Internal(state, "glInitNames");
			glInterleavedArrays = (gl_interleaved_arrays_func *)GetOpenGLProcAddress_Internal(state, "glInterleavedArrays");
			glIsEnabled = (gl_is_enabled_func *)GetOpenGLProcAddress_Internal(state, "glIsEnabled");
			glIsList = (gl_is_list_func *)GetOpenGLProcAddress_Internal(state, "glIsList");
			glIsTexture = (gl_is_texture_func *)GetOpenGLProcAddress_Internal(state, "glIsTexture");
			glLightModelf = (gl_light_modelf_func *)GetOpenGLProcAddress_Internal(state, "glLightModelf");
			glLightModelfv = (gl_light_modelfv_func *)GetOpenGLProcAddress_Internal(state, "glLightModelfv");
			glLightModeli = (gl_light_modeli_func *)GetOpenGLProcAddress_Internal(state, "glLightModeli");
			glLightModeliv = (gl_light_modeliv_func *)GetOpenGLProcAddress_Internal(state, "glLightModeliv");
			glLightf = (gl_lightf_func *)GetOpenGLProcAddress_Internal(state, "glLightf");
			glLightfv = (gl_lightfv_func *)GetOpenGLProcAddress_Internal(state, "glLightfv");
			glLighti = (gl_lighti_func *)GetOpenGLProcAddress_Internal(state, "glLighti");
			glLightiv = (gl_lightiv_func *)GetOpenGLProcAddress_Internal(state, "glLightiv");
			glLineStipple = (gl_line_stipple_func *)GetOpenGLProcAddress_Internal(state, "glLineStipple");
			glLineWidth = (gl_line_width_func *)GetOpenGLProcAddress_Internal(state, "glLineWidth");
			glListBase = (gl_list_base_func *)GetOpenGLProcAddress_Internal(state, "glListBase");
			glLoadIdentity = (gl_load_identity_func *)GetOpenGLProcAddress_Internal(state, "glLoadIdentity");
			glLoadMatrixd = (gl_load_matrixd_func *)GetOpenGLProcAddress_Internal(state, "glLoadMatrixd");
			glLoadMatrixf = (gl_load_matrixf_func *)GetOpenGLProcAddress_Internal(state, "glLoadMatrixf");
			glLoadName = (gl_load_name_func *)GetOpenGLProcAddress_Internal(state, "glLoadName");
			glLogicOp = (gl_logic_op_func *)GetOpenGLProcAddress_Internal(state, "glLogicOp");
			glMap1d = (gl_map1d_func *)GetOpenGLProcAddress_Internal(state, "glMap1d");
			glMap1f = (gl_map1f_func *)GetOpenGLProcAddress_Internal(state, "glMap1f");
			glMap2d = (gl_map2d_func *)GetOpenGLProcAddress_Internal(state, "glMap2d");
			glMap2f = (gl_map2f_func *)GetOpenGLProcAddress_Internal(state, "glMap2f");
			glMapGrid1d = (gl_map_grid1d_func *)GetOpenGLProcAddress_Internal(state, "glMapGrid1d");
			glMapGrid1f = (gl_map_grid1f_func *)GetOpenGLProcAddress_Internal(state, "glMapGrid1f");
			glMapGrid2d = (gl_map_grid2d_func *)GetOpenGLProcAddress_Internal(state, "glMapGrid2d");
			glMapGrid2f = (gl_map_grid2f_func *)GetOpenGLProcAddress_Internal(state, "glMapGrid2f");
			glMaterialf = (gl_materialf_func *)GetOpenGLProcAddress_Internal(state, "glMaterialf");
			glMaterialfv = (gl_materialfv_func *)GetOpenGLProcAddress_Internal(state, "glMaterialfv");
			glMateriali = (gl_materiali_func *)GetOpenGLProcAddress_Internal(state, "glMateriali");
			glMaterialiv = (gl_materialiv_func *)GetOpenGLProcAddress_Internal(state, "glMaterialiv");
			glMatrixMode = (gl_matrix_mode_func *)GetOpenGLProcAddress_Internal(state, "glMatrixMode");
			glMultMatrixd = (gl_mult_matrixd_func *)GetOpenGLProcAddress_Internal(state, "glMultMatrixd");
			glMultMatrixf = (gl_mult_matrixf_func *)GetOpenGLProcAddress_Internal(state, "glMultMatrixf");
			glNewList = (gl_new_list_func *)GetOpenGLProcAddress_Internal(state, "glNewList");
			glNormal3b = (gl_normal3b_func *)GetOpenGLProcAddress_Internal(state, "glNormal3b");
			glNormal3bv = (gl_normal3bv_func *)GetOpenGLProcAddress_Internal(state, "glNormal3bv");
			glNormal3d = (gl_normal3d_func *)GetOpenGLProcAddress_Internal(state, "glNormal3d");
			glNormal3dv = (gl_normal3dv_func *)GetOpenGLProcAddress_Internal(state, "glNormal3dv");
			glNormal3f = (gl_normal3f_func *)GetOpenGLProcAddress_Internal(state, "glNormal3f");
			glNormal3fv = (gl_normal3fv_func *)GetOpenGLProcAddress_Internal(state, "glNormal3fv");
			glNormal3i = (gl_normal3i_func *)GetOpenGLProcAddress_Internal(state, "glNormal3i");
			glNormal3iv = (gl_normal3iv_func *)GetOpenGLProcAddress_Internal(state, "glNormal3iv");
			glNormal3s = (gl_normal3s_func *)GetOpenGLProcAddress_Internal(state, "glNormal3s");
			glNormal3sv = (gl_normal3sv_func *)GetOpenGLProcAddress_Internal(state, "glNormal3sv");
			glNormalPointer = (gl_normal_pointer_func *)GetOpenGLProcAddress_Internal(state, "glNormalPointer");
			glOrtho = (gl_ortho_func *)GetOpenGLProcAddress_Internal(state, "glOrtho");
			glPassThrough = (gl_pass_through_func *)GetOpenGLProcAddress_Internal(state, "glPassThrough");
			glPixelMapfv = (gl_pixel_mapfv_func *)GetOpenGLProcAddress_Internal(state, "glPixelMapfv");
			glPixelMapuiv = (gl_pixel_mapuiv_func *)GetOpenGLProcAddress_Internal(state, "glPixelMapuiv");
			glPixelMapusv = (gl_pixel_mapusv_func *)GetOpenGLProcAddress_Internal(state, "glPixelMapusv");
			glPixelStoref = (gl_pixel_storef_func *)GetOpenGLProcAddress_Internal(state, "glPixelStoref");
			glPixelStorei = (gl_pixel_storei_func *)GetOpenGLProcAddress_Internal(state, "glPixelStorei");
			glPixelTransferf = (gl_pixel_transferf_func *)GetOpenGLProcAddress_Internal(state, "glPixelTransferf");
			glPixelTransferi = (gl_pixel_transferi_func *)GetOpenGLProcAddress_Internal(state, "glPixelTransferi");
			glPixelZoom = (gl_pixel_zoom_func *)GetOpenGLProcAddress_Internal(state, "glPixelZoom");
			glPointSize = (gl_point_size_func *)GetOpenGLProcAddress_Internal(state, "glPointSize");
			glPolygonMode = (gl_polygon_mode_func *)GetOpenGLProcAddress_Internal(state, "glPolygonMode");
			glPolygonOffset = (gl_polygon_offset_func *)GetOpenGLProcAddress_Internal(state, "glPolygonOffset");
			glPolygonStipple = (gl_polygon_stipple_func *)GetOpenGLProcAddress_Internal(state, "glPolygonStipple");
			glPopAttrib = (gl_pop_attrib_func *)GetOpenGLProcAddress_Internal(state, "glPopAttrib");
			glPopClientAttrib = (gl_pop_client_attrib_func *)GetOpenGLProcAddress_Internal(state, "glPopClientAttrib");
			glPopMatrix = (gl_pop_matrix_func *)GetOpenGLProcAddress_Internal(state, "glPopMatrix");
			glPopName = (gl_pop_name_func *)GetOpenGLProcAddress_Internal(state, "glPopName");
			glPrioritizeTextures = (gl_prioritize_textures_func *)GetOpenGLProcAddress_Internal(state, "glPrioritizeTextures");
			glPushAttrib = (gl_push_attrib_func *)GetOpenGLProcAddress_Internal(state, "glPushAttrib");
			glPushClientAttrib = (gl_push_client_attrib_func *)GetOpenGLProcAddress_Internal(state, "glPushClientAttrib");
			glPushMatrix = (gl_push_matrix_func *)GetOpenGLProcAddress_Internal(state, "glPushMatrix");
			glPushName = (gl_push_name_func *)GetOpenGLProcAddress_Internal(state, "glPushName");
			glRasterPos2d = (gl_raster_pos2d_func *)GetOpenGLProcAddress_Internal(state, "glRasterPos2d");
			glRasterPos2dv = (gl_raster_pos2dv_func *)GetOpenGLProcAddress_Internal(state, "glRasterPos2dv");
			glRasterPos2f = (gl_raster_pos2f_func *)GetOpenGLProcAddress_Internal(state, "glRasterPos2f");
			glRasterPos2fv = (gl_raster_pos2fv_func *)GetOpenGLProcAddress_Internal(state, "glRasterPos2fv");
			glRasterPos2i = (gl_raster_pos2i_func *)GetOpenGLProcAddress_Internal(state, "glRasterPos2i");
			glRasterPos2iv = (gl_raster_pos2iv_func *)GetOpenGLProcAddress_Internal(state, "glRasterPos2iv");
			glRasterPos2s = (gl_raster_pos2s_func *)GetOpenGLProcAddress_Internal(state, "glRasterPos2s");
			glRasterPos2sv = (gl_raster_pos2sv_func *)GetOpenGLProcAddress_Internal(state, "glRasterPos2sv");
			glRasterPos3d = (gl_raster_pos3d_func *)GetOpenGLProcAddress_Internal(state, "glRasterPos3d");
			glRasterPos3dv = (gl_raster_pos3dv_func *)GetOpenGLProcAddress_Internal(state, "glRasterPos3dv");
			glRasterPos3f = (gl_raster_pos3f_func *)GetOpenGLProcAddress_Internal(state, "glRasterPos3f");
			glRasterPos3fv = (gl_raster_pos3fv_func *)GetOpenGLProcAddress_Internal(state, "glRasterPos3fv");
			glRasterPos3i = (gl_raster_pos3i_func *)GetOpenGLProcAddress_Internal(state, "glRasterPos3i");
			glRasterPos3iv = (gl_raster_pos3iv_func *)GetOpenGLProcAddress_Internal(state, "glRasterPos3iv");
			glRasterPos3s = (gl_raster_pos3s_func *)GetOpenGLProcAddress_Internal(state, "glRasterPos3s");
			glRasterPos3sv = (gl_raster_pos3sv_func *)GetOpenGLProcAddress_Internal(state, "glRasterPos3sv");
			glRasterPos4d = (gl_raster_pos4d_func *)GetOpenGLProcAddress_Internal(state, "glRasterPos4d");
			glRasterPos4dv = (gl_raster_pos4dv_func *)GetOpenGLProcAddress_Internal(state, "glRasterPos4dv");
			glRasterPos4f = (gl_raster_pos4f_func *)GetOpenGLProcAddress_Internal(state, "glRasterPos4f");
			glRasterPos4fv = (gl_raster_pos4fv_func *)GetOpenGLProcAddress_Internal(state, "glRasterPos4fv");
			glRasterPos4i = (gl_raster_pos4i_func *)GetOpenGLProcAddress_Internal(state, "glRasterPos4i");
			glRasterPos4iv = (gl_raster_pos4iv_func *)GetOpenGLProcAddress_Internal(state, "glRasterPos4iv");
			glRasterPos4s = (gl_raster_pos4s_func *)GetOpenGLProcAddress_Internal(state, "glRasterPos4s");
			glRasterPos4sv = (gl_raster_pos4sv_func *)GetOpenGLProcAddress_Internal(state, "glRasterPos4sv");
			glReadBuffer = (gl_read_buffer_func *)GetOpenGLProcAddress_Internal(state, "glReadBuffer");
			glReadPixels = (gl_read_pixels_func *)GetOpenGLProcAddress_Internal(state, "glReadPixels");
			glRectd = (gl_rectd_func *)GetOpenGLProcAddress_Internal(state, "glRectd");
			glRectdv = (gl_rectdv_func *)GetOpenGLProcAddress_Internal(state, "glRectdv");
			glRectf = (gl_rectf_func *)GetOpenGLProcAddress_Internal(state, "glRectf");
			glRectfv = (gl_rectfv_func *)GetOpenGLProcAddress_Internal(state, "glRectfv");
			glRecti = (gl_recti_func *)GetOpenGLProcAddress_Internal(state, "glRecti");
			glRectiv = (gl_rectiv_func *)GetOpenGLProcAddress_Internal(state, "glRectiv");
			glRects = (gl_rects_func *)GetOpenGLProcAddress_Internal(state, "glRects");
			glRectsv = (gl_rectsv_func *)GetOpenGLProcAddress_Internal(state, "glRectsv");
			glRenderMode = (gl_render_mode_func *)GetOpenGLProcAddress_Internal(state, "glRenderMode");
			glRotated = (gl_rotated_func *)GetOpenGLProcAddress_Internal(state, "glRotated");
			glRotatef = (gl_rotatef_func *)GetOpenGLProcAddress_Internal(state, "glRotatef");
			glScaled = (gl_scaled_func *)GetOpenGLProcAddress_Internal(state, "glScaled");
			glScalef = (gl_scalef_func *)GetOpenGLProcAddress_Internal(state, "glScalef");
			glScissor = (gl_scissor_func *)GetOpenGLProcAddress_Internal(state, "glScissor");
			glSelectBuffer = (gl_select_buffer_func *)GetOpenGLProcAddress_Internal(state, "glSelectBuffer");
			glShadeModel = (gl_shade_model_func *)GetOpenGLProcAddress_Internal(state, "glShadeModel");
			glStencilFunc = (gl_stencil_func_func *)GetOpenGLProcAddress_Internal(state, "glStencilFunc");
			glStencilMask = (gl_stencil_mask_func *)GetOpenGLProcAddress_Internal(state, "glStencilMask");
			glStencilOp = (gl_stencil_op_func *)GetOpenGLProcAddress_Internal(state, "glStencilOp");
			glTexCoord1d = (gl_tex_coord1d_func *)GetOpenGLProcAddress_Internal(state, "glTexCoord1d");
			glTexCoord1dv = (gl_tex_coord1dv_func *)GetOpenGLProcAddress_Internal(state, "glTexCoord1dv");
			glTexCoord1f = (gl_tex_coord1f_func *)GetOpenGLProcAddress_Internal(state, "glTexCoord1f");
			glTexCoord1fv = (gl_tex_coord1fv_func *)GetOpenGLProcAddress_Internal(state, "glTexCoord1fv");
			glTexCoord1i = (gl_tex_coord1i_func *)GetOpenGLProcAddress_Internal(state, "glTexCoord1i");
			glTexCoord1iv = (gl_tex_coord1iv_func *)GetOpenGLProcAddress_Internal(state, "glTexCoord1iv");
			glTexCoord1s = (gl_tex_coord1s_func *)GetOpenGLProcAddress_Internal(state, "glTexCoord1s");
			glTexCoord1sv = (gl_tex_coord1sv_func *)GetOpenGLProcAddress_Internal(state, "glTexCoord1sv");
			glTexCoord2d = (gl_tex_coord2d_func *)GetOpenGLProcAddress_Internal(state, "glTexCoord2d");
			glTexCoord2dv = (gl_tex_coord2dv_func *)GetOpenGLProcAddress_Internal(state, "glTexCoord2dv");
			glTexCoord2f = (gl_tex_coord2f_func *)GetOpenGLProcAddress_Internal(state, "glTexCoord2f");
			glTexCoord2fv = (gl_tex_coord2fv_func *)GetOpenGLProcAddress_Internal(state, "glTexCoord2fv");
			glTexCoord2i = (gl_tex_coord2i_func *)GetOpenGLProcAddress_Internal(state, "glTexCoord2i");
			glTexCoord2iv = (gl_tex_coord2iv_func *)GetOpenGLProcAddress_Internal(state, "glTexCoord2iv");
			glTexCoord2s = (gl_tex_coord2s_func *)GetOpenGLProcAddress_Internal(state, "glTexCoord2s");
			glTexCoord2sv = (gl_tex_coord2sv_func *)GetOpenGLProcAddress_Internal(state, "glTexCoord2sv");
			glTexCoord3d = (gl_tex_coord3d_func *)GetOpenGLProcAddress_Internal(state, "glTexCoord3d");
			glTexCoord3dv = (gl_tex_coord3dv_func *)GetOpenGLProcAddress_Internal(state, "glTexCoord3dv");
			glTexCoord3f = (gl_tex_coord3f_func *)GetOpenGLProcAddress_Internal(state, "glTexCoord3f");
			glTexCoord3fv = (gl_tex_coord3fv_func *)GetOpenGLProcAddress_Internal(state, "glTexCoord3fv");
			glTexCoord3i = (gl_tex_coord3i_func *)GetOpenGLProcAddress_Internal(state, "glTexCoord3i");
			glTexCoord3iv = (gl_tex_coord3iv_func *)GetOpenGLProcAddress_Internal(state, "glTexCoord3iv");
			glTexCoord3s = (gl_tex_coord3s_func *)GetOpenGLProcAddress_Internal(state, "glTexCoord3s");
			glTexCoord3sv = (gl_tex_coord3sv_func *)GetOpenGLProcAddress_Internal(state, "glTexCoord3sv");
			glTexCoord4d = (gl_tex_coord4d_func *)GetOpenGLProcAddress_Internal(state, "glTexCoord4d");
			glTexCoord4dv = (gl_tex_coord4dv_func *)GetOpenGLProcAddress_Internal(state, "glTexCoord4dv");
			glTexCoord4f = (gl_tex_coord4f_func *)GetOpenGLProcAddress_Internal(state, "glTexCoord4f");
			glTexCoord4fv = (gl_tex_coord4fv_func *)GetOpenGLProcAddress_Internal(state, "glTexCoord4fv");
			glTexCoord4i = (gl_tex_coord4i_func *)GetOpenGLProcAddress_Internal(state, "glTexCoord4i");
			glTexCoord4iv = (gl_tex_coord4iv_func *)GetOpenGLProcAddress_Internal(state, "glTexCoord4iv");
			glTexCoord4s = (gl_tex_coord4s_func *)GetOpenGLProcAddress_Internal(state, "glTexCoord4s");
			glTexCoord4sv = (gl_tex_coord4sv_func *)GetOpenGLProcAddress_Internal(state, "glTexCoord4sv");
			glTexCoordPointer = (gl_tex_coord_pointer_func *)GetOpenGLProcAddress_Internal(state, "glTexCoordPointer");
			glTexEnvf = (gl_tex_envf_func *)GetOpenGLProcAddress_Internal(state, "glTexEnvf");
			glTexEnvfv = (gl_tex_envfv_func *)GetOpenGLProcAddress_Internal(state, "glTexEnvfv");
			glTexEnvi = (gl_tex_envi_func *)GetOpenGLProcAddress_Internal(state, "glTexEnvi");
			glTexEnviv = (gl_tex_enviv_func *)GetOpenGLProcAddress_Internal(state, "glTexEnviv");
			glTexGend = (gl_tex_gend_func *)GetOpenGLProcAddress_Internal(state, "glTexGend");
			glTexGendv = (gl_tex_gendv_func *)GetOpenGLProcAddress_Internal(state, "glTexGendv");
			glTexGenf = (gl_tex_genf_func *)GetOpenGLProcAddress_Internal(state, "glTexGenf");
			glTexGenfv = (gl_tex_genfv_func *)GetOpenGLProcAddress_Internal(state, "glTexGenfv");
			glTexGeni = (gl_tex_geni_func *)GetOpenGLProcAddress_Internal(state, "glTexGeni");
			glTexGeniv = (gl_tex_geniv_func *)GetOpenGLProcAddress_Internal(state, "glTexGeniv");
			glTexImage1D = (gl_tex_image1d_func *)GetOpenGLProcAddress_Internal(state, "glTexImage1D");
			glTexImage2D = (gl_tex_image2d_func *)GetOpenGLProcAddress_Internal(state, "glTexImage2D");
			glTexParameterf = (gl_tex_parameterf_func *)GetOpenGLProcAddress_Internal(state, "glTexParameterf");
			glTexParameterfv = (gl_tex_parameterfv_func *)GetOpenGLProcAddress_Internal(state, "glTexParameterfv");
			glTexParameteri = (gl_tex_parameteri_func *)GetOpenGLProcAddress_Internal(state, "glTexParameteri");
			glTexParameteriv = (gl_tex_parameteriv_func *)GetOpenGLProcAddress_Internal(state, "glTexParameteriv");
			glTexSubImage1D = (gl_tex_sub_image1d_func *)GetOpenGLProcAddress_Internal(state, "glTexSubImage1D");
			glTexSubImage2D = (gl_tex_sub_image2d_func *)GetOpenGLProcAddress_Internal(state, "glTexSubImage2D");
			glTranslated = (gl_translated_func *)GetOpenGLProcAddress_Internal(state, "glTranslated");
			glTranslatef = (gl_translatef_func *)GetOpenGLProcAddress_Internal(state, "glTranslatef");
			glVertex2d = (gl_vertex2d_func *)GetOpenGLProcAddress_Internal(state, "glVertex2d");
			glVertex2dv = (gl_vertex2dv_func *)GetOpenGLProcAddress_Internal(state, "glVertex2dv");
			glVertex2f = (gl_vertex2f_func *)GetOpenGLProcAddress_Internal(state, "glVertex2f");
			glVertex2fv = (gl_vertex2fv_func *)GetOpenGLProcAddress_Internal(state, "glVertex2fv");
			glVertex2i = (gl_vertex2i_func *)GetOpenGLProcAddress_Internal(state, "glVertex2i");
			glVertex2iv = (gl_vertex2iv_func *)GetOpenGLProcAddress_Internal(state, "glVertex2iv");
			glVertex2s = (gl_vertex2s_func *)GetOpenGLProcAddress_Internal(state, "glVertex2s");
			glVertex2sv = (gl_vertex2sv_func *)GetOpenGLProcAddress_Internal(state, "glVertex2sv");
			glVertex3d = (gl_vertex3d_func *)GetOpenGLProcAddress_Internal(state, "glVertex3d");
			glVertex3dv = (gl_vertex3dv_func *)GetOpenGLProcAddress_Internal(state, "glVertex3dv");
			glVertex3f = (gl_vertex3f_func *)GetOpenGLProcAddress_Internal(state, "glVertex3f");
			glVertex3fv = (gl_vertex3fv_func *)GetOpenGLProcAddress_Internal(state, "glVertex3fv");
			glVertex3i = (gl_vertex3i_func *)GetOpenGLProcAddress_Internal(state, "glVertex3i");
			glVertex3iv = (gl_vertex3iv_func *)GetOpenGLProcAddress_Internal(state, "glVertex3iv");
			glVertex3s = (gl_vertex3s_func *)GetOpenGLProcAddress_Internal(state, "glVertex3s");
			glVertex3sv = (gl_vertex3sv_func *)GetOpenGLProcAddress_Internal(state, "glVertex3sv");
			glVertex4d = (gl_vertex4d_func *)GetOpenGLProcAddress_Internal(state, "glVertex4d");
			glVertex4dv = (gl_vertex4dv_func *)GetOpenGLProcAddress_Internal(state, "glVertex4dv");
			glVertex4f = (gl_vertex4f_func *)GetOpenGLProcAddress_Internal(state, "glVertex4f");
			glVertex4fv = (gl_vertex4fv_func *)GetOpenGLProcAddress_Internal(state, "glVertex4fv");
			glVertex4i = (gl_vertex4i_func *)GetOpenGLProcAddress_Internal(state, "glVertex4i");
			glVertex4iv = (gl_vertex4iv_func *)GetOpenGLProcAddress_Internal(state, "glVertex4iv");
			glVertex4s = (gl_vertex4s_func *)GetOpenGLProcAddress_Internal(state, "glVertex4s");
			glVertex4sv = (gl_vertex4sv_func *)GetOpenGLProcAddress_Internal(state, "glVertex4sv");
			glVertexPointer = (gl_vertex_pointer_func *)GetOpenGLProcAddress_Internal(state, "glVertexPointer");
			glViewport = (gl_viewport_func *)GetOpenGLProcAddress_Internal(state, "glViewport");
#	endif //GL_VERSION_1_1

#	if GL_VERSION_1_2
			glDrawRangeElements = (gl_draw_range_elements_func *)GetOpenGLProcAddress_Internal(state, "glDrawRangeElements");
			glTexImage3D = (gl_tex_image3d_func *)GetOpenGLProcAddress_Internal(state, "glTexImage3D");
			glTexSubImage3D = (gl_tex_sub_image3d_func *)GetOpenGLProcAddress_Internal(state, "glTexSubImage3D");
			glCopyTexSubImage3D = (gl_copy_tex_sub_image3d_func *)GetOpenGLProcAddress_Internal(state, "glCopyTexSubImage3D");
#	endif //GL_VERSION_1_2

#	if GL_VERSION_1_3
			glActiveTexture = (gl_active_texture_func *)GetOpenGLProcAddress_Internal(state, "glActiveTexture");
			glSampleCoverage = (gl_sample_coverage_func *)GetOpenGLProcAddress_Internal(state, "glSampleCoverage");
			glCompressedTexImage3D = (gl_compressed_tex_image3d_func *)GetOpenGLProcAddress_Internal(state, "glCompressedTexImage3D");
			glCompressedTexImage2D = (gl_compressed_tex_image2d_func *)GetOpenGLProcAddress_Internal(state, "glCompressedTexImage2D");
			glCompressedTexImage1D = (gl_compressed_tex_image1d_func *)GetOpenGLProcAddress_Internal(state, "glCompressedTexImage1D");
			glCompressedTexSubImage3D = (gl_compressed_tex_sub_image3d_func *)GetOpenGLProcAddress_Internal(state, "glCompressedTexSubImage3D");
			glCompressedTexSubImage2D = (gl_compressed_tex_sub_image2d_func *)GetOpenGLProcAddress_Internal(state, "glCompressedTexSubImage2D");
			glCompressedTexSubImage1D = (gl_compressed_tex_sub_image1d_func *)GetOpenGLProcAddress_Internal(state, "glCompressedTexSubImage1D");
			glGetCompressedTexImage = (gl_get_compressed_tex_image_func *)GetOpenGLProcAddress_Internal(state, "glGetCompressedTexImage");
			glClientActiveTexture = (gl_client_active_texture_func *)GetOpenGLProcAddress_Internal(state, "glClientActiveTexture");
			glMultiTexCoord1d = (gl_multi_tex_coord1d_func *)GetOpenGLProcAddress_Internal(state, "glMultiTexCoord1d");
			glMultiTexCoord1dv = (gl_multi_tex_coord1dv_func *)GetOpenGLProcAddress_Internal(state, "glMultiTexCoord1dv");
			glMultiTexCoord1f = (gl_multi_tex_coord1f_func *)GetOpenGLProcAddress_Internal(state, "glMultiTexCoord1f");
			glMultiTexCoord1fv = (gl_multi_tex_coord1fv_func *)GetOpenGLProcAddress_Internal(state, "glMultiTexCoord1fv");
			glMultiTexCoord1i = (gl_multi_tex_coord1i_func *)GetOpenGLProcAddress_Internal(state, "glMultiTexCoord1i");
			glMultiTexCoord1iv = (gl_multi_tex_coord1iv_func *)GetOpenGLProcAddress_Internal(state, "glMultiTexCoord1iv");
			glMultiTexCoord1s = (gl_multi_tex_coord1s_func *)GetOpenGLProcAddress_Internal(state, "glMultiTexCoord1s");
			glMultiTexCoord1sv = (gl_multi_tex_coord1sv_func *)GetOpenGLProcAddress_Internal(state, "glMultiTexCoord1sv");
			glMultiTexCoord2d = (gl_multi_tex_coord2d_func *)GetOpenGLProcAddress_Internal(state, "glMultiTexCoord2d");
			glMultiTexCoord2dv = (gl_multi_tex_coord2dv_func *)GetOpenGLProcAddress_Internal(state, "glMultiTexCoord2dv");
			glMultiTexCoord2f = (gl_multi_tex_coord2f_func *)GetOpenGLProcAddress_Internal(state, "glMultiTexCoord2f");
			glMultiTexCoord2fv = (gl_multi_tex_coord2fv_func *)GetOpenGLProcAddress_Internal(state, "glMultiTexCoord2fv");
			glMultiTexCoord2i = (gl_multi_tex_coord2i_func *)GetOpenGLProcAddress_Internal(state, "glMultiTexCoord2i");
			glMultiTexCoord2iv = (gl_multi_tex_coord2iv_func *)GetOpenGLProcAddress_Internal(state, "glMultiTexCoord2iv");
			glMultiTexCoord2s = (gl_multi_tex_coord2s_func *)GetOpenGLProcAddress_Internal(state, "glMultiTexCoord2s");
			glMultiTexCoord2sv = (gl_multi_tex_coord2sv_func *)GetOpenGLProcAddress_Internal(state, "glMultiTexCoord2sv");
			glMultiTexCoord3d = (gl_multi_tex_coord3d_func *)GetOpenGLProcAddress_Internal(state, "glMultiTexCoord3d");
			glMultiTexCoord3dv = (gl_multi_tex_coord3dv_func *)GetOpenGLProcAddress_Internal(state, "glMultiTexCoord3dv");
			glMultiTexCoord3f = (gl_multi_tex_coord3f_func *)GetOpenGLProcAddress_Internal(state, "glMultiTexCoord3f");
			glMultiTexCoord3fv = (gl_multi_tex_coord3fv_func *)GetOpenGLProcAddress_Internal(state, "glMultiTexCoord3fv");
			glMultiTexCoord3i = (gl_multi_tex_coord3i_func *)GetOpenGLProcAddress_Internal(state, "glMultiTexCoord3i");
			glMultiTexCoord3iv = (gl_multi_tex_coord3iv_func *)GetOpenGLProcAddress_Internal(state, "glMultiTexCoord3iv");
			glMultiTexCoord3s = (gl_multi_tex_coord3s_func *)GetOpenGLProcAddress_Internal(state, "glMultiTexCoord3s");
			glMultiTexCoord3sv = (gl_multi_tex_coord3sv_func *)GetOpenGLProcAddress_Internal(state, "glMultiTexCoord3sv");
			glMultiTexCoord4d = (gl_multi_tex_coord4d_func *)GetOpenGLProcAddress_Internal(state, "glMultiTexCoord4d");
			glMultiTexCoord4dv = (gl_multi_tex_coord4dv_func *)GetOpenGLProcAddress_Internal(state, "glMultiTexCoord4dv");
			glMultiTexCoord4f = (gl_multi_tex_coord4f_func *)GetOpenGLProcAddress_Internal(state, "glMultiTexCoord4f");
			glMultiTexCoord4fv = (gl_multi_tex_coord4fv_func *)GetOpenGLProcAddress_Internal(state, "glMultiTexCoord4fv");
			glMultiTexCoord4i = (gl_multi_tex_coord4i_func *)GetOpenGLProcAddress_Internal(state, "glMultiTexCoord4i");
			glMultiTexCoord4iv = (gl_multi_tex_coord4iv_func *)GetOpenGLProcAddress_Internal(state, "glMultiTexCoord4iv");
			glMultiTexCoord4s = (gl_multi_tex_coord4s_func *)GetOpenGLProcAddress_Internal(state, "glMultiTexCoord4s");
			glMultiTexCoord4sv = (gl_multi_tex_coord4sv_func *)GetOpenGLProcAddress_Internal(state, "glMultiTexCoord4sv");
			glLoadTransposeMatrixf = (gl_load_transpose_matrixf_func *)GetOpenGLProcAddress_Internal(state, "glLoadTransposeMatrixf");
			glLoadTransposeMatrixd = (gl_load_transpose_matrixd_func *)GetOpenGLProcAddress_Internal(state, "glLoadTransposeMatrixd");
			glMultTransposeMatrixf = (gl_mult_transpose_matrixf_func *)GetOpenGLProcAddress_Internal(state, "glMultTransposeMatrixf");
			glMultTransposeMatrixd = (gl_mult_transpose_matrixd_func *)GetOpenGLProcAddress_Internal(state, "glMultTransposeMatrixd");
#	endif //GL_VERSION_1_3

#	if GL_VERSION_1_4
			glBlendFuncSeparate = (gl_blend_func_separate_func *)GetOpenGLProcAddress_Internal(state, "glBlendFuncSeparate");
			glMultiDrawArrays = (gl_multi_draw_arrays_func *)GetOpenGLProcAddress_Internal(state, "glMultiDrawArrays");
			glMultiDrawElements = (gl_multi_draw_elements_func *)GetOpenGLProcAddress_Internal(state, "glMultiDrawElements");
			glPointParameterf = (gl_point_parameterf_func *)GetOpenGLProcAddress_Internal(state, "glPointParameterf");
			glPointParameterfv = (gl_point_parameterfv_func *)GetOpenGLProcAddress_Internal(state, "glPointParameterfv");
			glPointParameteri = (gl_point_parameteri_func *)GetOpenGLProcAddress_Internal(state, "glPointParameteri");
			glPointParameteriv = (gl_point_parameteriv_func *)GetOpenGLProcAddress_Internal(state, "glPointParameteriv");
			glFogCoordf = (gl_fog_coordf_func *)GetOpenGLProcAddress_Internal(state, "glFogCoordf");
			glFogCoordfv = (gl_fog_coordfv_func *)GetOpenGLProcAddress_Internal(state, "glFogCoordfv");
			glFogCoordd = (gl_fog_coordd_func *)GetOpenGLProcAddress_Internal(state, "glFogCoordd");
			glFogCoorddv = (gl_fog_coorddv_func *)GetOpenGLProcAddress_Internal(state, "glFogCoorddv");
			glFogCoordPointer = (gl_fog_coord_pointer_func *)GetOpenGLProcAddress_Internal(state, "glFogCoordPointer");
			glSecondaryColor3b = (gl_secondary_color3b_func *)GetOpenGLProcAddress_Internal(state, "glSecondaryColor3b");
			glSecondaryColor3bv = (gl_secondary_color3bv_func *)GetOpenGLProcAddress_Internal(state, "glSecondaryColor3bv");
			glSecondaryColor3d = (gl_secondary_color3d_func *)GetOpenGLProcAddress_Internal(state, "glSecondaryColor3d");
			glSecondaryColor3dv = (gl_secondary_color3dv_func *)GetOpenGLProcAddress_Internal(state, "glSecondaryColor3dv");
			glSecondaryColor3f = (gl_secondary_color3f_func *)GetOpenGLProcAddress_Internal(state, "glSecondaryColor3f");
			glSecondaryColor3fv = (gl_secondary_color3fv_func *)GetOpenGLProcAddress_Internal(state, "glSecondaryColor3fv");
			glSecondaryColor3i = (gl_secondary_color3i_func *)GetOpenGLProcAddress_Internal(state, "glSecondaryColor3i");
			glSecondaryColor3iv = (gl_secondary_color3iv_func *)GetOpenGLProcAddress_Internal(state, "glSecondaryColor3iv");
			glSecondaryColor3s = (gl_secondary_color3s_func *)GetOpenGLProcAddress_Internal(state, "glSecondaryColor3s");
			glSecondaryColor3sv = (gl_secondary_color3sv_func *)GetOpenGLProcAddress_Internal(state, "glSecondaryColor3sv");
			glSecondaryColor3ub = (gl_secondary_color3ub_func *)GetOpenGLProcAddress_Internal(state, "glSecondaryColor3ub");
			glSecondaryColor3ubv = (gl_secondary_color3ubv_func *)GetOpenGLProcAddress_Internal(state, "glSecondaryColor3ubv");
			glSecondaryColor3ui = (gl_secondary_color3ui_func *)GetOpenGLProcAddress_Internal(state, "glSecondaryColor3ui");
			glSecondaryColor3uiv = (gl_secondary_color3uiv_func *)GetOpenGLProcAddress_Internal(state, "glSecondaryColor3uiv");
			glSecondaryColor3us = (gl_secondary_color3us_func *)GetOpenGLProcAddress_Internal(state, "glSecondaryColor3us");
			glSecondaryColor3usv = (gl_secondary_color3usv_func *)GetOpenGLProcAddress_Internal(state, "glSecondaryColor3usv");
			glSecondaryColorPointer = (gl_secondary_color_pointer_func *)GetOpenGLProcAddress_Internal(state, "glSecondaryColorPointer");
			glWindowPos2d = (gl_window_pos2d_func *)GetOpenGLProcAddress_Internal(state, "glWindowPos2d");
			glWindowPos2dv = (gl_window_pos2dv_func *)GetOpenGLProcAddress_Internal(state, "glWindowPos2dv");
			glWindowPos2f = (gl_window_pos2f_func *)GetOpenGLProcAddress_Internal(state, "glWindowPos2f");
			glWindowPos2fv = (gl_window_pos2fv_func *)GetOpenGLProcAddress_Internal(state, "glWindowPos2fv");
			glWindowPos2i = (gl_window_pos2i_func *)GetOpenGLProcAddress_Internal(state, "glWindowPos2i");
			glWindowPos2iv = (gl_window_pos2iv_func *)GetOpenGLProcAddress_Internal(state, "glWindowPos2iv");
			glWindowPos2s = (gl_window_pos2s_func *)GetOpenGLProcAddress_Internal(state, "glWindowPos2s");
			glWindowPos2sv = (gl_window_pos2sv_func *)GetOpenGLProcAddress_Internal(state, "glWindowPos2sv");
			glWindowPos3d = (gl_window_pos3d_func *)GetOpenGLProcAddress_Internal(state, "glWindowPos3d");
			glWindowPos3dv = (gl_window_pos3dv_func *)GetOpenGLProcAddress_Internal(state, "glWindowPos3dv");
			glWindowPos3f = (gl_window_pos3f_func *)GetOpenGLProcAddress_Internal(state, "glWindowPos3f");
			glWindowPos3fv = (gl_window_pos3fv_func *)GetOpenGLProcAddress_Internal(state, "glWindowPos3fv");
			glWindowPos3i = (gl_window_pos3i_func *)GetOpenGLProcAddress_Internal(state, "glWindowPos3i");
			glWindowPos3iv = (gl_window_pos3iv_func *)GetOpenGLProcAddress_Internal(state, "glWindowPos3iv");
			glWindowPos3s = (gl_window_pos3s_func *)GetOpenGLProcAddress_Internal(state, "glWindowPos3s");
			glWindowPos3sv = (gl_window_pos3sv_func *)GetOpenGLProcAddress_Internal(state, "glWindowPos3sv");
			glBlendColor = (gl_blend_color_func *)GetOpenGLProcAddress_Internal(state, "glBlendColor");
			glBlendEquation = (gl_blend_equation_func *)GetOpenGLProcAddress_Internal(state, "glBlendEquation");
#	endif //GL_VERSION_1_4

#	if GL_VERSION_1_5
			glGenQueries = (gl_gen_queries_func *)GetOpenGLProcAddress_Internal(state, "glGenQueries");
			glDeleteQueries = (gl_delete_queries_func *)GetOpenGLProcAddress_Internal(state, "glDeleteQueries");
			glIsQuery = (gl_is_query_func *)GetOpenGLProcAddress_Internal(state, "glIsQuery");
			glBeginQuery = (gl_begin_query_func *)GetOpenGLProcAddress_Internal(state, "glBeginQuery");
			glEndQuery = (gl_end_query_func *)GetOpenGLProcAddress_Internal(state, "glEndQuery");
			glGetQueryiv = (gl_get_queryiv_func *)GetOpenGLProcAddress_Internal(state, "glGetQueryiv");
			glGetQueryObjectiv = (gl_get_query_objectiv_func *)GetOpenGLProcAddress_Internal(state, "glGetQueryObjectiv");
			glGetQueryObjectuiv = (gl_get_query_objectuiv_func *)GetOpenGLProcAddress_Internal(state, "glGetQueryObjectuiv");
			glBindBuffer = (gl_bind_buffer_func *)GetOpenGLProcAddress_Internal(state, "glBindBuffer");
			glDeleteBuffers = (gl_delete_buffers_func *)GetOpenGLProcAddress_Internal(state, "glDeleteBuffers");
			glGenBuffers = (gl_gen_buffers_func *)GetOpenGLProcAddress_Internal(state, "glGenBuffers");
			glIsBuffer = (gl_is_buffer_func *)GetOpenGLProcAddress_Internal(state, "glIsBuffer");
			glBufferData = (gl_buffer_data_func *)GetOpenGLProcAddress_Internal(state, "glBufferData");
			glBufferSubData = (gl_buffer_sub_data_func *)GetOpenGLProcAddress_Internal(state, "glBufferSubData");
			glGetBufferSubData = (gl_get_buffer_sub_data_func *)GetOpenGLProcAddress_Internal(state, "glGetBufferSubData");
			glMapBuffer = (gl_map_buffer_func *)GetOpenGLProcAddress_Internal(state, "glMapBuffer");
			glUnmapBuffer = (gl_unmap_buffer_func *)GetOpenGLProcAddress_Internal(state, "glUnmapBuffer");
			glGetBufferParameteriv = (gl_get_buffer_parameteriv_func *)GetOpenGLProcAddress_Internal(state, "glGetBufferParameteriv");
			glGetBufferPointerv = (gl_get_buffer_pointerv_func *)GetOpenGLProcAddress_Internal(state, "glGetBufferPointerv");
#	endif //GL_VERSION_1_5

#	if GL_VERSION_2_0
			glBlendEquationSeparate = (gl_blend_equation_separate_func *)GetOpenGLProcAddress_Internal(state, "glBlendEquationSeparate");
			glDrawBuffers = (gl_draw_buffers_func *)GetOpenGLProcAddress_Internal(state, "glDrawBuffers");
			glStencilOpSeparate = (gl_stencil_op_separate_func *)GetOpenGLProcAddress_Internal(state, "glStencilOpSeparate");
			glStencilFuncSeparate = (gl_stencil_func_separate_func *)GetOpenGLProcAddress_Internal(state, "glStencilFuncSeparate");
			glStencilMaskSeparate = (gl_stencil_mask_separate_func *)GetOpenGLProcAddress_Internal(state, "glStencilMaskSeparate");
			glAttachShader = (gl_attach_shader_func *)GetOpenGLProcAddress_Internal(state, "glAttachShader");
			glBindAttribLocation = (gl_bind_attrib_location_func *)GetOpenGLProcAddress_Internal(state, "glBindAttribLocation");
			glCompileShader = (gl_compile_shader_func *)GetOpenGLProcAddress_Internal(state, "glCompileShader");
			glCreateProgram = (gl_create_program_func *)GetOpenGLProcAddress_Internal(state, "glCreateProgram");
			glCreateShader = (gl_create_shader_func *)GetOpenGLProcAddress_Internal(state, "glCreateShader");
			glDeleteProgram = (gl_delete_program_func *)GetOpenGLProcAddress_Internal(state, "glDeleteProgram");
			glDeleteShader = (gl_delete_shader_func *)GetOpenGLProcAddress_Internal(state, "glDeleteShader");
			glDetachShader = (gl_detach_shader_func *)GetOpenGLProcAddress_Internal(state, "glDetachShader");
			glDisableVertexAttribArray = (gl_disable_vertex_attrib_array_func *)GetOpenGLProcAddress_Internal(state, "glDisableVertexAttribArray");
			glEnableVertexAttribArray = (gl_enable_vertex_attrib_array_func *)GetOpenGLProcAddress_Internal(state, "glEnableVertexAttribArray");
			glGetActiveAttrib = (gl_get_active_attrib_func *)GetOpenGLProcAddress_Internal(state, "glGetActiveAttrib");
			glGetActiveUniform = (gl_get_active_uniform_func *)GetOpenGLProcAddress_Internal(state, "glGetActiveUniform");
			glGetAttachedShaders = (gl_get_attached_shaders_func *)GetOpenGLProcAddress_Internal(state, "glGetAttachedShaders");
			glGetAttribLocation = (gl_get_attrib_location_func *)GetOpenGLProcAddress_Internal(state, "glGetAttribLocation");
			glGetProgramiv = (gl_get_programiv_func *)GetOpenGLProcAddress_Internal(state, "glGetProgramiv");
			glGetProgramInfoLog = (gl_get_program_info_log_func *)GetOpenGLProcAddress_Internal(state, "glGetProgramInfoLog");
			glGetShaderiv = (gl_get_shaderiv_func *)GetOpenGLProcAddress_Internal(state, "glGetShaderiv");
			glGetShaderInfoLog = (gl_get_shader_info_log_func *)GetOpenGLProcAddress_Internal(state, "glGetShaderInfoLog");
			glGetShaderSource = (gl_get_shader_source_func *)GetOpenGLProcAddress_Internal(state, "glGetShaderSource");
			glGetUniformLocation = (gl_get_uniform_location_func *)GetOpenGLProcAddress_Internal(state, "glGetUniformLocation");
			glGetUniformfv = (gl_get_uniformfv_func *)GetOpenGLProcAddress_Internal(state, "glGetUniformfv");
			glGetUniformiv = (gl_get_uniformiv_func *)GetOpenGLProcAddress_Internal(state, "glGetUniformiv");
			glGetVertexAttribdv = (gl_get_vertex_attribdv_func *)GetOpenGLProcAddress_Internal(state, "glGetVertexAttribdv");
			glGetVertexAttribfv = (gl_get_vertex_attribfv_func *)GetOpenGLProcAddress_Internal(state, "glGetVertexAttribfv");
			glGetVertexAttribiv = (gl_get_vertex_attribiv_func *)GetOpenGLProcAddress_Internal(state, "glGetVertexAttribiv");
			glGetVertexAttribPointerv = (gl_get_vertex_attrib_pointerv_func *)GetOpenGLProcAddress_Internal(state, "glGetVertexAttribPointerv");
			glIsProgram = (gl_is_program_func *)GetOpenGLProcAddress_Internal(state, "glIsProgram");
			glIsShader = (gl_is_shader_func *)GetOpenGLProcAddress_Internal(state, "glIsShader");
			glLinkProgram = (gl_link_program_func *)GetOpenGLProcAddress_Internal(state, "glLinkProgram");
			glShaderSource = (gl_shader_source_func *)GetOpenGLProcAddress_Internal(state, "glShaderSource");
			glUseProgram = (gl_use_program_func *)GetOpenGLProcAddress_Internal(state, "glUseProgram");
			glUniform1f = (gl_uniform1f_func *)GetOpenGLProcAddress_Internal(state, "glUniform1f");
			glUniform2f = (gl_uniform2f_func *)GetOpenGLProcAddress_Internal(state, "glUniform2f");
			glUniform3f = (gl_uniform3f_func *)GetOpenGLProcAddress_Internal(state, "glUniform3f");
			glUniform4f = (gl_uniform4f_func *)GetOpenGLProcAddress_Internal(state, "glUniform4f");
			glUniform1i = (gl_uniform1i_func *)GetOpenGLProcAddress_Internal(state, "glUniform1i");
			glUniform2i = (gl_uniform2i_func *)GetOpenGLProcAddress_Internal(state, "glUniform2i");
			glUniform3i = (gl_uniform3i_func *)GetOpenGLProcAddress_Internal(state, "glUniform3i");
			glUniform4i = (gl_uniform4i_func *)GetOpenGLProcAddress_Internal(state, "glUniform4i");
			glUniform1fv = (gl_uniform1fv_func *)GetOpenGLProcAddress_Internal(state, "glUniform1fv");
			glUniform2fv = (gl_uniform2fv_func *)GetOpenGLProcAddress_Internal(state, "glUniform2fv");
			glUniform3fv = (gl_uniform3fv_func *)GetOpenGLProcAddress_Internal(state, "glUniform3fv");
			glUniform4fv = (gl_uniform4fv_func *)GetOpenGLProcAddress_Internal(state, "glUniform4fv");
			glUniform1iv = (gl_uniform1iv_func *)GetOpenGLProcAddress_Internal(state, "glUniform1iv");
			glUniform2iv = (gl_uniform2iv_func *)GetOpenGLProcAddress_Internal(state, "glUniform2iv");
			glUniform3iv = (gl_uniform3iv_func *)GetOpenGLProcAddress_Internal(state, "glUniform3iv");
			glUniform4iv = (gl_uniform4iv_func *)GetOpenGLProcAddress_Internal(state, "glUniform4iv");
			glUniformMatrix2fv = (gl_uniform_matrix2fv_func *)GetOpenGLProcAddress_Internal(state, "glUniformMatrix2fv");
			glUniformMatrix3fv = (gl_uniform_matrix3fv_func *)GetOpenGLProcAddress_Internal(state, "glUniformMatrix3fv");
			glUniformMatrix4fv = (gl_uniform_matrix4fv_func *)GetOpenGLProcAddress_Internal(state, "glUniformMatrix4fv");
			glValidateProgram = (gl_validate_program_func *)GetOpenGLProcAddress_Internal(state, "glValidateProgram");
			glVertexAttrib1d = (gl_vertex_attrib1d_func *)GetOpenGLProcAddress_Internal(state, "glVertexAttrib1d");
			glVertexAttrib1dv = (gl_vertex_attrib1dv_func *)GetOpenGLProcAddress_Internal(state, "glVertexAttrib1dv");
			glVertexAttrib1f = (gl_vertex_attrib1f_func *)GetOpenGLProcAddress_Internal(state, "glVertexAttrib1f");
			glVertexAttrib1fv = (gl_vertex_attrib1fv_func *)GetOpenGLProcAddress_Internal(state, "glVertexAttrib1fv");
			glVertexAttrib1s = (gl_vertex_attrib1s_func *)GetOpenGLProcAddress_Internal(state, "glVertexAttrib1s");
			glVertexAttrib1sv = (gl_vertex_attrib1sv_func *)GetOpenGLProcAddress_Internal(state, "glVertexAttrib1sv");
			glVertexAttrib2d = (gl_vertex_attrib2d_func *)GetOpenGLProcAddress_Internal(state, "glVertexAttrib2d");
			glVertexAttrib2dv = (gl_vertex_attrib2dv_func *)GetOpenGLProcAddress_Internal(state, "glVertexAttrib2dv");
			glVertexAttrib2f = (gl_vertex_attrib2f_func *)GetOpenGLProcAddress_Internal(state, "glVertexAttrib2f");
			glVertexAttrib2fv = (gl_vertex_attrib2fv_func *)GetOpenGLProcAddress_Internal(state, "glVertexAttrib2fv");
			glVertexAttrib2s = (gl_vertex_attrib2s_func *)GetOpenGLProcAddress_Internal(state, "glVertexAttrib2s");
			glVertexAttrib2sv = (gl_vertex_attrib2sv_func *)GetOpenGLProcAddress_Internal(state, "glVertexAttrib2sv");
			glVertexAttrib3d = (gl_vertex_attrib3d_func *)GetOpenGLProcAddress_Internal(state, "glVertexAttrib3d");
			glVertexAttrib3dv = (gl_vertex_attrib3dv_func *)GetOpenGLProcAddress_Internal(state, "glVertexAttrib3dv");
			glVertexAttrib3f = (gl_vertex_attrib3f_func *)GetOpenGLProcAddress_Internal(state, "glVertexAttrib3f");
			glVertexAttrib3fv = (gl_vertex_attrib3fv_func *)GetOpenGLProcAddress_Internal(state, "glVertexAttrib3fv");
			glVertexAttrib3s = (gl_vertex_attrib3s_func *)GetOpenGLProcAddress_Internal(state, "glVertexAttrib3s");
			glVertexAttrib3sv = (gl_vertex_attrib3sv_func *)GetOpenGLProcAddress_Internal(state, "glVertexAttrib3sv");
			glVertexAttrib4Nbv = (gl_vertex_attrib4_nbv_func *)GetOpenGLProcAddress_Internal(state, "glVertexAttrib4Nbv");
			glVertexAttrib4Niv = (gl_vertex_attrib4_niv_func *)GetOpenGLProcAddress_Internal(state, "glVertexAttrib4Niv");
			glVertexAttrib4Nsv = (gl_vertex_attrib4_nsv_func *)GetOpenGLProcAddress_Internal(state, "glVertexAttrib4Nsv");
			glVertexAttrib4Nub = (gl_vertex_attrib4_nub_func *)GetOpenGLProcAddress_Internal(state, "glVertexAttrib4Nub");
			glVertexAttrib4Nubv = (gl_vertex_attrib4_nubv_func *)GetOpenGLProcAddress_Internal(state, "glVertexAttrib4Nubv");
			glVertexAttrib4Nuiv = (gl_vertex_attrib4_nuiv_func *)GetOpenGLProcAddress_Internal(state, "glVertexAttrib4Nuiv");
			glVertexAttrib4Nusv = (gl_vertex_attrib4_nusv_func *)GetOpenGLProcAddress_Internal(state, "glVertexAttrib4Nusv");
			glVertexAttrib4bv = (gl_vertex_attrib4bv_func *)GetOpenGLProcAddress_Internal(state, "glVertexAttrib4bv");
			glVertexAttrib4d = (gl_vertex_attrib4d_func *)GetOpenGLProcAddress_Internal(state, "glVertexAttrib4d");
			glVertexAttrib4dv = (gl_vertex_attrib4dv_func *)GetOpenGLProcAddress_Internal(state, "glVertexAttrib4dv");
			glVertexAttrib4f = (gl_vertex_attrib4f_func *)GetOpenGLProcAddress_Internal(state, "glVertexAttrib4f");
			glVertexAttrib4fv = (gl_vertex_attrib4fv_func *)GetOpenGLProcAddress_Internal(state, "glVertexAttrib4fv");
			glVertexAttrib4iv = (gl_vertex_attrib4iv_func *)GetOpenGLProcAddress_Internal(state, "glVertexAttrib4iv");
			glVertexAttrib4s = (gl_vertex_attrib4s_func *)GetOpenGLProcAddress_Internal(state, "glVertexAttrib4s");
			glVertexAttrib4sv = (gl_vertex_attrib4sv_func *)GetOpenGLProcAddress_Internal(state, "glVertexAttrib4sv");
			glVertexAttrib4ubv = (gl_vertex_attrib4ubv_func *)GetOpenGLProcAddress_Internal(state, "glVertexAttrib4ubv");
			glVertexAttrib4uiv = (gl_vertex_attrib4uiv_func *)GetOpenGLProcAddress_Internal(state, "glVertexAttrib4uiv");
			glVertexAttrib4usv = (gl_vertex_attrib4usv_func *)GetOpenGLProcAddress_Internal(state, "glVertexAttrib4usv");
			glVertexAttribPointer = (gl_vertex_attrib_pointer_func *)GetOpenGLProcAddress_Internal(state, "glVertexAttribPointer");
#	endif //GL_VERSION_2_0

#	if GL_VERSION_2_1
			glUniformMatrix2x3fv = (gl_uniform_matrix2x3fv_func *)GetOpenGLProcAddress_Internal(state, "glUniformMatrix2x3fv");
			glUniformMatrix3x2fv = (gl_uniform_matrix3x2fv_func *)GetOpenGLProcAddress_Internal(state, "glUniformMatrix3x2fv");
			glUniformMatrix2x4fv = (gl_uniform_matrix2x4fv_func *)GetOpenGLProcAddress_Internal(state, "glUniformMatrix2x4fv");
			glUniformMatrix4x2fv = (gl_uniform_matrix4x2fv_func *)GetOpenGLProcAddress_Internal(state, "glUniformMatrix4x2fv");
			glUniformMatrix3x4fv = (gl_uniform_matrix3x4fv_func *)GetOpenGLProcAddress_Internal(state, "glUniformMatrix3x4fv");
			glUniformMatrix4x3fv = (gl_uniform_matrix4x3fv_func *)GetOpenGLProcAddress_Internal(state, "glUniformMatrix4x3fv");
#	endif //GL_VERSION_2_1

#	if GL_VERSION_3_0
			glColorMaski = (gl_color_maski_func *)GetOpenGLProcAddress_Internal(state, "glColorMaski");
			glGetBooleani_v = (gl_get_booleani_v_func *)GetOpenGLProcAddress_Internal(state, "glGetBooleani_v");
			glGetIntegeri_v = (gl_get_integeri_v_func *)GetOpenGLProcAddress_Internal(state, "glGetIntegeri_v");
			glEnablei = (gl_enablei_func *)GetOpenGLProcAddress_Internal(state, "glEnablei");
			glDisablei = (gl_disablei_func *)GetOpenGLProcAddress_Internal(state, "glDisablei");
			glIsEnabledi = (gl_is_enabledi_func *)GetOpenGLProcAddress_Internal(state, "glIsEnabledi");
			glBeginTransformFeedback = (gl_begin_transform_feedback_func *)GetOpenGLProcAddress_Internal(state, "glBeginTransformFeedback");
			glEndTransformFeedback = (gl_end_transform_feedback_func *)GetOpenGLProcAddress_Internal(state, "glEndTransformFeedback");
			glBindBufferRange = (gl_bind_buffer_range_func *)GetOpenGLProcAddress_Internal(state, "glBindBufferRange");
			glBindBufferBase = (gl_bind_buffer_base_func *)GetOpenGLProcAddress_Internal(state, "glBindBufferBase");
			glTransformFeedbackVaryings = (gl_transform_feedback_varyings_func *)GetOpenGLProcAddress_Internal(state, "glTransformFeedbackVaryings");
			glGetTransformFeedbackVarying = (gl_get_transform_feedback_varying_func *)GetOpenGLProcAddress_Internal(state, "glGetTransformFeedbackVarying");
			glClampColor = (gl_clamp_color_func *)GetOpenGLProcAddress_Internal(state, "glClampColor");
			glBeginConditionalRender = (gl_begin_conditional_render_func *)GetOpenGLProcAddress_Internal(state, "glBeginConditionalRender");
			glEndConditionalRender = (gl_end_conditional_render_func *)GetOpenGLProcAddress_Internal(state, "glEndConditionalRender");
			glVertexAttribIPointer = (gl_vertex_attrib_i_pointer_func *)GetOpenGLProcAddress_Internal(state, "glVertexAttribIPointer");
			glGetVertexAttribIiv = (gl_get_vertex_attrib_iiv_func *)GetOpenGLProcAddress_Internal(state, "glGetVertexAttribIiv");
			glGetVertexAttribIuiv = (gl_get_vertex_attrib_iuiv_func *)GetOpenGLProcAddress_Internal(state, "glGetVertexAttribIuiv");
			glVertexAttribI1i = (gl_vertex_attrib_i1i_func *)GetOpenGLProcAddress_Internal(state, "glVertexAttribI1i");
			glVertexAttribI2i = (gl_vertex_attrib_i2i_func *)GetOpenGLProcAddress_Internal(state, "glVertexAttribI2i");
			glVertexAttribI3i = (gl_vertex_attrib_i3i_func *)GetOpenGLProcAddress_Internal(state, "glVertexAttribI3i");
			glVertexAttribI4i = (gl_vertex_attrib_i4i_func *)GetOpenGLProcAddress_Internal(state, "glVertexAttribI4i");
			glVertexAttribI1ui = (gl_vertex_attrib_i1ui_func *)GetOpenGLProcAddress_Internal(state, "glVertexAttribI1ui");
			glVertexAttribI2ui = (gl_vertex_attrib_i2ui_func *)GetOpenGLProcAddress_Internal(state, "glVertexAttribI2ui");
			glVertexAttribI3ui = (gl_vertex_attrib_i3ui_func *)GetOpenGLProcAddress_Internal(state, "glVertexAttribI3ui");
			glVertexAttribI4ui = (gl_vertex_attrib_i4ui_func *)GetOpenGLProcAddress_Internal(state, "glVertexAttribI4ui");
			glVertexAttribI1iv = (gl_vertex_attrib_i1iv_func *)GetOpenGLProcAddress_Internal(state, "glVertexAttribI1iv");
			glVertexAttribI2iv = (gl_vertex_attrib_i2iv_func *)GetOpenGLProcAddress_Internal(state, "glVertexAttribI2iv");
			glVertexAttribI3iv = (gl_vertex_attrib_i3iv_func *)GetOpenGLProcAddress_Internal(state, "glVertexAttribI3iv");
			glVertexAttribI4iv = (gl_vertex_attrib_i4iv_func *)GetOpenGLProcAddress_Internal(state, "glVertexAttribI4iv");
			glVertexAttribI1uiv = (gl_vertex_attrib_i1uiv_func *)GetOpenGLProcAddress_Internal(state, "glVertexAttribI1uiv");
			glVertexAttribI2uiv = (gl_vertex_attrib_i2uiv_func *)GetOpenGLProcAddress_Internal(state, "glVertexAttribI2uiv");
			glVertexAttribI3uiv = (gl_vertex_attrib_i3uiv_func *)GetOpenGLProcAddress_Internal(state, "glVertexAttribI3uiv");
			glVertexAttribI4uiv = (gl_vertex_attrib_i4uiv_func *)GetOpenGLProcAddress_Internal(state, "glVertexAttribI4uiv");
			glVertexAttribI4bv = (gl_vertex_attrib_i4bv_func *)GetOpenGLProcAddress_Internal(state, "glVertexAttribI4bv");
			glVertexAttribI4sv = (gl_vertex_attrib_i4sv_func *)GetOpenGLProcAddress_Internal(state, "glVertexAttribI4sv");
			glVertexAttribI4ubv = (gl_vertex_attrib_i4ubv_func *)GetOpenGLProcAddress_Internal(state, "glVertexAttribI4ubv");
			glVertexAttribI4usv = (gl_vertex_attrib_i4usv_func *)GetOpenGLProcAddress_Internal(state, "glVertexAttribI4usv");
			glGetUniformuiv = (gl_get_uniformuiv_func *)GetOpenGLProcAddress_Internal(state, "glGetUniformuiv");
			glBindFragDataLocation = (gl_bind_frag_data_location_func *)GetOpenGLProcAddress_Internal(state, "glBindFragDataLocation");
			glGetFragDataLocation = (gl_get_frag_data_location_func *)GetOpenGLProcAddress_Internal(state, "glGetFragDataLocation");
			glUniform1ui = (gl_uniform1ui_func *)GetOpenGLProcAddress_Internal(state, "glUniform1ui");
			glUniform2ui = (gl_uniform2ui_func *)GetOpenGLProcAddress_Internal(state, "glUniform2ui");
			glUniform3ui = (gl_uniform3ui_func *)GetOpenGLProcAddress_Internal(state, "glUniform3ui");
			glUniform4ui = (gl_uniform4ui_func *)GetOpenGLProcAddress_Internal(state, "glUniform4ui");
			glUniform1uiv = (gl_uniform1uiv_func *)GetOpenGLProcAddress_Internal(state, "glUniform1uiv");
			glUniform2uiv = (gl_uniform2uiv_func *)GetOpenGLProcAddress_Internal(state, "glUniform2uiv");
			glUniform3uiv = (gl_uniform3uiv_func *)GetOpenGLProcAddress_Internal(state, "glUniform3uiv");
			glUniform4uiv = (gl_uniform4uiv_func *)GetOpenGLProcAddress_Internal(state, "glUniform4uiv");
			glTexParameterIiv = (gl_tex_parameter_iiv_func *)GetOpenGLProcAddress_Internal(state, "glTexParameterIiv");
			glTexParameterIuiv = (gl_tex_parameter_iuiv_func *)GetOpenGLProcAddress_Internal(state, "glTexParameterIuiv");
			glGetTexParameterIiv = (gl_get_tex_parameter_iiv_func *)GetOpenGLProcAddress_Internal(state, "glGetTexParameterIiv");
			glGetTexParameterIuiv = (gl_get_tex_parameter_iuiv_func *)GetOpenGLProcAddress_Internal(state, "glGetTexParameterIuiv");
			glClearBufferiv = (gl_clear_bufferiv_func *)GetOpenGLProcAddress_Internal(state, "glClearBufferiv");
			glClearBufferuiv = (gl_clear_bufferuiv_func *)GetOpenGLProcAddress_Internal(state, "glClearBufferuiv");
			glClearBufferfv = (gl_clear_bufferfv_func *)GetOpenGLProcAddress_Internal(state, "glClearBufferfv");
			glClearBufferfi = (gl_clear_bufferfi_func *)GetOpenGLProcAddress_Internal(state, "glClearBufferfi");
			glGetStringi = (gl_get_stringi_func *)GetOpenGLProcAddress_Internal(state, "glGetStringi");
			glIsRenderbuffer = (gl_is_renderbuffer_func *)GetOpenGLProcAddress_Internal(state, "glIsRenderbuffer");
			glBindRenderbuffer = (gl_bind_renderbuffer_func *)GetOpenGLProcAddress_Internal(state, "glBindRenderbuffer");
			glDeleteRenderbuffers = (gl_delete_renderbuffers_func *)GetOpenGLProcAddress_Internal(state, "glDeleteRenderbuffers");
			glGenRenderbuffers = (gl_gen_renderbuffers_func *)GetOpenGLProcAddress_Internal(state, "glGenRenderbuffers");
			glRenderbufferStorage = (gl_renderbuffer_storage_func *)GetOpenGLProcAddress_Internal(state, "glRenderbufferStorage");
			glGetRenderbufferParameteriv = (gl_get_renderbuffer_parameteriv_func *)GetOpenGLProcAddress_Internal(state, "glGetRenderbufferParameteriv");
			glIsFramebuffer = (gl_is_framebuffer_func *)GetOpenGLProcAddress_Internal(state, "glIsFramebuffer");
			glBindFramebuffer = (gl_bind_framebuffer_func *)GetOpenGLProcAddress_Internal(state, "glBindFramebuffer");
			glDeleteFramebuffers = (gl_delete_framebuffers_func *)GetOpenGLProcAddress_Internal(state, "glDeleteFramebuffers");
			glGenFramebuffers = (gl_gen_framebuffers_func *)GetOpenGLProcAddress_Internal(state, "glGenFramebuffers");
			glCheckFramebufferStatus = (gl_check_framebuffer_status_func *)GetOpenGLProcAddress_Internal(state, "glCheckFramebufferStatus");
			glFramebufferTexture1D = (gl_framebuffer_texture1d_func *)GetOpenGLProcAddress_Internal(state, "glFramebufferTexture1D");
			glFramebufferTexture2D = (gl_framebuffer_texture2d_func *)GetOpenGLProcAddress_Internal(state, "glFramebufferTexture2D");
			glFramebufferTexture3D = (gl_framebuffer_texture3d_func *)GetOpenGLProcAddress_Internal(state, "glFramebufferTexture3D");
			glFramebufferRenderbuffer = (gl_framebuffer_renderbuffer_func *)GetOpenGLProcAddress_Internal(state, "glFramebufferRenderbuffer");
			glGetFramebufferAttachmentParameteriv = (gl_get_framebuffer_attachment_parameteriv_func *)GetOpenGLProcAddress_Internal(state, "glGetFramebufferAttachmentParameteriv");
			glGenerateMipmap = (gl_generate_mipmap_func *)GetOpenGLProcAddress_Internal(state, "glGenerateMipmap");
			glBlitFramebuffer = (gl_blit_framebuffer_func *)GetOpenGLProcAddress_Internal(state, "glBlitFramebuffer");
			glRenderbufferStorageMultisample = (gl_renderbuffer_storage_multisample_func *)GetOpenGLProcAddress_Internal(state, "glRenderbufferStorageMultisample");
			glFramebufferTextureLayer = (gl_framebuffer_texture_layer_func *)GetOpenGLProcAddress_Internal(state, "glFramebufferTextureLayer");
			glMapBufferRange = (gl_map_buffer_range_func *)GetOpenGLProcAddress_Internal(state, "glMapBufferRange");
			glFlushMappedBufferRange = (gl_flush_mapped_buffer_range_func *)GetOpenGLProcAddress_Internal(state, "glFlushMappedBufferRange");
			glBindVertexArray = (gl_bind_vertex_array_func *)GetOpenGLProcAddress_Internal(state, "glBindVertexArray");
			glDeleteVertexArrays = (gl_delete_vertex_arrays_func *)GetOpenGLProcAddress_Internal(state, "glDeleteVertexArrays");
			glGenVertexArrays = (gl_gen_vertex_arrays_func *)GetOpenGLProcAddress_Internal(state, "glGenVertexArrays");
			glIsVertexArray = (gl_is_vertex_array_func *)GetOpenGLProcAddress_Internal(state, "glIsVertexArray");
#	endif //GL_VERSION_3_0

#	if GL_VERSION_3_1
			glDrawArraysInstanced = (gl_draw_arrays_instanced_func *)GetOpenGLProcAddress_Internal(state, "glDrawArraysInstanced");
			glDrawElementsInstanced = (gl_draw_elements_instanced_func *)GetOpenGLProcAddress_Internal(state, "glDrawElementsInstanced");
			glTexBuffer = (gl_tex_buffer_func *)GetOpenGLProcAddress_Internal(state, "glTexBuffer");
			glPrimitiveRestartIndex = (gl_primitive_restart_index_func *)GetOpenGLProcAddress_Internal(state, "glPrimitiveRestartIndex");
			glCopyBufferSubData = (gl_copy_buffer_sub_data_func *)GetOpenGLProcAddress_Internal(state, "glCopyBufferSubData");
			glGetUniformIndices = (gl_get_uniform_indices_func *)GetOpenGLProcAddress_Internal(state, "glGetUniformIndices");
			glGetActiveUniformsiv = (gl_get_active_uniformsiv_func *)GetOpenGLProcAddress_Internal(state, "glGetActiveUniformsiv");
			glGetActiveUniformName = (gl_get_active_uniform_name_func *)GetOpenGLProcAddress_Internal(state, "glGetActiveUniformName");
			glGetUniformBlockIndex = (gl_get_uniform_block_index_func *)GetOpenGLProcAddress_Internal(state, "glGetUniformBlockIndex");
			glGetActiveUniformBlockiv = (gl_get_active_uniform_blockiv_func *)GetOpenGLProcAddress_Internal(state, "glGetActiveUniformBlockiv");
			glGetActiveUniformBlockName = (gl_get_active_uniform_block_name_func *)GetOpenGLProcAddress_Internal(state, "glGetActiveUniformBlockName");
			glUniformBlockBinding = (gl_uniform_block_binding_func *)GetOpenGLProcAddress_Internal(state, "glUniformBlockBinding");
#	endif //GL_VERSION_3_1

#	if GL_VERSION_3_2
			glDrawElementsBaseVertex = (gl_draw_elements_base_vertex_func *)GetOpenGLProcAddress_Internal(state, "glDrawElementsBaseVertex");
			glDrawRangeElementsBaseVertex = (gl_draw_range_elements_base_vertex_func *)GetOpenGLProcAddress_Internal(state, "glDrawRangeElementsBaseVertex");
			glDrawElementsInstancedBaseVertex = (gl_draw_elements_instanced_base_vertex_func *)GetOpenGLProcAddress_Internal(state, "glDrawElementsInstancedBaseVertex");
			glMultiDrawElementsBaseVertex = (gl_multi_draw_elements_base_vertex_func *)GetOpenGLProcAddress_Internal(state, "glMultiDrawElementsBaseVertex");
			glProvokingVertex = (gl_provoking_vertex_func *)GetOpenGLProcAddress_Internal(state, "glProvokingVertex");
			glFenceSync = (gl_fence_sync_func *)GetOpenGLProcAddress_Internal(state, "glFenceSync");
			glIsSync = (gl_is_sync_func *)GetOpenGLProcAddress_Internal(state, "glIsSync");
			glDeleteSync = (gl_delete_sync_func *)GetOpenGLProcAddress_Internal(state, "glDeleteSync");
			glClientWaitSync = (gl_client_wait_sync_func *)GetOpenGLProcAddress_Internal(state, "glClientWaitSync");
			glWaitSync = (gl_wait_sync_func *)GetOpenGLProcAddress_Internal(state, "glWaitSync");
			glGetInteger64v = (gl_get_integer64v_func *)GetOpenGLProcAddress_Internal(state, "glGetInteger64v");
			glGetSynciv = (gl_get_synciv_func *)GetOpenGLProcAddress_Internal(state, "glGetSynciv");
			glGetInteger64i_v = (gl_get_integer64i_v_func *)GetOpenGLProcAddress_Internal(state, "glGetInteger64i_v");
			glGetBufferParameteri64v = (gl_get_buffer_parameteri64v_func *)GetOpenGLProcAddress_Internal(state, "glGetBufferParameteri64v");
			glFramebufferTexture = (gl_framebuffer_texture_func *)GetOpenGLProcAddress_Internal(state, "glFramebufferTexture");
			glTexImage2DMultisample = (gl_tex_image2_d_multisample_func *)GetOpenGLProcAddress_Internal(state, "glTexImage2DMultisample");
			glTexImage3DMultisample = (gl_tex_image3_d_multisample_func *)GetOpenGLProcAddress_Internal(state, "glTexImage3DMultisample");
			glGetMultisamplefv = (gl_get_multisamplefv_func *)GetOpenGLProcAddress_Internal(state, "glGetMultisamplefv");
			glSampleMaski = (gl_sample_maski_func *)GetOpenGLProcAddress_Internal(state, "glSampleMaski");
#	endif //GL_VERSION_3_2

#	if GL_VERSION_3_3
			glBindFragDataLocationIndexed = (gl_bind_frag_data_location_indexed_func *)GetOpenGLProcAddress_Internal(state, "glBindFragDataLocationIndexed");
			glGetFragDataIndex = (gl_get_frag_data_index_func *)GetOpenGLProcAddress_Internal(state, "glGetFragDataIndex");
			glGenSamplers = (gl_gen_samplers_func *)GetOpenGLProcAddress_Internal(state, "glGenSamplers");
			glDeleteSamplers = (gl_delete_samplers_func *)GetOpenGLProcAddress_Internal(state, "glDeleteSamplers");
			glIsSampler = (gl_is_sampler_func *)GetOpenGLProcAddress_Internal(state, "glIsSampler");
			glBindSampler = (gl_bind_sampler_func *)GetOpenGLProcAddress_Internal(state, "glBindSampler");
			glSamplerParameteri = (gl_sampler_parameteri_func *)GetOpenGLProcAddress_Internal(state, "glSamplerParameteri");
			glSamplerParameteriv = (gl_sampler_parameteriv_func *)GetOpenGLProcAddress_Internal(state, "glSamplerParameteriv");
			glSamplerParameterf = (gl_sampler_parameterf_func *)GetOpenGLProcAddress_Internal(state, "glSamplerParameterf");
			glSamplerParameterfv = (gl_sampler_parameterfv_func *)GetOpenGLProcAddress_Internal(state, "glSamplerParameterfv");
			glSamplerParameterIiv = (gl_sampler_parameter_iiv_func *)GetOpenGLProcAddress_Internal(state, "glSamplerParameterIiv");
			glSamplerParameterIuiv = (gl_sampler_parameter_iuiv_func *)GetOpenGLProcAddress_Internal(state, "glSamplerParameterIuiv");
			glGetSamplerParameteriv = (gl_get_sampler_parameteriv_func *)GetOpenGLProcAddress_Internal(state, "glGetSamplerParameteriv");
			glGetSamplerParameterIiv = (gl_get_sampler_parameter_iiv_func *)GetOpenGLProcAddress_Internal(state, "glGetSamplerParameterIiv");
			glGetSamplerParameterfv = (gl_get_sampler_parameterfv_func *)GetOpenGLProcAddress_Internal(state, "glGetSamplerParameterfv");
			glGetSamplerParameterIuiv = (gl_get_sampler_parameter_iuiv_func *)GetOpenGLProcAddress_Internal(state, "glGetSamplerParameterIuiv");
			glQueryCounter = (gl_query_counter_func *)GetOpenGLProcAddress_Internal(state, "glQueryCounter");
			glGetQueryObjecti64v = (gl_get_query_objecti64v_func *)GetOpenGLProcAddress_Internal(state, "glGetQueryObjecti64v");
			glGetQueryObjectui64v = (gl_get_query_objectui64v_func *)GetOpenGLProcAddress_Internal(state, "glGetQueryObjectui64v");
			glVertexAttribDivisor = (gl_vertex_attrib_divisor_func *)GetOpenGLProcAddress_Internal(state, "glVertexAttribDivisor");
			glVertexAttribP1ui = (gl_vertex_attrib_p1ui_func *)GetOpenGLProcAddress_Internal(state, "glVertexAttribP1ui");
			glVertexAttribP1uiv = (gl_vertex_attrib_p1uiv_func *)GetOpenGLProcAddress_Internal(state, "glVertexAttribP1uiv");
			glVertexAttribP2ui = (gl_vertex_attrib_p2ui_func *)GetOpenGLProcAddress_Internal(state, "glVertexAttribP2ui");
			glVertexAttribP2uiv = (gl_vertex_attrib_p2uiv_func *)GetOpenGLProcAddress_Internal(state, "glVertexAttribP2uiv");
			glVertexAttribP3ui = (gl_vertex_attrib_p3ui_func *)GetOpenGLProcAddress_Internal(state, "glVertexAttribP3ui");
			glVertexAttribP3uiv = (gl_vertex_attrib_p3uiv_func *)GetOpenGLProcAddress_Internal(state, "glVertexAttribP3uiv");
			glVertexAttribP4ui = (gl_vertex_attrib_p4ui_func *)GetOpenGLProcAddress_Internal(state, "glVertexAttribP4ui");
			glVertexAttribP4uiv = (gl_vertex_attrib_p4uiv_func *)GetOpenGLProcAddress_Internal(state, "glVertexAttribP4uiv");
			glVertexP2ui = (gl_vertex_p2ui_func *)GetOpenGLProcAddress_Internal(state, "glVertexP2ui");
			glVertexP2uiv = (gl_vertex_p2uiv_func *)GetOpenGLProcAddress_Internal(state, "glVertexP2uiv");
			glVertexP3ui = (gl_vertex_p3ui_func *)GetOpenGLProcAddress_Internal(state, "glVertexP3ui");
			glVertexP3uiv = (gl_vertex_p3uiv_func *)GetOpenGLProcAddress_Internal(state, "glVertexP3uiv");
			glVertexP4ui = (gl_vertex_p4ui_func *)GetOpenGLProcAddress_Internal(state, "glVertexP4ui");
			glVertexP4uiv = (gl_vertex_p4uiv_func *)GetOpenGLProcAddress_Internal(state, "glVertexP4uiv");
			glTexCoordP1ui = (gl_tex_coord_p1ui_func *)GetOpenGLProcAddress_Internal(state, "glTexCoordP1ui");
			glTexCoordP1uiv = (gl_tex_coord_p1uiv_func *)GetOpenGLProcAddress_Internal(state, "glTexCoordP1uiv");
			glTexCoordP2ui = (gl_tex_coord_p2ui_func *)GetOpenGLProcAddress_Internal(state, "glTexCoordP2ui");
			glTexCoordP2uiv = (gl_tex_coord_p2uiv_func *)GetOpenGLProcAddress_Internal(state, "glTexCoordP2uiv");
			glTexCoordP3ui = (gl_tex_coord_p3ui_func *)GetOpenGLProcAddress_Internal(state, "glTexCoordP3ui");
			glTexCoordP3uiv = (gl_tex_coord_p3uiv_func *)GetOpenGLProcAddress_Internal(state, "glTexCoordP3uiv");
			glTexCoordP4ui = (gl_tex_coord_p4ui_func *)GetOpenGLProcAddress_Internal(state, "glTexCoordP4ui");
			glTexCoordP4uiv = (gl_tex_coord_p4uiv_func *)GetOpenGLProcAddress_Internal(state, "glTexCoordP4uiv");
			glMultiTexCoordP1ui = (gl_multi_tex_coord_p1ui_func *)GetOpenGLProcAddress_Internal(state, "glMultiTexCoordP1ui");
			glMultiTexCoordP1uiv = (gl_multi_tex_coord_p1uiv_func *)GetOpenGLProcAddress_Internal(state, "glMultiTexCoordP1uiv");
			glMultiTexCoordP2ui = (gl_multi_tex_coord_p2ui_func *)GetOpenGLProcAddress_Internal(state, "glMultiTexCoordP2ui");
			glMultiTexCoordP2uiv = (gl_multi_tex_coord_p2uiv_func *)GetOpenGLProcAddress_Internal(state, "glMultiTexCoordP2uiv");
			glMultiTexCoordP3ui = (gl_multi_tex_coord_p3ui_func *)GetOpenGLProcAddress_Internal(state, "glMultiTexCoordP3ui");
			glMultiTexCoordP3uiv = (gl_multi_tex_coord_p3uiv_func *)GetOpenGLProcAddress_Internal(state, "glMultiTexCoordP3uiv");
			glMultiTexCoordP4ui = (gl_multi_tex_coord_p4ui_func *)GetOpenGLProcAddress_Internal(state, "glMultiTexCoordP4ui");
			glMultiTexCoordP4uiv = (gl_multi_tex_coord_p4uiv_func *)GetOpenGLProcAddress_Internal(state, "glMultiTexCoordP4uiv");
			glNormalP3ui = (gl_normal_p3ui_func *)GetOpenGLProcAddress_Internal(state, "glNormalP3ui");
			glNormalP3uiv = (gl_normal_p3uiv_func *)GetOpenGLProcAddress_Internal(state, "glNormalP3uiv");
			glColorP3ui = (gl_color_p3ui_func *)GetOpenGLProcAddress_Internal(state, "glColorP3ui");
			glColorP3uiv = (gl_color_p3uiv_func *)GetOpenGLProcAddress_Internal(state, "glColorP3uiv");
			glColorP4ui = (gl_color_p4ui_func *)GetOpenGLProcAddress_Internal(state, "glColorP4ui");
			glColorP4uiv = (gl_color_p4uiv_func *)GetOpenGLProcAddress_Internal(state, "glColorP4uiv");
			glSecondaryColorP3ui = (gl_secondary_color_p3ui_func *)GetOpenGLProcAddress_Internal(state, "glSecondaryColorP3ui");
			glSecondaryColorP3uiv = (gl_secondary_color_p3uiv_func *)GetOpenGLProcAddress_Internal(state, "glSecondaryColorP3uiv");
#	endif //GL_VERSION_3_3

#	if GL_VERSION_4_0
			glMinSampleShading = (gl_min_sample_shading_func *)GetOpenGLProcAddress_Internal(state, "glMinSampleShading");
			glBlendEquationi = (gl_blend_equationi_func *)GetOpenGLProcAddress_Internal(state, "glBlendEquationi");
			glBlendEquationSeparatei = (gl_blend_equation_separatei_func *)GetOpenGLProcAddress_Internal(state, "glBlendEquationSeparatei");
			glBlendFunci = (gl_blend_funci_func *)GetOpenGLProcAddress_Internal(state, "glBlendFunci");
			glBlendFuncSeparatei = (gl_blend_func_separatei_func *)GetOpenGLProcAddress_Internal(state, "glBlendFuncSeparatei");
			glDrawArraysIndirect = (gl_draw_arrays_indirect_func *)GetOpenGLProcAddress_Internal(state, "glDrawArraysIndirect");
			glDrawElementsIndirect = (gl_draw_elements_indirect_func *)GetOpenGLProcAddress_Internal(state, "glDrawElementsIndirect");
			glUniform1d = (gl_uniform1d_func *)GetOpenGLProcAddress_Internal(state, "glUniform1d");
			glUniform2d = (gl_uniform2d_func *)GetOpenGLProcAddress_Internal(state, "glUniform2d");
			glUniform3d = (gl_uniform3d_func *)GetOpenGLProcAddress_Internal(state, "glUniform3d");
			glUniform4d = (gl_uniform4d_func *)GetOpenGLProcAddress_Internal(state, "glUniform4d");
			glUniform1dv = (gl_uniform1dv_func *)GetOpenGLProcAddress_Internal(state, "glUniform1dv");
			glUniform2dv = (gl_uniform2dv_func *)GetOpenGLProcAddress_Internal(state, "glUniform2dv");
			glUniform3dv = (gl_uniform3dv_func *)GetOpenGLProcAddress_Internal(state, "glUniform3dv");
			glUniform4dv = (gl_uniform4dv_func *)GetOpenGLProcAddress_Internal(state, "glUniform4dv");
			glUniformMatrix2dv = (gl_uniform_matrix2dv_func *)GetOpenGLProcAddress_Internal(state, "glUniformMatrix2dv");
			glUniformMatrix3dv = (gl_uniform_matrix3dv_func *)GetOpenGLProcAddress_Internal(state, "glUniformMatrix3dv");
			glUniformMatrix4dv = (gl_uniform_matrix4dv_func *)GetOpenGLProcAddress_Internal(state, "glUniformMatrix4dv");
			glUniformMatrix2x3dv = (gl_uniform_matrix2x3dv_func *)GetOpenGLProcAddress_Internal(state, "glUniformMatrix2x3dv");
			glUniformMatrix2x4dv = (gl_uniform_matrix2x4dv_func *)GetOpenGLProcAddress_Internal(state, "glUniformMatrix2x4dv");
			glUniformMatrix3x2dv = (gl_uniform_matrix3x2dv_func *)GetOpenGLProcAddress_Internal(state, "glUniformMatrix3x2dv");
			glUniformMatrix3x4dv = (gl_uniform_matrix3x4dv_func *)GetOpenGLProcAddress_Internal(state, "glUniformMatrix3x4dv");
			glUniformMatrix4x2dv = (gl_uniform_matrix4x2dv_func *)GetOpenGLProcAddress_Internal(state, "glUniformMatrix4x2dv");
			glUniformMatrix4x3dv = (gl_uniform_matrix4x3dv_func *)GetOpenGLProcAddress_Internal(state, "glUniformMatrix4x3dv");
			glGetUniformdv = (gl_get_uniformdv_func *)GetOpenGLProcAddress_Internal(state, "glGetUniformdv");
			glGetSubroutineUniformLocation = (gl_get_subroutine_uniform_location_func *)GetOpenGLProcAddress_Internal(state, "glGetSubroutineUniformLocation");
			glGetSubroutineIndex = (gl_get_subroutine_index_func *)GetOpenGLProcAddress_Internal(state, "glGetSubroutineIndex");
			glGetActiveSubroutineUniformiv = (gl_get_active_subroutine_uniformiv_func *)GetOpenGLProcAddress_Internal(state, "glGetActiveSubroutineUniformiv");
			glGetActiveSubroutineUniformName = (gl_get_active_subroutine_uniform_name_func *)GetOpenGLProcAddress_Internal(state, "glGetActiveSubroutineUniformName");
			glGetActiveSubroutineName = (gl_get_active_subroutine_name_func *)GetOpenGLProcAddress_Internal(state, "glGetActiveSubroutineName");
			glUniformSubroutinesuiv = (gl_uniform_subroutinesuiv_func *)GetOpenGLProcAddress_Internal(state, "glUniformSubroutinesuiv");
			glGetUniformSubroutineuiv = (gl_get_uniform_subroutineuiv_func *)GetOpenGLProcAddress_Internal(state, "glGetUniformSubroutineuiv");
			glGetProgramStageiv = (gl_get_program_stageiv_func *)GetOpenGLProcAddress_Internal(state, "glGetProgramStageiv");
			glPatchParameteri = (gl_patch_parameteri_func *)GetOpenGLProcAddress_Internal(state, "glPatchParameteri");
			glPatchParameterfv = (gl_patch_parameterfv_func *)GetOpenGLProcAddress_Internal(state, "glPatchParameterfv");
			glBindTransformFeedback = (gl_bind_transform_feedback_func *)GetOpenGLProcAddress_Internal(state, "glBindTransformFeedback");
			glDeleteTransformFeedbacks = (gl_delete_transform_feedbacks_func *)GetOpenGLProcAddress_Internal(state, "glDeleteTransformFeedbacks");
			glGenTransformFeedbacks = (gl_gen_transform_feedbacks_func *)GetOpenGLProcAddress_Internal(state, "glGenTransformFeedbacks");
			glIsTransformFeedback = (gl_is_transform_feedback_func *)GetOpenGLProcAddress_Internal(state, "glIsTransformFeedback");
			glPauseTransformFeedback = (gl_pause_transform_feedback_func *)GetOpenGLProcAddress_Internal(state, "glPauseTransformFeedback");
			glResumeTransformFeedback = (gl_resume_transform_feedback_func *)GetOpenGLProcAddress_Internal(state, "glResumeTransformFeedback");
			glDrawTransformFeedback = (gl_draw_transform_feedback_func *)GetOpenGLProcAddress_Internal(state, "glDrawTransformFeedback");
			glDrawTransformFeedbackStream = (gl_draw_transform_feedback_stream_func *)GetOpenGLProcAddress_Internal(state, "glDrawTransformFeedbackStream");
			glBeginQueryIndexed = (gl_begin_query_indexed_func *)GetOpenGLProcAddress_Internal(state, "glBeginQueryIndexed");
			glEndQueryIndexed = (gl_end_query_indexed_func *)GetOpenGLProcAddress_Internal(state, "glEndQueryIndexed");
			glGetQueryIndexediv = (gl_get_query_indexediv_func *)GetOpenGLProcAddress_Internal(state, "glGetQueryIndexediv");
#	endif //GL_VERSION_4_0

#	if GL_VERSION_4_1
			glReleaseShaderCompiler = (gl_release_shader_compiler_func *)GetOpenGLProcAddress_Internal(state, "glReleaseShaderCompiler");
			glShaderBinary = (gl_shader_binary_func *)GetOpenGLProcAddress_Internal(state, "glShaderBinary");
			glGetShaderPrecisionFormat = (gl_get_shader_precision_format_func *)GetOpenGLProcAddress_Internal(state, "glGetShaderPrecisionFormat");
			glDepthRangef = (gl_depth_rangef_func *)GetOpenGLProcAddress_Internal(state, "glDepthRangef");
			glClearDepthf = (gl_clear_depthf_func *)GetOpenGLProcAddress_Internal(state, "glClearDepthf");
			glGetProgramBinary = (gl_get_program_binary_func *)GetOpenGLProcAddress_Internal(state, "glGetProgramBinary");
			glProgramBinary = (gl_program_binary_func *)GetOpenGLProcAddress_Internal(state, "glProgramBinary");
			glProgramParameteri = (gl_program_parameteri_func *)GetOpenGLProcAddress_Internal(state, "glProgramParameteri");
			glUseProgramStages = (gl_use_program_stages_func *)GetOpenGLProcAddress_Internal(state, "glUseProgramStages");
			glActiveShaderProgram = (gl_active_shader_program_func *)GetOpenGLProcAddress_Internal(state, "glActiveShaderProgram");
			glCreateShaderProgramv = (gl_create_shader_programv_func *)GetOpenGLProcAddress_Internal(state, "glCreateShaderProgramv");
			glBindProgramPipeline = (gl_bind_program_pipeline_func *)GetOpenGLProcAddress_Internal(state, "glBindProgramPipeline");
			glDeleteProgramPipelines = (gl_delete_program_pipelines_func *)GetOpenGLProcAddress_Internal(state, "glDeleteProgramPipelines");
			glGenProgramPipelines = (gl_gen_program_pipelines_func *)GetOpenGLProcAddress_Internal(state, "glGenProgramPipelines");
			glIsProgramPipeline = (gl_is_program_pipeline_func *)GetOpenGLProcAddress_Internal(state, "glIsProgramPipeline");
			glGetProgramPipelineiv = (gl_get_program_pipelineiv_func *)GetOpenGLProcAddress_Internal(state, "glGetProgramPipelineiv");
			glProgramUniform1i = (gl_program_uniform1i_func *)GetOpenGLProcAddress_Internal(state, "glProgramUniform1i");
			glProgramUniform1iv = (gl_program_uniform1iv_func *)GetOpenGLProcAddress_Internal(state, "glProgramUniform1iv");
			glProgramUniform1f = (gl_program_uniform1f_func *)GetOpenGLProcAddress_Internal(state, "glProgramUniform1f");
			glProgramUniform1fv = (gl_program_uniform1fv_func *)GetOpenGLProcAddress_Internal(state, "glProgramUniform1fv");
			glProgramUniform1d = (gl_program_uniform1d_func *)GetOpenGLProcAddress_Internal(state, "glProgramUniform1d");
			glProgramUniform1dv = (gl_program_uniform1dv_func *)GetOpenGLProcAddress_Internal(state, "glProgramUniform1dv");
			glProgramUniform1ui = (gl_program_uniform1ui_func *)GetOpenGLProcAddress_Internal(state, "glProgramUniform1ui");
			glProgramUniform1uiv = (gl_program_uniform1uiv_func *)GetOpenGLProcAddress_Internal(state, "glProgramUniform1uiv");
			glProgramUniform2i = (gl_program_uniform2i_func *)GetOpenGLProcAddress_Internal(state, "glProgramUniform2i");
			glProgramUniform2iv = (gl_program_uniform2iv_func *)GetOpenGLProcAddress_Internal(state, "glProgramUniform2iv");
			glProgramUniform2f = (gl_program_uniform2f_func *)GetOpenGLProcAddress_Internal(state, "glProgramUniform2f");
			glProgramUniform2fv = (gl_program_uniform2fv_func *)GetOpenGLProcAddress_Internal(state, "glProgramUniform2fv");
			glProgramUniform2d = (gl_program_uniform2d_func *)GetOpenGLProcAddress_Internal(state, "glProgramUniform2d");
			glProgramUniform2dv = (gl_program_uniform2dv_func *)GetOpenGLProcAddress_Internal(state, "glProgramUniform2dv");
			glProgramUniform2ui = (gl_program_uniform2ui_func *)GetOpenGLProcAddress_Internal(state, "glProgramUniform2ui");
			glProgramUniform2uiv = (gl_program_uniform2uiv_func *)GetOpenGLProcAddress_Internal(state, "glProgramUniform2uiv");
			glProgramUniform3i = (gl_program_uniform3i_func *)GetOpenGLProcAddress_Internal(state, "glProgramUniform3i");
			glProgramUniform3iv = (gl_program_uniform3iv_func *)GetOpenGLProcAddress_Internal(state, "glProgramUniform3iv");
			glProgramUniform3f = (gl_program_uniform3f_func *)GetOpenGLProcAddress_Internal(state, "glProgramUniform3f");
			glProgramUniform3fv = (gl_program_uniform3fv_func *)GetOpenGLProcAddress_Internal(state, "glProgramUniform3fv");
			glProgramUniform3d = (gl_program_uniform3d_func *)GetOpenGLProcAddress_Internal(state, "glProgramUniform3d");
			glProgramUniform3dv = (gl_program_uniform3dv_func *)GetOpenGLProcAddress_Internal(state, "glProgramUniform3dv");
			glProgramUniform3ui = (gl_program_uniform3ui_func *)GetOpenGLProcAddress_Internal(state, "glProgramUniform3ui");
			glProgramUniform3uiv = (gl_program_uniform3uiv_func *)GetOpenGLProcAddress_Internal(state, "glProgramUniform3uiv");
			glProgramUniform4i = (gl_program_uniform4i_func *)GetOpenGLProcAddress_Internal(state, "glProgramUniform4i");
			glProgramUniform4iv = (gl_program_uniform4iv_func *)GetOpenGLProcAddress_Internal(state, "glProgramUniform4iv");
			glProgramUniform4f = (gl_program_uniform4f_func *)GetOpenGLProcAddress_Internal(state, "glProgramUniform4f");
			glProgramUniform4fv = (gl_program_uniform4fv_func *)GetOpenGLProcAddress_Internal(state, "glProgramUniform4fv");
			glProgramUniform4d = (gl_program_uniform4d_func *)GetOpenGLProcAddress_Internal(state, "glProgramUniform4d");
			glProgramUniform4dv = (gl_program_uniform4dv_func *)GetOpenGLProcAddress_Internal(state, "glProgramUniform4dv");
			glProgramUniform4ui = (gl_program_uniform4ui_func *)GetOpenGLProcAddress_Internal(state, "glProgramUniform4ui");
			glProgramUniform4uiv = (gl_program_uniform4uiv_func *)GetOpenGLProcAddress_Internal(state, "glProgramUniform4uiv");
			glProgramUniformMatrix2fv = (gl_program_uniform_matrix2fv_func *)GetOpenGLProcAddress_Internal(state, "glProgramUniformMatrix2fv");
			glProgramUniformMatrix3fv = (gl_program_uniform_matrix3fv_func *)GetOpenGLProcAddress_Internal(state, "glProgramUniformMatrix3fv");
			glProgramUniformMatrix4fv = (gl_program_uniform_matrix4fv_func *)GetOpenGLProcAddress_Internal(state, "glProgramUniformMatrix4fv");
			glProgramUniformMatrix2dv = (gl_program_uniform_matrix2dv_func *)GetOpenGLProcAddress_Internal(state, "glProgramUniformMatrix2dv");
			glProgramUniformMatrix3dv = (gl_program_uniform_matrix3dv_func *)GetOpenGLProcAddress_Internal(state, "glProgramUniformMatrix3dv");
			glProgramUniformMatrix4dv = (gl_program_uniform_matrix4dv_func *)GetOpenGLProcAddress_Internal(state, "glProgramUniformMatrix4dv");
			glProgramUniformMatrix2x3fv = (gl_program_uniform_matrix2x3fv_func *)GetOpenGLProcAddress_Internal(state, "glProgramUniformMatrix2x3fv");
			glProgramUniformMatrix3x2fv = (gl_program_uniform_matrix3x2fv_func *)GetOpenGLProcAddress_Internal(state, "glProgramUniformMatrix3x2fv");
			glProgramUniformMatrix2x4fv = (gl_program_uniform_matrix2x4fv_func *)GetOpenGLProcAddress_Internal(state, "glProgramUniformMatrix2x4fv");
			glProgramUniformMatrix4x2fv = (gl_program_uniform_matrix4x2fv_func *)GetOpenGLProcAddress_Internal(state, "glProgramUniformMatrix4x2fv");
			glProgramUniformMatrix3x4fv = (gl_program_uniform_matrix3x4fv_func *)GetOpenGLProcAddress_Internal(state, "glProgramUniformMatrix3x4fv");
			glProgramUniformMatrix4x3fv = (gl_program_uniform_matrix4x3fv_func *)GetOpenGLProcAddress_Internal(state, "glProgramUniformMatrix4x3fv");
			glProgramUniformMatrix2x3dv = (gl_program_uniform_matrix2x3dv_func *)GetOpenGLProcAddress_Internal(state, "glProgramUniformMatrix2x3dv");
			glProgramUniformMatrix3x2dv = (gl_program_uniform_matrix3x2dv_func *)GetOpenGLProcAddress_Internal(state, "glProgramUniformMatrix3x2dv");
			glProgramUniformMatrix2x4dv = (gl_program_uniform_matrix2x4dv_func *)GetOpenGLProcAddress_Internal(state, "glProgramUniformMatrix2x4dv");
			glProgramUniformMatrix4x2dv = (gl_program_uniform_matrix4x2dv_func *)GetOpenGLProcAddress_Internal(state, "glProgramUniformMatrix4x2dv");
			glProgramUniformMatrix3x4dv = (gl_program_uniform_matrix3x4dv_func *)GetOpenGLProcAddress_Internal(state, "glProgramUniformMatrix3x4dv");
			glProgramUniformMatrix4x3dv = (gl_program_uniform_matrix4x3dv_func *)GetOpenGLProcAddress_Internal(state, "glProgramUniformMatrix4x3dv");
			glValidateProgramPipeline = (gl_validate_program_pipeline_func *)GetOpenGLProcAddress_Internal(state, "glValidateProgramPipeline");
			glGetProgramPipelineInfoLog = (gl_get_program_pipeline_info_log_func *)GetOpenGLProcAddress_Internal(state, "glGetProgramPipelineInfoLog");
			glVertexAttribL1d = (gl_vertex_attrib_l1d_func *)GetOpenGLProcAddress_Internal(state, "glVertexAttribL1d");
			glVertexAttribL2d = (gl_vertex_attrib_l2d_func *)GetOpenGLProcAddress_Internal(state, "glVertexAttribL2d");
			glVertexAttribL3d = (gl_vertex_attrib_l3d_func *)GetOpenGLProcAddress_Internal(state, "glVertexAttribL3d");
			glVertexAttribL4d = (gl_vertex_attrib_l4d_func *)GetOpenGLProcAddress_Internal(state, "glVertexAttribL4d");
			glVertexAttribL1dv = (gl_vertex_attrib_l1dv_func *)GetOpenGLProcAddress_Internal(state, "glVertexAttribL1dv");
			glVertexAttribL2dv = (gl_vertex_attrib_l2dv_func *)GetOpenGLProcAddress_Internal(state, "glVertexAttribL2dv");
			glVertexAttribL3dv = (gl_vertex_attrib_l3dv_func *)GetOpenGLProcAddress_Internal(state, "glVertexAttribL3dv");
			glVertexAttribL4dv = (gl_vertex_attrib_l4dv_func *)GetOpenGLProcAddress_Internal(state, "glVertexAttribL4dv");
			glVertexAttribLPointer = (gl_vertex_attrib_l_pointer_func *)GetOpenGLProcAddress_Internal(state, "glVertexAttribLPointer");
			glGetVertexAttribLdv = (gl_get_vertex_attrib_ldv_func *)GetOpenGLProcAddress_Internal(state, "glGetVertexAttribLdv");
			glViewportArrayv = (gl_viewport_arrayv_func *)GetOpenGLProcAddress_Internal(state, "glViewportArrayv");
			glViewportIndexedf = (gl_viewport_indexedf_func *)GetOpenGLProcAddress_Internal(state, "glViewportIndexedf");
			glViewportIndexedfv = (gl_viewport_indexedfv_func *)GetOpenGLProcAddress_Internal(state, "glViewportIndexedfv");
			glScissorArrayv = (gl_scissor_arrayv_func *)GetOpenGLProcAddress_Internal(state, "glScissorArrayv");
			glScissorIndexed = (gl_scissor_indexed_func *)GetOpenGLProcAddress_Internal(state, "glScissorIndexed");
			glScissorIndexedv = (gl_scissor_indexedv_func *)GetOpenGLProcAddress_Internal(state, "glScissorIndexedv");
			glDepthRangeArrayv = (gl_depth_range_arrayv_func *)GetOpenGLProcAddress_Internal(state, "glDepthRangeArrayv");
			glDepthRangeIndexed = (gl_depth_range_indexed_func *)GetOpenGLProcAddress_Internal(state, "glDepthRangeIndexed");
			glGetFloati_v = (gl_get_floati_v_func *)GetOpenGLProcAddress_Internal(state, "glGetFloati_v");
			glGetDoublei_v = (gl_get_doublei_v_func *)GetOpenGLProcAddress_Internal(state, "glGetDoublei_v");
#	endif //GL_VERSION_4_1

#	if GL_VERSION_4_2
			glDrawArraysInstancedBaseInstance = (gl_draw_arrays_instanced_base_instance_func *)GetOpenGLProcAddress_Internal(state, "glDrawArraysInstancedBaseInstance");
			glDrawElementsInstancedBaseInstance = (gl_draw_elements_instanced_base_instance_func *)GetOpenGLProcAddress_Internal(state, "glDrawElementsInstancedBaseInstance");
			glDrawElementsInstancedBaseVertexBaseInstance = (gl_draw_elements_instanced_base_vertex_base_instance_func *)GetOpenGLProcAddress_Internal(state, "glDrawElementsInstancedBaseVertexBaseInstance");
			glGetInternalformativ = (gl_get_internalformativ_func *)GetOpenGLProcAddress_Internal(state, "glGetInternalformativ");
			glGetActiveAtomicCounterBufferiv = (gl_get_active_atomic_counter_bufferiv_func *)GetOpenGLProcAddress_Internal(state, "glGetActiveAtomicCounterBufferiv");
			glBindImageTexture = (gl_bind_image_texture_func *)GetOpenGLProcAddress_Internal(state, "glBindImageTexture");
			glMemoryBarrier = (gl_memory_barrier_func *)GetOpenGLProcAddress_Internal(state, "glMemoryBarrier");
			glTexStorage1D = (gl_tex_storage1d_func *)GetOpenGLProcAddress_Internal(state, "glTexStorage1D");
			glTexStorage2D = (gl_tex_storage2d_func *)GetOpenGLProcAddress_Internal(state, "glTexStorage2D");
			glTexStorage3D = (gl_tex_storage3d_func *)GetOpenGLProcAddress_Internal(state, "glTexStorage3D");
			glDrawTransformFeedbackInstanced = (gl_draw_transform_feedback_instanced_func *)GetOpenGLProcAddress_Internal(state, "glDrawTransformFeedbackInstanced");
			glDrawTransformFeedbackStreamInstanced = (gl_draw_transform_feedback_stream_instanced_func *)GetOpenGLProcAddress_Internal(state, "glDrawTransformFeedbackStreamInstanced");
#	endif //GL_VERSION_4_2

#	if GL_VERSION_4_3
			glClearBufferData = (gl_clear_buffer_data_func *)GetOpenGLProcAddress_Internal(state, "glClearBufferData");
			glClearBufferSubData = (gl_clear_buffer_sub_data_func *)GetOpenGLProcAddress_Internal(state, "glClearBufferSubData");
			glDispatchCompute = (gl_dispatch_compute_func *)GetOpenGLProcAddress_Internal(state, "glDispatchCompute");
			glDispatchComputeIndirect = (gl_dispatch_compute_indirect_func *)GetOpenGLProcAddress_Internal(state, "glDispatchComputeIndirect");
			glCopyImageSubData = (gl_copy_image_sub_data_func *)GetOpenGLProcAddress_Internal(state, "glCopyImageSubData");
			glFramebufferParameteri = (gl_framebuffer_parameteri_func *)GetOpenGLProcAddress_Internal(state, "glFramebufferParameteri");
			glGetFramebufferParameteriv = (gl_get_framebuffer_parameteriv_func *)GetOpenGLProcAddress_Internal(state, "glGetFramebufferParameteriv");
			glGetInternalformati64v = (gl_get_internalformati64v_func *)GetOpenGLProcAddress_Internal(state, "glGetInternalformati64v");
			glInvalidateTexSubImage = (gl_invalidate_tex_sub_image_func *)GetOpenGLProcAddress_Internal(state, "glInvalidateTexSubImage");
			glInvalidateTexImage = (gl_invalidate_tex_image_func *)GetOpenGLProcAddress_Internal(state, "glInvalidateTexImage");
			glInvalidateBufferSubData = (gl_invalidate_buffer_sub_data_func *)GetOpenGLProcAddress_Internal(state, "glInvalidateBufferSubData");
			glInvalidateBufferData = (gl_invalidate_buffer_data_func *)GetOpenGLProcAddress_Internal(state, "glInvalidateBufferData");
			glInvalidateFramebuffer = (gl_invalidate_framebuffer_func *)GetOpenGLProcAddress_Internal(state, "glInvalidateFramebuffer");
			glInvalidateSubFramebuffer = (gl_invalidate_sub_framebuffer_func *)GetOpenGLProcAddress_Internal(state, "glInvalidateSubFramebuffer");
			glMultiDrawArraysIndirect = (gl_multi_draw_arrays_indirect_func *)GetOpenGLProcAddress_Internal(state, "glMultiDrawArraysIndirect");
			glMultiDrawElementsIndirect = (gl_multi_draw_elements_indirect_func *)GetOpenGLProcAddress_Internal(state, "glMultiDrawElementsIndirect");
			glGetProgramInterfaceiv = (gl_get_program_interfaceiv_func *)GetOpenGLProcAddress_Internal(state, "glGetProgramInterfaceiv");
			glGetProgramResourceIndex = (gl_get_program_resource_index_func *)GetOpenGLProcAddress_Internal(state, "glGetProgramResourceIndex");
			glGetProgramResourceName = (gl_get_program_resource_name_func *)GetOpenGLProcAddress_Internal(state, "glGetProgramResourceName");
			glGetProgramResourceiv = (gl_get_program_resourceiv_func *)GetOpenGLProcAddress_Internal(state, "glGetProgramResourceiv");
			glGetProgramResourceLocation = (gl_get_program_resource_location_func *)GetOpenGLProcAddress_Internal(state, "glGetProgramResourceLocation");
			glGetProgramResourceLocationIndex = (gl_get_program_resource_location_index_func *)GetOpenGLProcAddress_Internal(state, "glGetProgramResourceLocationIndex");
			glShaderStorageBlockBinding = (gl_shader_storage_block_binding_func *)GetOpenGLProcAddress_Internal(state, "glShaderStorageBlockBinding");
			glTexBufferRange = (gl_tex_buffer_range_func *)GetOpenGLProcAddress_Internal(state, "glTexBufferRange");
			glTexStorage2DMultisample = (gl_tex_storage2_d_multisample_func *)GetOpenGLProcAddress_Internal(state, "glTexStorage2DMultisample");
			glTexStorage3DMultisample = (gl_tex_storage3_d_multisample_func *)GetOpenGLProcAddress_Internal(state, "glTexStorage3DMultisample");
			glTextureView = (gl_texture_view_func *)GetOpenGLProcAddress_Internal(state, "glTextureView");
			glBindVertexBuffer = (gl_bind_vertex_buffer_func *)GetOpenGLProcAddress_Internal(state, "glBindVertexBuffer");
			glVertexAttribFormat = (gl_vertex_attrib_format_func *)GetOpenGLProcAddress_Internal(state, "glVertexAttribFormat");
			glVertexAttribIFormat = (gl_vertex_attrib_i_format_func *)GetOpenGLProcAddress_Internal(state, "glVertexAttribIFormat");
			glVertexAttribLFormat = (gl_vertex_attrib_l_format_func *)GetOpenGLProcAddress_Internal(state, "glVertexAttribLFormat");
			glVertexAttribBinding = (gl_vertex_attrib_binding_func *)GetOpenGLProcAddress_Internal(state, "glVertexAttribBinding");
			glVertexBindingDivisor = (gl_vertex_binding_divisor_func *)GetOpenGLProcAddress_Internal(state, "glVertexBindingDivisor");
			glDebugMessageControl = (gl_debug_message_control_func *)GetOpenGLProcAddress_Internal(state, "glDebugMessageControl");
			glDebugMessageInsert = (gl_debug_message_insert_func *)GetOpenGLProcAddress_Internal(state, "glDebugMessageInsert");
			glDebugMessageCallback = (gl_debug_message_callback_func *)GetOpenGLProcAddress_Internal(state, "glDebugMessageCallback");
			glGetDebugMessageLog = (gl_get_debug_message_log_func *)GetOpenGLProcAddress_Internal(state, "glGetDebugMessageLog");
			glPushDebugGroup = (gl_push_debug_group_func *)GetOpenGLProcAddress_Internal(state, "glPushDebugGroup");
			glPopDebugGroup = (gl_pop_debug_group_func *)GetOpenGLProcAddress_Internal(state, "glPopDebugGroup");
			glObjectLabel = (gl_object_label_func *)GetOpenGLProcAddress_Internal(state, "glObjectLabel");
			glGetObjectLabel = (gl_get_object_label_func *)GetOpenGLProcAddress_Internal(state, "glGetObjectLabel");
			glObjectPtrLabel = (gl_object_ptr_label_func *)GetOpenGLProcAddress_Internal(state, "glObjectPtrLabel");
			glGetObjectPtrLabel = (gl_get_object_ptr_label_func *)GetOpenGLProcAddress_Internal(state, "glGetObjectPtrLabel");
#	endif //GL_VERSION_4_3

#	if GL_VERSION_4_4
			glBufferStorage = (gl_buffer_storage_func *)GetOpenGLProcAddress_Internal(state, "glBufferStorage");
			glClearTexImage = (gl_clear_tex_image_func *)GetOpenGLProcAddress_Internal(state, "glClearTexImage");
			glClearTexSubImage = (gl_clear_tex_sub_image_func *)GetOpenGLProcAddress_Internal(state, "glClearTexSubImage");
			glBindBuffersBase = (gl_bind_buffers_base_func *)GetOpenGLProcAddress_Internal(state, "glBindBuffersBase");
			glBindBuffersRange = (gl_bind_buffers_range_func *)GetOpenGLProcAddress_Internal(state, "glBindBuffersRange");
			glBindTextures = (gl_bind_textures_func *)GetOpenGLProcAddress_Internal(state, "glBindTextures");
			glBindSamplers = (gl_bind_samplers_func *)GetOpenGLProcAddress_Internal(state, "glBindSamplers");
			glBindImageTextures = (gl_bind_image_textures_func *)GetOpenGLProcAddress_Internal(state, "glBindImageTextures");
			glBindVertexBuffers = (gl_bind_vertex_buffers_func *)GetOpenGLProcAddress_Internal(state, "glBindVertexBuffers");
#	endif //GL_VERSION_4_4

#	if GL_VERSION_4_5
			glClipControl = (gl_clip_control_func *)GetOpenGLProcAddress_Internal(state, "glClipControl");
			glCreateTransformFeedbacks = (gl_create_transform_feedbacks_func *)GetOpenGLProcAddress_Internal(state, "glCreateTransformFeedbacks");
			glTransformFeedbackBufferBase = (gl_transform_feedback_buffer_base_func *)GetOpenGLProcAddress_Internal(state, "glTransformFeedbackBufferBase");
			glTransformFeedbackBufferRange = (gl_transform_feedback_buffer_range_func *)GetOpenGLProcAddress_Internal(state, "glTransformFeedbackBufferRange");
			glGetTransformFeedbackiv = (gl_get_transform_feedbackiv_func *)GetOpenGLProcAddress_Internal(state, "glGetTransformFeedbackiv");
			glGetTransformFeedbacki_v = (gl_get_transform_feedbacki_v_func *)GetOpenGLProcAddress_Internal(state, "glGetTransformFeedbacki_v");
			glGetTransformFeedbacki64_v = (gl_get_transform_feedbacki64_v_func *)GetOpenGLProcAddress_Internal(state, "glGetTransformFeedbacki64_v");
			glCreateBuffers = (gl_create_buffers_func *)GetOpenGLProcAddress_Internal(state, "glCreateBuffers");
			glNamedBufferStorage = (gl_named_buffer_storage_func *)GetOpenGLProcAddress_Internal(state, "glNamedBufferStorage");
			glNamedBufferData = (gl_named_buffer_data_func *)GetOpenGLProcAddress_Internal(state, "glNamedBufferData");
			glNamedBufferSubData = (gl_named_buffer_sub_data_func *)GetOpenGLProcAddress_Internal(state, "glNamedBufferSubData");
			glCopyNamedBufferSubData = (gl_copy_named_buffer_sub_data_func *)GetOpenGLProcAddress_Internal(state, "glCopyNamedBufferSubData");
			glClearNamedBufferData = (gl_clear_named_buffer_data_func *)GetOpenGLProcAddress_Internal(state, "glClearNamedBufferData");
			glClearNamedBufferSubData = (gl_clear_named_buffer_sub_data_func *)GetOpenGLProcAddress_Internal(state, "glClearNamedBufferSubData");
			glMapNamedBuffer = (gl_map_named_buffer_func *)GetOpenGLProcAddress_Internal(state, "glMapNamedBuffer");
			glMapNamedBufferRange = (gl_map_named_buffer_range_func *)GetOpenGLProcAddress_Internal(state, "glMapNamedBufferRange");
			glUnmapNamedBuffer = (gl_unmap_named_buffer_func *)GetOpenGLProcAddress_Internal(state, "glUnmapNamedBuffer");
			glFlushMappedNamedBufferRange = (gl_flush_mapped_named_buffer_range_func *)GetOpenGLProcAddress_Internal(state, "glFlushMappedNamedBufferRange");
			glGetNamedBufferParameteriv = (gl_get_named_buffer_parameteriv_func *)GetOpenGLProcAddress_Internal(state, "glGetNamedBufferParameteriv");
			glGetNamedBufferParameteri64v = (gl_get_named_buffer_parameteri64v_func *)GetOpenGLProcAddress_Internal(state, "glGetNamedBufferParameteri64v");
			glGetNamedBufferPointerv = (gl_get_named_buffer_pointerv_func *)GetOpenGLProcAddress_Internal(state, "glGetNamedBufferPointerv");
			glGetNamedBufferSubData = (gl_get_named_buffer_sub_data_func *)GetOpenGLProcAddress_Internal(state, "glGetNamedBufferSubData");
			glCreateFramebuffers = (gl_create_framebuffers_func *)GetOpenGLProcAddress_Internal(state, "glCreateFramebuffers");
			glNamedFramebufferRenderbuffer = (gl_named_framebuffer_renderbuffer_func *)GetOpenGLProcAddress_Internal(state, "glNamedFramebufferRenderbuffer");
			glNamedFramebufferParameteri = (gl_named_framebuffer_parameteri_func *)GetOpenGLProcAddress_Internal(state, "glNamedFramebufferParameteri");
			glNamedFramebufferTexture = (gl_named_framebuffer_texture_func *)GetOpenGLProcAddress_Internal(state, "glNamedFramebufferTexture");
			glNamedFramebufferTextureLayer = (gl_named_framebuffer_texture_layer_func *)GetOpenGLProcAddress_Internal(state, "glNamedFramebufferTextureLayer");
			glNamedFramebufferDrawBuffer = (gl_named_framebuffer_draw_buffer_func *)GetOpenGLProcAddress_Internal(state, "glNamedFramebufferDrawBuffer");
			glNamedFramebufferDrawBuffers = (gl_named_framebuffer_draw_buffers_func *)GetOpenGLProcAddress_Internal(state, "glNamedFramebufferDrawBuffers");
			glNamedFramebufferReadBuffer = (gl_named_framebuffer_read_buffer_func *)GetOpenGLProcAddress_Internal(state, "glNamedFramebufferReadBuffer");
			glInvalidateNamedFramebufferData = (gl_invalidate_named_framebuffer_data_func *)GetOpenGLProcAddress_Internal(state, "glInvalidateNamedFramebufferData");
			glInvalidateNamedFramebufferSubData = (gl_invalidate_named_framebuffer_sub_data_func *)GetOpenGLProcAddress_Internal(state, "glInvalidateNamedFramebufferSubData");
			glClearNamedFramebufferiv = (gl_clear_named_framebufferiv_func *)GetOpenGLProcAddress_Internal(state, "glClearNamedFramebufferiv");
			glClearNamedFramebufferuiv = (gl_clear_named_framebufferuiv_func *)GetOpenGLProcAddress_Internal(state, "glClearNamedFramebufferuiv");
			glClearNamedFramebufferfv = (gl_clear_named_framebufferfv_func *)GetOpenGLProcAddress_Internal(state, "glClearNamedFramebufferfv");
			glClearNamedFramebufferfi = (gl_clear_named_framebufferfi_func *)GetOpenGLProcAddress_Internal(state, "glClearNamedFramebufferfi");
			glBlitNamedFramebuffer = (gl_blit_named_framebuffer_func *)GetOpenGLProcAddress_Internal(state, "glBlitNamedFramebuffer");
			glCheckNamedFramebufferStatus = (gl_check_named_framebuffer_status_func *)GetOpenGLProcAddress_Internal(state, "glCheckNamedFramebufferStatus");
			glGetNamedFramebufferParameteriv = (gl_get_named_framebuffer_parameteriv_func *)GetOpenGLProcAddress_Internal(state, "glGetNamedFramebufferParameteriv");
			glGetNamedFramebufferAttachmentParameteriv = (gl_get_named_framebuffer_attachment_parameteriv_func *)GetOpenGLProcAddress_Internal(state, "glGetNamedFramebufferAttachmentParameteriv");
			glCreateRenderbuffers = (gl_create_renderbuffers_func *)GetOpenGLProcAddress_Internal(state, "glCreateRenderbuffers");
			glNamedRenderbufferStorage = (gl_named_renderbuffer_storage_func *)GetOpenGLProcAddress_Internal(state, "glNamedRenderbufferStorage");
			glNamedRenderbufferStorageMultisample = (gl_named_renderbuffer_storage_multisample_func *)GetOpenGLProcAddress_Internal(state, "glNamedRenderbufferStorageMultisample");
			glGetNamedRenderbufferParameteriv = (gl_get_named_renderbuffer_parameteriv_func *)GetOpenGLProcAddress_Internal(state, "glGetNamedRenderbufferParameteriv");
			glCreateTextures = (gl_create_textures_func *)GetOpenGLProcAddress_Internal(state, "glCreateTextures");
			glTextureBuffer = (gl_texture_buffer_func *)GetOpenGLProcAddress_Internal(state, "glTextureBuffer");
			glTextureBufferRange = (gl_texture_buffer_range_func *)GetOpenGLProcAddress_Internal(state, "glTextureBufferRange");
			glTextureStorage1D = (gl_texture_storage1d_func *)GetOpenGLProcAddress_Internal(state, "glTextureStorage1D");
			glTextureStorage2D = (gl_texture_storage2d_func *)GetOpenGLProcAddress_Internal(state, "glTextureStorage2D");
			glTextureStorage3D = (gl_texture_storage3d_func *)GetOpenGLProcAddress_Internal(state, "glTextureStorage3D");
			glTextureStorage2DMultisample = (gl_texture_storage2_d_multisample_func *)GetOpenGLProcAddress_Internal(state, "glTextureStorage2DMultisample");
			glTextureStorage3DMultisample = (gl_texture_storage3_d_multisample_func *)GetOpenGLProcAddress_Internal(state, "glTextureStorage3DMultisample");
			glTextureSubImage1D = (gl_texture_sub_image1d_func *)GetOpenGLProcAddress_Internal(state, "glTextureSubImage1D");
			glTextureSubImage2D = (gl_texture_sub_image2d_func *)GetOpenGLProcAddress_Internal(state, "glTextureSubImage2D");
			glTextureSubImage3D = (gl_texture_sub_image3d_func *)GetOpenGLProcAddress_Internal(state, "glTextureSubImage3D");
			glCompressedTextureSubImage1D = (gl_compressed_texture_sub_image1d_func *)GetOpenGLProcAddress_Internal(state, "glCompressedTextureSubImage1D");
			glCompressedTextureSubImage2D = (gl_compressed_texture_sub_image2d_func *)GetOpenGLProcAddress_Internal(state, "glCompressedTextureSubImage2D");
			glCompressedTextureSubImage3D = (gl_compressed_texture_sub_image3d_func *)GetOpenGLProcAddress_Internal(state, "glCompressedTextureSubImage3D");
			glCopyTextureSubImage1D = (gl_copy_texture_sub_image1d_func *)GetOpenGLProcAddress_Internal(state, "glCopyTextureSubImage1D");
			glCopyTextureSubImage2D = (gl_copy_texture_sub_image2d_func *)GetOpenGLProcAddress_Internal(state, "glCopyTextureSubImage2D");
			glCopyTextureSubImage3D = (gl_copy_texture_sub_image3d_func *)GetOpenGLProcAddress_Internal(state, "glCopyTextureSubImage3D");
			glTextureParameterf = (gl_texture_parameterf_func *)GetOpenGLProcAddress_Internal(state, "glTextureParameterf");
			glTextureParameterfv = (gl_texture_parameterfv_func *)GetOpenGLProcAddress_Internal(state, "glTextureParameterfv");
			glTextureParameteri = (gl_texture_parameteri_func *)GetOpenGLProcAddress_Internal(state, "glTextureParameteri");
			glTextureParameterIiv = (gl_texture_parameter_iiv_func *)GetOpenGLProcAddress_Internal(state, "glTextureParameterIiv");
			glTextureParameterIuiv = (gl_texture_parameter_iuiv_func *)GetOpenGLProcAddress_Internal(state, "glTextureParameterIuiv");
			glTextureParameteriv = (gl_texture_parameteriv_func *)GetOpenGLProcAddress_Internal(state, "glTextureParameteriv");
			glGenerateTextureMipmap = (gl_generate_texture_mipmap_func *)GetOpenGLProcAddress_Internal(state, "glGenerateTextureMipmap");
			glBindTextureUnit = (gl_bind_texture_unit_func *)GetOpenGLProcAddress_Internal(state, "glBindTextureUnit");
			glGetTextureImage = (gl_get_texture_image_func *)GetOpenGLProcAddress_Internal(state, "glGetTextureImage");
			glGetCompressedTextureImage = (gl_get_compressed_texture_image_func *)GetOpenGLProcAddress_Internal(state, "glGetCompressedTextureImage");
			glGetTextureLevelParameterfv = (gl_get_texture_level_parameterfv_func *)GetOpenGLProcAddress_Internal(state, "glGetTextureLevelParameterfv");
			glGetTextureLevelParameteriv = (gl_get_texture_level_parameteriv_func *)GetOpenGLProcAddress_Internal(state, "glGetTextureLevelParameteriv");
			glGetTextureParameterfv = (gl_get_texture_parameterfv_func *)GetOpenGLProcAddress_Internal(state, "glGetTextureParameterfv");
			glGetTextureParameterIiv = (gl_get_texture_parameter_iiv_func *)GetOpenGLProcAddress_Internal(state, "glGetTextureParameterIiv");
			glGetTextureParameterIuiv = (gl_get_texture_parameter_iuiv_func *)GetOpenGLProcAddress_Internal(state, "glGetTextureParameterIuiv");
			glGetTextureParameteriv = (gl_get_texture_parameteriv_func *)GetOpenGLProcAddress_Internal(state, "glGetTextureParameteriv");
			glCreateVertexArrays = (gl_create_vertex_arrays_func *)GetOpenGLProcAddress_Internal(state, "glCreateVertexArrays");
			glDisableVertexArrayAttrib = (gl_disable_vertex_array_attrib_func *)GetOpenGLProcAddress_Internal(state, "glDisableVertexArrayAttrib");
			glEnableVertexArrayAttrib = (gl_enable_vertex_array_attrib_func *)GetOpenGLProcAddress_Internal(state, "glEnableVertexArrayAttrib");
			glVertexArrayElementBuffer = (gl_vertex_array_element_buffer_func *)GetOpenGLProcAddress_Internal(state, "glVertexArrayElementBuffer");
			glVertexArrayVertexBuffer = (gl_vertex_array_vertex_buffer_func *)GetOpenGLProcAddress_Internal(state, "glVertexArrayVertexBuffer");
			glVertexArrayVertexBuffers = (gl_vertex_array_vertex_buffers_func *)GetOpenGLProcAddress_Internal(state, "glVertexArrayVertexBuffers");
			glVertexArrayAttribBinding = (gl_vertex_array_attrib_binding_func *)GetOpenGLProcAddress_Internal(state, "glVertexArrayAttribBinding");
			glVertexArrayAttribFormat = (gl_vertex_array_attrib_format_func *)GetOpenGLProcAddress_Internal(state, "glVertexArrayAttribFormat");
			glVertexArrayAttribIFormat = (gl_vertex_array_attrib_i_format_func *)GetOpenGLProcAddress_Internal(state, "glVertexArrayAttribIFormat");
			glVertexArrayAttribLFormat = (gl_vertex_array_attrib_l_format_func *)GetOpenGLProcAddress_Internal(state, "glVertexArrayAttribLFormat");
			glVertexArrayBindingDivisor = (gl_vertex_array_binding_divisor_func *)GetOpenGLProcAddress_Internal(state, "glVertexArrayBindingDivisor");
			glGetVertexArrayiv = (gl_get_vertex_arrayiv_func *)GetOpenGLProcAddress_Internal(state, "glGetVertexArrayiv");
			glGetVertexArrayIndexediv = (gl_get_vertex_array_indexediv_func *)GetOpenGLProcAddress_Internal(state, "glGetVertexArrayIndexediv");
			glGetVertexArrayIndexed64iv = (gl_get_vertex_array_indexed64iv_func *)GetOpenGLProcAddress_Internal(state, "glGetVertexArrayIndexed64iv");
			glCreateSamplers = (gl_create_samplers_func *)GetOpenGLProcAddress_Internal(state, "glCreateSamplers");
			glCreateProgramPipelines = (gl_create_program_pipelines_func *)GetOpenGLProcAddress_Internal(state, "glCreateProgramPipelines");
			glCreateQueries = (gl_create_queries_func *)GetOpenGLProcAddress_Internal(state, "glCreateQueries");
			glGetQueryBufferObjecti64v = (gl_get_query_buffer_objecti64v_func *)GetOpenGLProcAddress_Internal(state, "glGetQueryBufferObjecti64v");
			glGetQueryBufferObjectiv = (gl_get_query_buffer_objectiv_func *)GetOpenGLProcAddress_Internal(state, "glGetQueryBufferObjectiv");
			glGetQueryBufferObjectui64v = (gl_get_query_buffer_objectui64v_func *)GetOpenGLProcAddress_Internal(state, "glGetQueryBufferObjectui64v");
			glGetQueryBufferObjectuiv = (gl_get_query_buffer_objectuiv_func *)GetOpenGLProcAddress_Internal(state, "glGetQueryBufferObjectuiv");
			glMemoryBarrierByRegion = (gl_memory_barrier_by_region_func *)GetOpenGLProcAddress_Internal(state, "glMemoryBarrierByRegion");
			glGetTextureSubImage = (gl_get_texture_sub_image_func *)GetOpenGLProcAddress_Internal(state, "glGetTextureSubImage");
			glGetCompressedTextureSubImage = (gl_get_compressed_texture_sub_image_func *)GetOpenGLProcAddress_Internal(state, "glGetCompressedTextureSubImage");
			glGetGraphicsResetStatus = (gl_get_graphics_reset_status_func *)GetOpenGLProcAddress_Internal(state, "glGetGraphicsResetStatus");
			glGetnCompressedTexImage = (gl_getn_compressed_tex_image_func *)GetOpenGLProcAddress_Internal(state, "glGetnCompressedTexImage");
			glGetnTexImage = (gl_getn_tex_image_func *)GetOpenGLProcAddress_Internal(state, "glGetnTexImage");
			glGetnUniformdv = (gl_getn_uniformdv_func *)GetOpenGLProcAddress_Internal(state, "glGetnUniformdv");
			glGetnUniformfv = (gl_getn_uniformfv_func *)GetOpenGLProcAddress_Internal(state, "glGetnUniformfv");
			glGetnUniformiv = (gl_getn_uniformiv_func *)GetOpenGLProcAddress_Internal(state, "glGetnUniformiv");
			glGetnUniformuiv = (gl_getn_uniformuiv_func *)GetOpenGLProcAddress_Internal(state, "glGetnUniformuiv");
			glReadnPixels = (gl_readn_pixels_func *)GetOpenGLProcAddress_Internal(state, "glReadnPixels");
			glGetnMapdv = (gl_getn_mapdv_func *)GetOpenGLProcAddress_Internal(state, "glGetnMapdv");
			glGetnMapfv = (gl_getn_mapfv_func *)GetOpenGLProcAddress_Internal(state, "glGetnMapfv");
			glGetnMapiv = (gl_getn_mapiv_func *)GetOpenGLProcAddress_Internal(state, "glGetnMapiv");
			glGetnPixelMapfv = (gl_getn_pixel_mapfv_func *)GetOpenGLProcAddress_Internal(state, "glGetnPixelMapfv");
			glGetnPixelMapuiv = (gl_getn_pixel_mapuiv_func *)GetOpenGLProcAddress_Internal(state, "glGetnPixelMapuiv");
			glGetnPixelMapusv = (gl_getn_pixel_mapusv_func *)GetOpenGLProcAddress_Internal(state, "glGetnPixelMapusv");
			glGetnPolygonStipple = (gl_getn_polygon_stipple_func *)GetOpenGLProcAddress_Internal(state, "glGetnPolygonStipple");
			glGetnColorTable = (gl_getn_color_table_func *)GetOpenGLProcAddress_Internal(state, "glGetnColorTable");
			glGetnConvolutionFilter = (gl_getn_convolution_filter_func *)GetOpenGLProcAddress_Internal(state, "glGetnConvolutionFilter");
			glGetnSeparableFilter = (gl_getn_separable_filter_func *)GetOpenGLProcAddress_Internal(state, "glGetnSeparableFilter");
			glGetnHistogram = (gl_getn_histogram_func *)GetOpenGLProcAddress_Internal(state, "glGetnHistogram");
			glGetnMinmax = (gl_getn_minmax_func *)GetOpenGLProcAddress_Internal(state, "glGetnMinmax");
			glTextureBarrier = (gl_texture_barrier_func *)GetOpenGLProcAddress_Internal(state, "glTextureBarrier");
#	endif //GL_VERSION_4_5
		}

		static bool LoadOpenGL_Internal(OpenGLState &state) {
#	if defined(FDYNGL_PLATFORM_WIN32)
			// user.dll
			state.win32.user32.libraryHandle = LoadLibraryA("user32.dll");
			if (state.win32.user32.libraryHandle == nullptr) {
				SetLastError_Internal(state, "Failed loading win32 user32.dll!");
				return false;
			}
			state.win32.user32.GetDC = (win32_func_GetDC *)GetProcAddress(state.win32.user32.libraryHandle, "GetDC");
			state.win32.user32.ReleaseDC = (win32_func_ReleaseDC *)GetProcAddress(state.win32.user32.libraryHandle, "ReleaseDC");

			// gdi.dll
			state.win32.gdi32.libraryHandle = LoadLibraryA("gdi32.dll");
			if (state.win32.gdi32.libraryHandle == nullptr) {
				SetLastError_Internal(state, "Failed loading win32 gdi32.dll!");
				return false;
			}
			state.win32.gdi32.ChoosePixelFormat = (win32_func_ChoosePixelFormat *)GetProcAddress(state.win32.gdi32.libraryHandle, "ChoosePixelFormat");
			state.win32.gdi32.SetPixelFormat = (win32_func_SetPixelFormat *)GetProcAddress(state.win32.gdi32.libraryHandle, "SetPixelFormat");
			state.win32.gdi32.DescribePixelFormat = (win32_func_DescribePixelFormat *)GetProcAddress(state.win32.gdi32.libraryHandle, "DescribePixelFormat");
			state.win32.gdi32.SwapBuffers = (win32_func_SwapBuffers *)GetProcAddress(state.win32.gdi32.libraryHandle, "SwapBuffers");

			// opengl32.dll
			const char *win32LibraryNames[] = {
				"opengl32.dll",
			};
			HMODULE glLibraryHandle = nullptr;
			for (int i = 0; i < FDYNGL_ARRAYCOUNT(win32LibraryNames); ++i) {
				glLibraryHandle = LoadLibraryA(win32LibraryNames[i]);
				if (glLibraryHandle != nullptr) {
					state.win32.opengl32.wglGetProcAddress = (win32_func_wglGetProcAddress *)GetProcAddress(glLibraryHandle, "wglGetProcAddress");
					state.win32.opengl32.wglCreateContext = (win32_func_wglCreateContext *)GetProcAddress(glLibraryHandle, "wglCreateContext");
					state.win32.opengl32.wglDeleteContext = (win32_func_wglDeleteContext *)GetProcAddress(glLibraryHandle, "wglDeleteContext");
					state.win32.opengl32.wglMakeCurrent = (win32_func_wglMakeCurrent *)GetProcAddress(glLibraryHandle, "wglMakeCurrent");
					break;
				}
			}
			if (glLibraryHandle == nullptr) {
				SetLastError_Internal(state, "Failed loading win32 opengl32.dll!");
				return false;
			}
			state.win32.opengl32.libraryHandle = glLibraryHandle;
#	elif defined(FDYNGL_PLATFORM_POSIX)
			const char *posixLibraryNames[] = {
				"libGL.so",
				"libGL.so.1",
			};
			void *glLibraryHandle = nullptr;
			for (int i = 0; i < FDYNGL_ARRAYCOUNT(posixLibraryNames); ++i) {
				glLibraryHandle = dlopen(posixLibraryNames[i], RTLD_NOW);
				if (glLibraryHandle != nullptr) {
					state.posix_glx.glXGetProcAddress = (glx_func_glXGetProcAddress *)dlsym(glLibraryHandle, "glXGetProcAddress");
					break;
				}
			}
			if (glLibraryHandle == nullptr) {
				SetLastError_Internal(state, "Failed loading posix libGL.so!");
				return false;
			}
			state.posix_glx.libraryHandle = glLibraryHandle;
#	endif
			state.isLoaded = true;
			return(true);
		}

		static void UnloadOpenGL_Internal(OpenGLState &state) {
			if (state.isLoaded) {
#			if defined(FDYNGL_PLATFORM_WIN32)
				if (state.win32.opengl32.libraryHandle != nullptr) {
					FreeLibrary(state.win32.opengl32.libraryHandle);
				}
				if (state.win32.gdi32.libraryHandle != nullptr) {
					FreeLibrary(state.win32.gdi32.libraryHandle);
				}
				if (state.win32.user32.libraryHandle != nullptr) {
					FreeLibrary(state.win32.user32.libraryHandle);
				}
#			elif defined(FDYNGL_PLATFORM_POSIX) && defined(FDYNGL_PLATFORM_GLX)
				if (state.posix_glx.libraryHandle != nullptr) {
					dlclose(state.posix_glx.libraryHandle);
				}
#			endif
			}
			state = {};
		}

		static void DestroyOpenGLContext_Internal(OpenGLState &state, OpenGLContext &context) {
			if (!state.isLoaded) {
				SetLastError_Internal(state, "OpenGL library was not loaded!");
				return;
			}
#		if defined(FDYNGL_PLATFORM_WIN32)
			if (context.renderingContext.win32.renderingContext != nullptr) {
				state.win32.opengl32.wglMakeCurrent(nullptr, nullptr);
				state.win32.opengl32.wglDeleteContext(context.renderingContext.win32.renderingContext);
				context.renderingContext.win32.renderingContext = nullptr;
			}
			if (context.windowHandle.win32.requireToReleaseDC) {
				state.win32.user32.ReleaseDC(context.windowHandle.win32.windowHandle, context.windowHandle.win32.deviceContext);
				context.windowHandle.win32.deviceContext = nullptr;
				context.windowHandle.win32.requireToReleaseDC = false;
			}
#		elif defined(FDYNGL_PLATFORM_POSIX) && defined(FDYNGL_PLATFORM_GLX)
#		endif
			context = {};
		}

		static bool CreateOpenGLContext_Internal(OpenGLState &state, const OpenGLContextCreationParameters &contextCreationParams, OpenGLContext &outContext) {
			if (!state.isLoaded) {
				SetLastError_Internal(state, "OpenGL library is not loaded!");
				return false;
			}

			outContext = {};
#		if defined(FDYNGL_PLATFORM_WIN32)
			HDC deviceContext = contextCreationParams.windowHandle.win32.deviceContext;
			HWND handle = contextCreationParams.windowHandle.win32.windowHandle;
			bool requireToReleaseDC = false;
			if (deviceContext == nullptr) {
				if (handle == nullptr) {
					SetLastError_Internal(state, "Missing win32 window handle in opengl context creation!");
					return false;
				}
				deviceContext = state.win32.user32.GetDC(handle);
				requireToReleaseDC = true;
			}

			outContext.windowHandle.win32.deviceContext = deviceContext;
			outContext.windowHandle.win32.windowHandle = handle;
			outContext.windowHandle.win32.requireToReleaseDC = requireToReleaseDC;

			PIXELFORMATDESCRIPTOR pfd = {};
			pfd.nSize = sizeof(pfd);
			pfd.nVersion = 1;
			pfd.dwFlags = PFD_DOUBLEBUFFER | PFD_SUPPORT_OPENGL | PFD_DRAW_TO_WINDOW;
			pfd.iPixelType = PFD_TYPE_RGBA;
			pfd.cColorBits = 32;
			pfd.cDepthBits = 24;
			pfd.cAlphaBits = 8;
			pfd.iLayerType = PFD_MAIN_PLANE;

			int pixelFormat = state.win32.gdi32.ChoosePixelFormat(deviceContext, &pfd);
			if (!pixelFormat) {
				SetLastError_Internal(state, "Failed win32 choosing pixel format for device context '%p'!", deviceContext);
				DestroyOpenGLContext_Internal(state, outContext);
				return false;
			}

			if (!state.win32.gdi32.SetPixelFormat(deviceContext, pixelFormat, &pfd)) {
				SetLastError_Internal(state, "Failed win32 setting pixel format '%d' for device context '%p'!", pixelFormat, deviceContext);
				DestroyOpenGLContext_Internal(state, outContext);
				return false;
			}

			state.win32.gdi32.DescribePixelFormat(deviceContext, pixelFormat, sizeof(pfd), &pfd);

			HGLRC legacyRenderingContext = state.win32.opengl32.wglCreateContext(deviceContext);
			if (!legacyRenderingContext) {
				SetLastError_Internal(state, "Failed win32 creating opengl legacy rendering context for device context '%p'!", deviceContext);
				DestroyOpenGLContext_Internal(state, outContext);
				return false;
			}

			if (!state.win32.opengl32.wglMakeCurrent(deviceContext, legacyRenderingContext)) {
				SetLastError_Internal(state, "Failed win32 activating opengl legacy rendering context '%p' for device context '%p'!", legacyRenderingContext, deviceContext);
				DestroyOpenGLContext_Internal(state, outContext);
				return false;
			}

			outContext.renderingContext.win32.renderingContext = legacyRenderingContext;
			outContext.isValid = true;
#		elif defined(FDYNGL_PLATFORM_POSIX) && defined(FDYNGL_PLATFORM_GLX)
			// @TODO(final): Implement GLX context creation
#		endif
			return (outContext.isValid);
		}
	} // platform
} // fdyngl

namespace fdyngl {
	static OpenGLState globalOpenGLState = {};

	fdyngl_api bool CreateOpenGLContext(const OpenGLContextCreationParameters &contextCreationParams, OpenGLContext &outContext) {
		OpenGLState &state = globalOpenGLState;
		if (!state.isLoaded) {
			if (!platform::LoadOpenGL_Internal(state)) {
				assert(strlen(state.lastError) > 0);
				return false;
			}
		}
		bool result = platform::CreateOpenGLContext_Internal(globalOpenGLState, contextCreationParams, outContext);
		if (!result) {
			assert(strlen(state.lastError) > 0);
		}
		return(result);
	}

	fdyngl_api void DestroyOpenGLContext(OpenGLContext &context) {
		platform::DestroyOpenGLContext_Internal(globalOpenGLState, context);
	}

	fdyngl_api void LoadOpenGLFunctions() {
		OpenGLState &state = globalOpenGLState;
		if (state.isLoaded) {
			platform::LoadOpenGLExtensions_Internal(state);
		}
	}

	fdyngl_api bool LoadOpenGL(const bool loadFunctions) {
		OpenGLState &state = globalOpenGLState;
		if (!state.isLoaded) {
			if (!platform::LoadOpenGL_Internal(state)) {
				assert(strlen(state.lastError) > 0);
				return false;
			}
		}
		if (loadFunctions) {
			LoadOpenGLFunctions();
		}
		return true;
	}

	fdyngl_api void UnloadOpenGL() {
		OpenGLState &state = globalOpenGLState;
		platform::UnloadOpenGL_Internal(state);
		assert(!state.isLoaded);
	}

	fdyngl_api void PresentOpenGL(const OpenGLContext &context) {
		const OpenGLState &state = globalOpenGLState;
#	if defined(FDYNGL_PLATFORM_WIN32)
		if (context.windowHandle.win32.deviceContext != nullptr) {
			state.win32.gdi32.SwapBuffers(context.windowHandle.win32.deviceContext);
		}
#	endif
	}

	fdyngl_api const char *GetLastError() {
		const OpenGLState &state = globalOpenGLState;
		return state.lastError;
	}
} // fdyngl

#endif // FDYNGL_IMPLEMENTATION