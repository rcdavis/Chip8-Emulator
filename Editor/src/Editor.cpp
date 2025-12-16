#include "Editor.h"

#include "glad/gl.h"
#include "GLFW/glfw3.h"

#include "Utils/Log.h"

static void GlfwErrorCallback(int error, const char* description);
static void APIENTRY GLDebugCallback(GLenum source, GLenum type, GLuint id, GLenum severity,
	GLsizei length, const GLchar* message, const void* userParam);

Editor::Editor() :
	mChip8(),
	mWindow(nullptr)
{}

Editor::~Editor() {
	glfwTerminate();
	mWindow = nullptr;
}

bool Editor::Init(int argc, char** argv) {
	glfwSetErrorCallback(GlfwErrorCallback);

	if (!glfwInit()) {
		LOG_CRITICAL("Failed to init GLFW");
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
		LOG_CRITICAL("Failed to create GLFW window");
		return false;
	}

	glfwMakeContextCurrent(mWindow);

	if (!gladLoadGL((GLADloadfunc)glfwGetProcAddress)) {
		LOG_CRITICAL("Failed to init GLAD");
		return false;
	}

#ifdef DEBUG
	GLint glMajorVer = 0, glMinorVer = 0;
	glGetIntegerv(GL_MAJOR_VERSION, &glMajorVer);
	glGetIntegerv(GL_MINOR_VERSION, &glMinorVer);

	LOG_INFO("OpenGL Info:");
	LOG_INFO("  Vendor: {0}", (char*)glGetString(GL_VENDOR));
	LOG_INFO("  Renderer: {0}", (char*)glGetString(GL_RENDERER));
	LOG_INFO("  Version: {0}.{1}", glMajorVer, glMinorVer);

	if (GLAD_GL_KHR_debug) {
		LOG_INFO("Setting up OpenGL debug callback...");
		glEnable(GL_DEBUG_OUTPUT);
		glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
		glDebugMessageCallback(GLDebugCallback, nullptr);
	} else {
		LOG_INFO("Unable to use OpenGL debug callback");
	}
#endif

	glfwSwapInterval(1);

	glClearColor(1.0f, 0.0f, 1.0f, 1.0f);

	if (argc > 1) {
		mChip8.LoadGame(argv[1]);
	}

	return true;
}

void Editor::Run() {
	while (!glfwWindowShouldClose(mWindow)) {
		glfwPollEvents();

		UpdateInput();

		glClear(GL_COLOR_BUFFER_BIT);

		mChip8.EmulateCycle();

		if (mChip8.mShouldRedraw) {
			// TODO: Render VRAM
		}

		glfwSwapBuffers(mWindow);
	}
}

void Editor::UpdateInput() {
	memset(std::data(mChip8.mKeys), 0, std::size(mChip8.mKeys));

	if (glfwGetKey(mWindow, GLFW_KEY_1) == GLFW_PRESS)
		mChip8.mKeys[0] = 1;
	if (glfwGetKey(mWindow, GLFW_KEY_2) == GLFW_PRESS)
		mChip8.mKeys[1] = 1;
	if (glfwGetKey(mWindow, GLFW_KEY_3) == GLFW_PRESS)
		mChip8.mKeys[2] = 1;
	if (glfwGetKey(mWindow, GLFW_KEY_4) == GLFW_PRESS)
		mChip8.mKeys[3] = 1;

	if (glfwGetKey(mWindow, GLFW_KEY_Q) == GLFW_PRESS)
		mChip8.mKeys[4] = 1;
	if (glfwGetKey(mWindow, GLFW_KEY_W) == GLFW_PRESS)
		mChip8.mKeys[5] = 1;
	if (glfwGetKey(mWindow, GLFW_KEY_E) == GLFW_PRESS)
		mChip8.mKeys[6] = 1;
	if (glfwGetKey(mWindow, GLFW_KEY_R) == GLFW_PRESS)
		mChip8.mKeys[7] = 1;

	if (glfwGetKey(mWindow, GLFW_KEY_A) == GLFW_PRESS)
		mChip8.mKeys[8] = 1;
	if (glfwGetKey(mWindow, GLFW_KEY_S) == GLFW_PRESS)
		mChip8.mKeys[9] = 1;
	if (glfwGetKey(mWindow, GLFW_KEY_D) == GLFW_PRESS)
		mChip8.mKeys[10] = 1;
	if (glfwGetKey(mWindow, GLFW_KEY_F) == GLFW_PRESS)
		mChip8.mKeys[11] = 1;

	if (glfwGetKey(mWindow, GLFW_KEY_Z) == GLFW_PRESS)
		mChip8.mKeys[12] = 1;
	if (glfwGetKey(mWindow, GLFW_KEY_X) == GLFW_PRESS)
		mChip8.mKeys[13] = 1;
	if (glfwGetKey(mWindow, GLFW_KEY_C) == GLFW_PRESS)
		mChip8.mKeys[14] = 1;
	if (glfwGetKey(mWindow, GLFW_KEY_V) == GLFW_PRESS)
		mChip8.mKeys[15] = 1;
}

void GlfwErrorCallback(int error, const char* description) {
	LOG_ERROR("GLFW Error ({0}): {1}", error, description);
}

void APIENTRY GLDebugCallback(GLenum source, GLenum type, GLuint id, GLenum severity,
	GLsizei length, const GLchar* message, const void* userParam)
{
	switch (severity)
	{
	case GL_DEBUG_SEVERITY_HIGH:
		LOG_CRITICAL("OpenGL Debug Message: {0}", message);
		break;

	case GL_DEBUG_SEVERITY_MEDIUM:
		LOG_ERROR("OpenGL Debug Message: {0}", message);
		break;

	case GL_DEBUG_SEVERITY_LOW:
		LOG_WARN("OpenGL Debug Message: {0}", message);
		break;

	case GL_DEBUG_SEVERITY_NOTIFICATION:
		LOG_INFO("OpenGL Debug Message: {0}", message);
		break;

	default:
		LOG_TRACE("OpenGL Debug Message: {0}", message);
		break;
	}
}
