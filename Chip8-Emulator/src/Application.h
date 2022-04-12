#pragma once

#include "Chip8.h"
#include "OpenGL/OpenGLShader.h"

#include <vector>
#include <string>
#include <filesystem>

struct GLFWwindow;

class Application
{
public:
    Application() = default;
    ~Application();

    bool Init();
    void Shutdown();

    void Run();
    void UpdateInput();

    bool TestInit();
    void TestShutdown();
    void TestRun();

private:
    struct GameEntry
    {
        std::string name;
        std::filesystem::path filepath;
    };

    std::vector<GameEntry> GetGameList();
    std::filesystem::path ChooseGame();
    static void ErrorCallback(int error, const char* description);

    void InitVertexBuffer();
    void InitIndexBuffer();
    void InitTexture();
    void InitShader();

    void InitTestVertexBuffer();
    void InitTestShader();

private:
    Chip8 mChip8;
    GLFWwindow* mWindow = nullptr;
    uint32_t mVAO = 0;
    uint32_t mVertexBuffer = 0;
    uint32_t mIndexBuffer = 0;
    uint32_t mPBO = 0;
    uint32_t mTexture = 0;
    OpenGLShader mShader;

    uint32_t mTestVB = 0;
    OpenGLShader mTestShader;
};
