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

private:
    Chip8 mChip8;
    GLFWwindow* mWindow = nullptr;
    uint32_t mVAO = 0;
    uint32_t mVertexBuffer = 0;
    uint32_t mIndexBuffer = 0;
    uint32_t mTexture = 0;
    OpenGLShader mShader;
};
