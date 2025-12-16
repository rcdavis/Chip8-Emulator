#include "Editor.h"

#include "glad/gl.h"
#include "GLFW/glfw3.h"

#include "OpenGL/GLShader.h"

#include "Utils/Log.h"

static void GlfwErrorCallback(int error, const char* description);
static void APIENTRY GLDebugCallback(GLenum source, GLenum type, GLuint id, GLenum severity,
	GLsizei length, const GLchar* message, const void* userParam);

Editor::Editor() :
	mChip8(),
	mWindow(nullptr)
{}

Editor::~Editor() {
	if (mTexture)
		glDeleteTextures(1, &mTexture);

	if (mIb)
		glDeleteBuffers(1, &mIb);

	if (mVb)
		glDeleteBuffers(1, &mVb);

	if (mVao)
		glDeleteVertexArrays(1, &mVao);

	mShader.Delete();

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

	mScreenData.resize(mChip8.GetVramSize());

	SetupRenderQuad();

	if (argc > 1) {
		mChip8.LoadGame(argv[1]);
	}

	return true;
}

void Editor::Run() {
	while (!glfwWindowShouldClose(mWindow)) {
		glfwPollEvents();

		UpdateInput();

		mChip8.EmulateCycle();

		if (mChip8.mShouldRedraw) {
			glClear(GL_COLOR_BUFFER_BIT);

			for (size_t i = 0; i < mChip8.GetVram().size(); ++i) {
				mScreenData[i] = (mChip8.GetVram()[i] == 1) ? 0xFFFFFFFF : 0xFF000000;
			}

			glBindTexture(GL_TEXTURE_2D, mTexture);
			glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, mChip8.GetWidth(), mChip8.GetHeight(), GL_RGBA, GL_UNSIGNED_BYTE, mScreenData.data());

			mShader.Bind();

			glBindVertexArray(mVao);

			glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, nullptr);

			mChip8.mShouldRedraw = false;
		}

		glfwSwapBuffers(mWindow);
	}
}

void Editor::UpdateInput() {
	mChip8.mKeys[0x1] = (glfwGetKey(mWindow, GLFW_KEY_1) == GLFW_PRESS) ? 1 : 0;
    mChip8.mKeys[0x2] = (glfwGetKey(mWindow, GLFW_KEY_2) == GLFW_PRESS) ? 1 : 0;
    mChip8.mKeys[0x3] = (glfwGetKey(mWindow, GLFW_KEY_3) == GLFW_PRESS) ? 1 : 0;
    mChip8.mKeys[0xC] = (glfwGetKey(mWindow, GLFW_KEY_4) == GLFW_PRESS) ? 1 : 0;

    mChip8.mKeys[0x4] = (glfwGetKey(mWindow, GLFW_KEY_Q) == GLFW_PRESS) ? 1 : 0;
    mChip8.mKeys[0x5] = (glfwGetKey(mWindow, GLFW_KEY_W) == GLFW_PRESS) ? 1 : 0;
    mChip8.mKeys[0x6] = (glfwGetKey(mWindow, GLFW_KEY_E) == GLFW_PRESS) ? 1 : 0;
    mChip8.mKeys[0xD] = (glfwGetKey(mWindow, GLFW_KEY_R) == GLFW_PRESS) ? 1 : 0;

    mChip8.mKeys[0x7] = (glfwGetKey(mWindow, GLFW_KEY_A) == GLFW_PRESS) ? 1 : 0;
    mChip8.mKeys[0x8] = (glfwGetKey(mWindow, GLFW_KEY_S) == GLFW_PRESS) ? 1 : 0;
    mChip8.mKeys[0x9] = (glfwGetKey(mWindow, GLFW_KEY_D) == GLFW_PRESS) ? 1 : 0;
    mChip8.mKeys[0xE] = (glfwGetKey(mWindow, GLFW_KEY_F) == GLFW_PRESS) ? 1 : 0;

    mChip8.mKeys[0xA] = (glfwGetKey(mWindow, GLFW_KEY_Z) == GLFW_PRESS) ? 1 : 0;
    mChip8.mKeys[0x0] = (glfwGetKey(mWindow, GLFW_KEY_X) == GLFW_PRESS) ? 1 : 0;
    mChip8.mKeys[0xB] = (glfwGetKey(mWindow, GLFW_KEY_C) == GLFW_PRESS) ? 1 : 0;
    mChip8.mKeys[0xF] = (glfwGetKey(mWindow, GLFW_KEY_V) == GLFW_PRESS) ? 1 : 0;
}

void Editor::SetupRenderQuad() {
	glGenVertexArrays(1, &mVao);
	glBindVertexArray(mVao);

	// Texture
    glGenTextures(1, &mTexture);
    glBindTexture(GL_TEXTURE_2D, mTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, mChip8.GetWidth(), mChip8.GetHeight(), 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	// Vertex Buffer
	// 2 for position, 2 for texture coordinates
    constexpr std::array<float, 16> vertices = {
        -1.0f, -1.0f, 0.0f, 1.0f,
        1.0f, -1.0f, 1.0f, 1.0f,
        1.0f, 1.0f, 1.0f, 0.0f,
        -1.0f, 1.0f, 0.0f, 0.0f
    };

    glGenBuffers(1, &mVb);
    glBindBuffer(GL_ARRAY_BUFFER, mVb);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * std::size(vertices),
        std::data(vertices), GL_STATIC_DRAW);

	// Index Buffer
	constexpr std::array<uint16_t, 6> indices = {
        0, 1, 2,
        2, 3, 0
    };

    glGenBuffers(1, &mIb);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mIb);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint16_t) * std::size(indices),
        std::data(indices), GL_STATIC_DRAW);

	// Shader
	constexpr char* vertexSrc = "#version 330 core\n"
        "in vec2 a_Position;\n"
        "in vec2 a_TexCoord;\n"
        "out vec2 v_TexCoord;\n"
        "void main() {\n"
        "v_TexCoord = a_TexCoord;\n"
        "gl_Position = vec4(a_Position, 0.0, 1.0);\n"
        "}";

    constexpr char* fragmentSrc = "#version 330 core\n"
        "in vec2 v_TexCoord;\n"
        "out vec4 color;\n"
        "uniform sampler2D u_Texture;\n"
        "void main() {\n"
        "color = texture(u_Texture, v_TexCoord);\n"
        "}";

    mShader.Create(vertexSrc, fragmentSrc);
    mShader.Bind();
    mShader.SetVertexAttribf("a_Position", 2, sizeof(float) * 4);
    mShader.SetVertexAttribf("a_TexCoord", 2, sizeof(float) * 4, sizeof(float) * 2);

	glBindVertexArray(0);
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
