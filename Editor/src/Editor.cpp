#include "Editor.h"

#include <iostream>

#include "glad/gl.h"
#include "GLFW/glfw3.h"

static void GlfwErrorCallback(int error, const char* description);
static void APIENTRY GLDebugCallback(GLenum source, GLenum type, GLuint id, GLenum severity,
	GLsizei length, const GLchar* message, const void* userParam);

Editor::Editor() :
	mWindow(nullptr)
{}

Editor::~Editor() {
	glfwTerminate();
	mWindow = nullptr;
}

bool Editor::Init() {
	glfwSetErrorCallback(GlfwErrorCallback);

	if (!glfwInit()) {
		std::cerr << "Failed to init GLFW\n";
		return false;
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef DEBUG
	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);
#endif

	mWindow = glfwCreateWindow(1600, 900, "Chip8 Emulator", nullptr, nullptr);
	if (!mWindow) {
		std::cerr << "Failed to create GLFW window\n";
		return false;
	}

	glfwMakeContextCurrent(mWindow);

	if (!gladLoadGL((GLADloadfunc)glfwGetProcAddress)) {
		std::cerr << "Failed to init GLAD\n";
		return false;
	}

#ifdef DEBUG
	GLint glMajorVer = 0, glMinorVer = 0;
	glGetIntegerv(GL_MAJOR_VERSION, &glMajorVer);
	glGetIntegerv(GL_MINOR_VERSION, &glMinorVer);
	std::cout << "OpenGL Info:\n";
	std::cout << "  Vendor: " << glGetString(GL_VENDOR) << "\n";
	std::cout << "  Renderer: " << glGetString(GL_RENDERER) << "\n";
	std::cout << "  Version: " << glMajorVer << "." << glMinorVer << "\n";

	//LOG_INFO("OpenGL Info:");
	//LOG_INFO("  Vendor: {0}", (char*)glGetString(GL_VENDOR));
	//LOG_INFO("  Renderer: {0}", (char*)glGetString(GL_RENDERER));
	//LOG_INFO("  Version: {0}.{1}", glMajorVer, glMinorVer);

	if (GLAD_GL_KHR_debug) {
		std::cout << "Setting up OpenGL debug callback...\n";
		//LOG_INFO("Setting up OpenGL debug callback...");
		glEnable(GL_DEBUG_OUTPUT);
		glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
		glDebugMessageCallback(GLDebugCallback, nullptr);
	} else {
		std::cout << "Unable to use OpenGL debug callback\n";
		//LOG_INFO("Unable to use OpenGL debug callback");
	}
#endif

	glfwSwapInterval(1);

	glClearColor(1.0f, 0.0f, 1.0f, 1.0f);

	return true;
}

void Editor::Run() {
	while (!glfwWindowShouldClose(mWindow)) {
		glfwPollEvents();

		glClear(GL_COLOR_BUFFER_BIT);

		glfwSwapBuffers(mWindow);
	}
}

void GlfwErrorCallback(int error, const char* description) {
	std::cerr << "GLFW Error (" << error << "): " << description << "\n";
	//LOG_ERROR("GLFW Error ({0}): {1}", error, description);
}

void APIENTRY GLDebugCallback(GLenum source, GLenum type, GLuint id, GLenum severity,
	GLsizei length, const GLchar* message, const void* userParam)
{
	std::cerr << "OpenGL Debug Message: " << message << "\n";
}
