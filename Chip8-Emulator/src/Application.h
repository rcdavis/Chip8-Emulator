#pragma once

#include "Chip8.h"
#include "OpenGL/OpenGLShader.h"
#include "OpenGL/OpenGLFramebuffer.h"

#include <imgui.h>
#include <imgui_memory_editor.h>

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

    void KeyCallback(int key, int scancode, int action, int mods);

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
    void InitImGui();

    void ImGuiBeginFrame();
    void ImGuiEndFrame();
    void ImGuiRender();

    void ImGuiMainMenuRender();

    void DrawChip8();

    void LoadGame();

    void RenderChip8InfoPanel();

private:
    Chip8 mChip8;
    GLFWwindow* mWindow = nullptr;
    uint32_t mVAO = 0;
    uint32_t mVertexBuffer = 0;
    uint32_t mIndexBuffer = 0;
    uint32_t mTexture = 0;
    OpenGLShader mShader;
    OpenGLFramebuffer mFrameBuffer;

    MemoryEditor mMemoryEditor;
    MemoryEditor mVramEditor;

    bool mImGuiInitialized = false;
    bool mIsMetricsWindowOpen = true;
    bool mIsChip8InfoWindowOpen = true;
};
