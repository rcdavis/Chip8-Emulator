#include "Editor.h"

#include <iostream>

#include "GLFW/glfw3.h"

static void GlfwErrorCallback(int error, const char* description);

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

	mWindow = glfwCreateWindow(1600, 900, "Chip8 Emulator", nullptr, nullptr);
	if (!mWindow) {
		std::cerr << "Failed to create GLFW window\n";
		return false;
	}

	glfwMakeContextCurrent(mWindow);

	glfwSwapInterval(1);

	return true;
}

void Editor::Run() {
	while (!glfwWindowShouldClose(mWindow)) {
		glfwPollEvents();

		glfwSwapBuffers(mWindow);
	}
}

void GlfwErrorCallback(int error, const char* description) {
	std::cerr << "GLFW Error (" << error << "): " << description << "\n";
	//LOG_ERROR("GLFW Error ({0}): {1}", error, description);
}
