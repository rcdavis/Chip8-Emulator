#include "Application.h"

#include "Log.h"
#include "TimeStep.h"

#include <GLFW/glfw3.h>
#include <glad/glad.h>

#include <fstream>
#include <array>

Application::~Application()
{
    TestShutdown();
}

bool Application::Init()
{
    const std::filesystem::path game = ChooseGame();
    if (std::empty(game))
        return false;

    mChip8.LoadGame(game);

    glfwSetErrorCallback(ErrorCallback);

    if (!glfwInit())
        return false;

    mWindow = glfwCreateWindow(640, 480, "Chip8 Emulator", nullptr, nullptr);
    //mWindow = glfwCreateWindow(Chip8::SCREEN_WIDTH, Chip8::SCREEN_HEIGHT, "Chip8 Emulator", nullptr, nullptr);
    if (!mWindow)
        return false;

    glfwMakeContextCurrent(mWindow);
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
        return false;

    glfwSwapInterval(1);

    glClearColor(1.0f, 0.0f, 1.0f, 1.0f);

    glCreateVertexArrays(1, &mVAO);
    glBindVertexArray(mVAO);

    InitVertexBuffer();
    InitIndexBuffer();
    InitTexture();
    InitShader();

    return true;
}

void Application::Shutdown()
{
    mShader.Delete();

    if (mTexture)
    {
        glDeleteTextures(1, &mTexture);
        mTexture = 0;
    }

    if (mPBO)
    {
        glDeleteBuffers(1, &mPBO);
        mPBO = 0;
    }

    if (mVertexBuffer)
    {
        glDeleteBuffers(1, &mVertexBuffer);
        mVertexBuffer = 0;
    }

    if (mIndexBuffer)
    {
        glDeleteBuffers(1, &mIndexBuffer);
        mIndexBuffer = 0;
    }

    if (mVAO)
    {
        glDeleteVertexArrays(1, &mVAO);
        mVAO = 0;
    }

    if (mWindow)
    {
        glfwDestroyWindow(mWindow);
        mWindow = nullptr;
    }

    glfwTerminate();
}

void Application::Run()
{
    while (!glfwWindowShouldClose(mWindow))
    {
        const TimeStep ts = (float)glfwGetTime();

        mChip8.EmulateCycle();

        if (mChip8.mRedraw)
        {
            glClear(GL_COLOR_BUFFER_BIT);

            glBindVertexArray(mVAO);
            mShader.Bind();

            //const auto image = mChip8.GetVram();
            const auto image = std::data(mChip8.GetVramImage());
            glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, Chip8::SCREEN_WIDTH, Chip8::SCREEN_HEIGHT, GL_RGBA, GL_UNSIGNED_BYTE, image);

            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, nullptr);

            glfwSwapBuffers(mWindow);
        }

        glfwPollEvents();
    }
}

void Application::UpdateInput()
{
    uint8_t* const keys = mChip8.GetKeys();

    keys[0x1] = (glfwGetKey(mWindow, GLFW_KEY_1) == GLFW_PRESS) ? 1 : 0;
    keys[0x2] = (glfwGetKey(mWindow, GLFW_KEY_2) == GLFW_PRESS) ? 1 : 0;
    keys[0x3] = (glfwGetKey(mWindow, GLFW_KEY_3) == GLFW_PRESS) ? 1 : 0;
    keys[0xC] = (glfwGetKey(mWindow, GLFW_KEY_4) == GLFW_PRESS) ? 1 : 0;

    keys[0x4] = (glfwGetKey(mWindow, GLFW_KEY_Q) == GLFW_PRESS) ? 1 : 0;
    keys[0x5] = (glfwGetKey(mWindow, GLFW_KEY_W) == GLFW_PRESS) ? 1 : 0;
    keys[0x6] = (glfwGetKey(mWindow, GLFW_KEY_E) == GLFW_PRESS) ? 1 : 0;
    keys[0xD] = (glfwGetKey(mWindow, GLFW_KEY_R) == GLFW_PRESS) ? 1 : 0;

    keys[0x7] = (glfwGetKey(mWindow, GLFW_KEY_A) == GLFW_PRESS) ? 1 : 0;
    keys[0x8] = (glfwGetKey(mWindow, GLFW_KEY_S) == GLFW_PRESS) ? 1 : 0;
    keys[0x9] = (glfwGetKey(mWindow, GLFW_KEY_D) == GLFW_PRESS) ? 1 : 0;
    keys[0xE] = (glfwGetKey(mWindow, GLFW_KEY_F) == GLFW_PRESS) ? 1 : 0;

    keys[0xA] = (glfwGetKey(mWindow, GLFW_KEY_Z) == GLFW_PRESS) ? 1 : 0;
    keys[0x0] = (glfwGetKey(mWindow, GLFW_KEY_X) == GLFW_PRESS) ? 1 : 0;
    keys[0xB] = (glfwGetKey(mWindow, GLFW_KEY_C) == GLFW_PRESS) ? 1 : 0;
    keys[0xF] = (glfwGetKey(mWindow, GLFW_KEY_V) == GLFW_PRESS) ? 1 : 0;
}

std::vector<Application::GameEntry> Application::GetGameList()
{
    constexpr char* gameListFile = "Resources/Games/GameList.glist";
    std::ifstream file(gameListFile);
    if (!file)
    {
        LOG_ERROR("Failed to open game list: {}", gameListFile);
        return {};
    }

    std::vector<GameEntry> games;
    std::string line;
    while (std::getline(file, line))
    {
        const auto index = line.find_first_of('=');
        if (index == std::string::npos)
            continue;

        GameEntry entry;
        entry.name = line.substr(0, index);
        entry.filepath = line.substr(index + 1);
        games.push_back(entry);
    }

    return games;
}

std::filesystem::path Application::ChooseGame()
{
    const auto games = GetGameList();
    if (std::empty(games))
    {
        LOG_ERROR("Empty game list");
        return {};
    }

    // TODO: Make able to choose
    return games.front().filepath;
}

void Application::ErrorCallback(int error, const char* description)
{
    LOG_ERROR("GLFW ERROR {}: {}", error, description);
}

void Application::InitVertexBuffer()
{
    // 2 for position, 2 for texture coordinates
    constexpr std::array<float, 16> vertices = {
        -1.0f, -1.0f, 0.0f, 0.0f,
        1.0f, -1.0f, 1.0f, 0.0f,
        1.0f, 1.0f, 1.0f, 1.0f,
        -1.0f, 1.0f, 0.0f, 1.0f
    };

    glGenBuffers(1, &mVertexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, mVertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * std::size(vertices), std::data(vertices), GL_STATIC_DRAW);
}

void Application::InitIndexBuffer()
{
    constexpr std::array<uint16_t, 6> indices = {
        0, 1, 2,
        2, 3, 0
    };

    glGenBuffers(1, &mIndexBuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mIndexBuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint16_t) * std::size(indices), std::data(indices), GL_STATIC_DRAW);
}

void Application::InitTexture()
{
    glActiveTexture(GL_TEXTURE0);
    glGenTextures(1, &mTexture);
    glBindTexture(GL_TEXTURE_2D, mTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, Chip8::SCREEN_WIDTH, Chip8::SCREEN_HEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTextureParameteri(mTexture, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTextureParameteri(mTexture, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTextureParameteri(mTexture, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTextureParameteri(mTexture, GL_TEXTURE_WRAP_T, GL_REPEAT);
}

void Application::InitShader()
{
    constexpr char* vertexSrc = "#version 450 core\n"
        "layout(location = 0) in vec2 a_Position;\n"
        "layout(location = 1) in vec2 a_TexCoord;\n"
        "layout(location = 0) out vec2 v_TexCoord;\n"
        "void main() {\n"
        "v_TexCoord = a_TexCoord;\n"
        "gl_Position = vec4(a_Position, 0.5, 1.0);\n"
        "}";

    constexpr char* fragmentSrc = "#version 450 core\n"
        "layout(location = 0) in vec2 v_TexCoord;\n"
        "layout(location = 0) out vec4 color;\n"
        "layout(binding = 0) uniform sampler2D u_Texture;\n"
        "void main() {\n"
        "color = texture(u_Texture, v_TexCoord);\n"
        "}";

    mShader.Create(vertexSrc, fragmentSrc);
    mShader.SetVertexAttribf("a_Position", 2, sizeof(float) * 2);
    mShader.SetVertexAttribf("a_TexCoord", 2, sizeof(float) * 2, sizeof(float) * 2);
}

void Application::InitTestVertexBuffer()
{
    constexpr std::array<float, 8> vertices = {
        -0.5f, -0.5f,
        0.5f, -0.5f,
        0.5f, 0.5f,
        -0.5f, 0.5f
    };

    glGenBuffers(1, &mTestVB);
    glBindBuffer(GL_ARRAY_BUFFER, mTestVB);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * std::size(vertices), std::data(vertices), GL_STATIC_DRAW);
}

bool Application::TestInit()
{
    glfwSetErrorCallback(ErrorCallback);

    if (!glfwInit())
        return false;

    mWindow = glfwCreateWindow(640, 480, "Test Window", nullptr, nullptr);
    if (!mWindow)
        return false;

    glfwMakeContextCurrent(mWindow);
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
        return false;

    glfwSwapInterval(1);

    glClearColor(1.0f, 0.0f, 1.0f, 1.0f);

    glCreateVertexArrays(1, &mVAO);
    glBindVertexArray(mVAO);

    InitTestVertexBuffer();
    InitIndexBuffer();
    InitTestShader();

    return true;
}

void Application::TestShutdown()
{
    mTestShader.Delete();

    if (mTestVB)
    {
        glDeleteBuffers(1, &mTestVB);
        mTestVB = 0;
    }

    if (mIndexBuffer)
    {
        glDeleteBuffers(1, &mIndexBuffer);
        mIndexBuffer = 0;
    }

    if (mVAO)
    {
        glDeleteVertexArrays(1, &mVAO);
        mVAO = 0;
    }

    if (mWindow)
    {
        glfwDestroyWindow(mWindow);
        mWindow = nullptr;
    }

    glfwTerminate();
}

void Application::InitTestShader()
{
    constexpr char* vertexSrc = "#version 450 core\n"
        "layout(location = 0) in vec2 a_Position;\n"
        "void main() {\n"
        "gl_Position = vec4(a_Position, 0.0, 1.0);\n"
        "}";

    constexpr char* fragmentSrc = "#version 450 core\n"
        "out vec4 color;\n"
        "void main() {\n"
        "color = vec4(1.0, 1.0, 1.0, 1.0);\n"
        "}";

    mTestShader.Create(vertexSrc, fragmentSrc);
    mTestShader.Bind();
    mTestShader.SetVertexAttribf("a_Position", 2);
}

void Application::TestRun()
{
    while (!glfwWindowShouldClose(mWindow))
    {
        const TimeStep ts = (float)glfwGetTime();

        glClear(GL_COLOR_BUFFER_BIT);

        glBindVertexArray(mVAO);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, nullptr);

        glfwSwapBuffers(mWindow);

        glfwPollEvents();
    }
}
