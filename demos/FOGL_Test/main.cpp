#define USE_LEGACY_OPENGL 0
#define USE_FPL_OPENGL_CONTEXT_CREATION 0

#define FPL_IMPLEMENTATION
#define FPL_AUTO_NAMESPACE
#if !USE_FPL_OPENGL_CONTEXT_CREATION
#	define FPL_NO_VIDEO_OPENGL
#endif
#include "final_platform_layer.hpp"

#define FDYNGL_IMPLEMENTATION
#include "final_dynamic_opengl.hpp"

static GLuint CreateShaderType(GLenum type, const char *source) {
	GLuint shaderId = glCreateShader(type);

	glShaderSource(shaderId, 1, &source, nullptr);
	glCompileShader(shaderId);

	GLint compileResult;
	glGetShaderiv(shaderId, GL_COMPILE_STATUS, &compileResult);
	if (!compileResult) {
		GLint infoLen;
		glGetShaderiv(shaderId, GL_INFO_LOG_LENGTH, &infoLen);
		char *info = (char *)MemoryStackAllocate(infoLen);
		glGetShaderInfoLog(shaderId, infoLen, &infoLen, info);
		ConsoleFormatError("Failed compiling %s shader!\n", (type == GL_VERTEX_SHADER ? "vertex" : "fragment"));
		ConsoleFormatError("%s\n", info);
	}

	return(shaderId);
}

static GLuint CreateShaderProgram(const char *name, const char *vertexSource, const char *fragmentSource) {
	GLuint programId = glCreateProgram();

	GLuint vertexShader = CreateShaderType(GL_VERTEX_SHADER, vertexSource);
	GLuint fragmentShader = CreateShaderType(GL_FRAGMENT_SHADER, fragmentSource);

	glAttachShader(programId, vertexShader);
	glAttachShader(programId, fragmentShader);
	glLinkProgram(programId);
	glValidateProgram(programId);

	GLint linkResult;
	glGetProgramiv(programId, GL_LINK_STATUS, &linkResult);
	if (!linkResult) {
		GLint infoLen;
		glGetProgramiv(programId, GL_INFO_LOG_LENGTH, &infoLen);

		char *info = (char *)MemoryStackAllocate(infoLen);
		glGetProgramInfoLog(programId, infoLen, &infoLen, info);
		ConsoleFormatError("Failed linking '%s' shader!\n", name);
		ConsoleFormatError("%s\n", info);
	}

	glDeleteShader(fragmentShader);
	glDeleteShader(vertexShader);

	return(programId);
}

static void RunModern(const fdyngl::OpenGLContext *context) {
	const char *version = (const char *)glGetString(GL_VERSION);
	const char *vendor = (const char *)glGetString(GL_VENDOR);
	const char *renderer = (const char *)glGetString(GL_RENDERER);
	ConsoleFormatOut("OpenGL version: %s\n", version);
	ConsoleFormatOut("OpenGL vendor: %s\n", vendor);
	ConsoleFormatOut("OpenGL renderer: %s\n", renderer);

	GLuint vertexArrayID;
	glGenVertexArrays(1, &vertexArrayID);
	glBindVertexArray(vertexArrayID);

	const char *glslVersion = (const char *)glGetString(GL_SHADING_LANGUAGE_VERSION);
	ConsoleFormatOut("OpenGL GLSL Version %s:\n", glslVersion);

	int profileMask;
	int contextFlags;
	glGetIntegerv(GL_CONTEXT_PROFILE_MASK, &profileMask);
	glGetIntegerv(GL_CONTEXT_FLAGS, &contextFlags);
	ConsoleFormatOut("OpenGL supported profiles:\n");
	ConsoleFormatOut("\tCore: %s\n", ((profileMask & GL_CONTEXT_CORE_PROFILE_BIT) ? "yes" : "no"));
	ConsoleFormatOut("\tForward: %s\n", ((contextFlags & GL_CONTEXT_FLAG_FORWARD_COMPATIBLE_BIT) ? "yes" : "no"));

	ConsoleOut("Running modern opengl\n");

	const char vertexSource[] = {
		"#version 330 core\n"
		"\n"
		"layout(location = 0) in vec4 inPosition;\n"
		"\n"
		"void main() {\n"
		"\tgl_Position = inPosition;\n"
		"}\n"
	};

	const char fragmentSource[] = {
		"#version 330 core\n"
		"\n"
		"layout(location = 0) out vec4 outColor;\n"
		"\n"
		"void main() {\n"
		"\toutColor = vec4(1.0, 0.0, 0.0, 1.0);\n"
		"}\n"
	};

	GLuint shaderProgram = CreateShaderProgram("Test", vertexSource, fragmentSource);

	float vertices[] = {
		0.0f, 0.5f,
		-0.5f, -0.5f,
		0.5f, -0.5f
	};
	GLuint buffer;
	glGenBuffers(1, &buffer);
	glBindBuffer(GL_ARRAY_BUFFER, buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glUseProgram(shaderProgram);

	glBindBuffer(GL_ARRAY_BUFFER, buffer);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 2, nullptr);

	glClearColor(0.39f, 0.58f, 0.93f, 1.0f);
	while (WindowUpdate()) {
		WindowSize windowArea = GetWindowArea();
		glViewport(0, 0, windowArea.width, windowArea.height);

		glClear(GL_COLOR_BUFFER_BIT);

		glDrawArrays(GL_TRIANGLES, 0, 3);

#if USE_FPL_OPENGL_CONTEXT_CREATION
		VideoFlip();
#else
		fdyngl::PresentOpenGL(*context);
#endif
	}

	glDisableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glBindVertexArray(0);
	glDeleteVertexArrays(1, &vertexArrayID);

}

int main(int argc, char **args) {
	int result = 0;
	Settings settings = DefaultSettings();

	InitFlags initFlags;
#if USE_FPL_OPENGL_CONTEXT_CREATION
	initFlags = InitFlags::Video;
	settings.video.driver = VideoDriverType::OpenGL;
#if !USE_LEGACY_OPENGL
	CopyAnsiString("FPL Modern OpenGL", settings.window.windowTitle, FPL_ARRAYCOUNT(settings.window.windowTitle));
	settings.video.opengl.compabilityFlags = OpenGLCompabilityFlags::Core;
	settings.video.opengl.majorVersion = 3;
	settings.video.opengl.minorVersion = 3;
#else
	CopyAnsiString("FPL Legacy OpenGL", settings.window.windowTitle, FPL_ARRAYCOUNT(settings.window.windowTitle));
	settings.video.profile = VideoCompabilityProfile::Legacy;
#endif
#else
	initFlags = InitFlags::Window;
#endif

	if (InitPlatform(initFlags, settings)) {
#if !USE_FPL_OPENGL_CONTEXT_CREATION
		fdyngl::OpenGLContextCreationParameters contextCreationParams = {};
#	if !USE_LEGACY_OPENGL
		CopyAnsiString("DYNGL Modern OpenGL", settings.window.windowTitle, FPL_ARRAYCOUNT(settings.window.windowTitle));
		contextCreationParams.profile = fdyngl::OpenGLProfileType::CoreProfile;
		contextCreationParams.majorVersion = 3;
		contextCreationParams.minorVersion = 3;
#	else
		CopyAnsiString("DYNGL Legacy OpenGL", settings.window.windowTitle, FPL_ARRAYCOUNT(settings.window.windowTitle));
		contextCreationParams.profile = fdyngl::OpenGLProfileType::LegacyProfile;
#	endif
#	if defined(FDYNGL_PLATFORM_WIN32)
		contextCreationParams.windowHandle.win32.deviceContext = fpl::platform::global__AppState->window.win32.deviceContext;
#	endif
		fdyngl::OpenGLContext glContext = {};
		if (fdyngl::LoadOpenGL(false)) {
			if (fdyngl::CreateOpenGLContext(contextCreationParams, glContext)) {
				fdyngl::LoadOpenGLFunctions();
				RunModern(&glContext);
				fdyngl::DestroyOpenGLContext(glContext);
			}
			fdyngl::UnloadOpenGL();
		}
#else
		if (fdyngl::InitOpenGL()) {
			RunModern(nullptr);
			fdyngl::UnloadOpenGL();
		}
#endif
		ReleasePlatform();
		result = 0;
	} else {
		result = -1;
	}
	return(result);
}