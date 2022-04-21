#pragma once

#include "Chip8.h"
#include "OpenGL/OpenGLShader.h"
#include "OpenGL/OpenGLFramebuffer.h"
#include "OpenGL/OpenGLTexture.h"
#include "Panels/OpcodeLogPanel.h"
#include "Panels/Chip8InfoPanel.h"

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

    void KeyCallback(int key, int scancode, int action, int mods);

private:
    static void ErrorCallback(int error, const char* description);

    void InitVertexBuffer();
    void InitIndexBuffer();
    void InitShader();
    void InitImGui();

    void ImGuiBeginFrame();
    void ImGuiEndFrame();
    void ImGuiRender();

    void ImGuiMainMenuRender();

    void UpdateInput(std::array<uint8_t, 16>& keys);
    void DrawChip8(const std::vector<uint32_t>& vram);

    void LoadGame();

    void LoadEmulatorSettings();
    void SaveEmulatorSettings();

    void AddOpcodeLogLine(const std::string& line);

private:
    enum class Theme
    {
        Light,
        Dark
    };

private:
    Chip8 mChip8;
    GLFWwindow* mWindow = nullptr;
    uint32_t mVAO = 0;
    uint32_t mVertexBuffer = 0;
    uint32_t mIndexBuffer = 0;
    OpenGLShader mShader;
    OpenGLFramebuffer mFrameBuffer;
    OpenGLTexture mTexture;

    OpcodeLogPanel mOpcodeLogPanel;
    Chip8InfoPanel mChip8InfoPanel;

    MemoryEditor mMemoryEditor;
    MemoryEditor mVramEditor;

    Theme mTheme = Theme::Dark;

    bool mImGuiInitialized = false;
    bool mIsMetricsWindowOpen = true;
    bool mIsChip8InfoWindowOpen = true;
};
