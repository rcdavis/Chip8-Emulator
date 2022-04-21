#include "Application.h"

#include "Log.h"
#include "Utils/PlatformUtils.h"
#include "Utils/FileUtils.h"

#include <GLFW/glfw3.h>
#include <glad/glad.h>

#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include <fstream>
#include <array>

constexpr char* DialogFilter = "Chip8 Game (*.c8, *.ch8)\0*.c8;*.ch8\0\0";

struct Vertex
{
    float posX = 0.0f;
    float posY = 0.0f;

    float texCoordU = 0.0f;
    float texCoordV = 0.0f;
};

Application::~Application()
{
    Shutdown();
}

bool Application::Init()
{
    glfwSetErrorCallback(ErrorCallback);

    if (!glfwInit())
        return false;

    mWindow = glfwCreateWindow(1600, 900, "Chip8 Emulator", nullptr, nullptr);
    if (!mWindow)
        return false;

    glfwMakeContextCurrent(mWindow);
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
        return false;

    glfwSwapInterval(1);

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    mFrameBuffer.Create(mChip8.GetScreenWidth(), mChip8.GetScreenHeight());

    glCreateVertexArrays(1, &mVAO);
    glBindVertexArray(mVAO);

    InitVertexBuffer();
    InitIndexBuffer();
    InitTexture();
    InitShader();
    InitImGui();

    glfwSetWindowUserPointer(mWindow, this);

    glfwSetFramebufferSizeCallback(mWindow, [](GLFWwindow* window, int width, int height) {
        glViewport(0, 0, width, height);
    });

    glfwSetKeyCallback(mWindow, [](GLFWwindow* window, int key, int scancode, int action, int mods) {
        Application& app = *(Application*)glfwGetWindowUserPointer(window);
        app.KeyCallback(key, scancode, action, mods);
    });

    mMemoryEditor.Open = false;
    mMemoryEditor.OptShowDataPreview = true;

    mVramEditor.Open = false;
    mVramEditor.OptShowDataPreview = true;

    mChip8.SetUpdateInputFunc(std::bind(&Application::UpdateInput, this, std::placeholders::_1));
    mChip8.SetRenderFunc(std::bind(&Application::DrawChip8, this, std::placeholders::_1));
    mChip8.SetOpcodeLogFunc(std::bind(&Application::AddOpcodeLogLine, this, std::placeholders::_1));

    mFrameBuffer.Bind();
    glClear(GL_COLOR_BUFFER_BIT);
    mFrameBuffer.Unbind();

    mOpcodeLogPanel.SetWindow(mWindow);

    LoadEmulatorSettings();

    return true;
}

void Application::Shutdown()
{
    SaveEmulatorSettings();

    if (mImGuiInitialized)
    {
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
        mImGuiInitialized = false;
    }

    mShader.Delete();
    mFrameBuffer.Destroy();

    if (mTexture)
    {
        glDeleteTextures(1, &mTexture);
        mTexture = 0;
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
        mChip8.Emulate();

        ImGuiBeginFrame();

        ImGuiRender();

        ImGuiEndFrame();

        glfwSwapBuffers(mWindow);

        glfwPollEvents();
    }
}

void Application::UpdateInput(std::array<uint8_t, 16>& keys)
{
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

void Application::KeyCallback(int key, int scancode, int action, int mods)
{
    if (action == GLFW_RELEASE)
    {
        if (key == GLFW_KEY_O && mods == GLFW_MOD_CONTROL)
            LoadGame();
        else if (mods == GLFW_MOD_SHIFT)
        {
            if (key == GLFW_KEY_F1)
                mChip8.SaveState(1);
            else if (key == GLFW_KEY_F2)
                mChip8.SaveState(2);
            else if (key == GLFW_KEY_F3)
                mChip8.SaveState(3);
            else if (key == GLFW_KEY_F4)
                mChip8.SaveState(4);
            else if (key == GLFW_KEY_F5)
                mChip8.SaveState(5);
            else if (key == GLFW_KEY_F6)
                mChip8.SaveState(6);
            else if (key == GLFW_KEY_F7)
                mChip8.SaveState(7);
            else if (key == GLFW_KEY_F8)
                mChip8.SaveState(8);
            else if (key == GLFW_KEY_F9)
                mChip8.SaveState(9);
            else if (key == GLFW_KEY_F10)
                mChip8.SaveState(10);
        }
        else if (key == GLFW_KEY_F1)
            mChip8.LoadState(1);
        else if (key == GLFW_KEY_F2)
            mChip8.LoadState(2);
        else if (key == GLFW_KEY_F3)
            mChip8.LoadState(3);
        else if (key == GLFW_KEY_F4)
            mChip8.LoadState(4);
        else if (key == GLFW_KEY_F5)
            mChip8.LoadState(5);
        else if (key == GLFW_KEY_F6)
            mChip8.LoadState(6);
        else if (key == GLFW_KEY_F7)
            mChip8.LoadState(7);
        else if (key == GLFW_KEY_F8)
            mChip8.LoadState(8);
        else if (key == GLFW_KEY_F9)
            mChip8.LoadState(9);
        else if (key == GLFW_KEY_F10)
            mChip8.LoadState(10);
    }
}

void Application::ErrorCallback(int error, const char* description)
{
    LOG_ERROR("GLFW ERROR {}: {}", error, description);
}

void Application::InitVertexBuffer()
{
    // 2 for position, 2 for texture coordinates
    constexpr std::array<Vertex, 4> vertices = {{
        { -1.0f, -1.0f, 0.0f, 1.0f },
        { 1.0f, -1.0f, 1.0f, 1.0f },
        { 1.0f, 1.0f, 1.0f, 0.0f },
        { -1.0f, 1.0f, 0.0f, 0.0f }
    }};

    glGenBuffers(1, &mVertexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, mVertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * std::size(vertices),
        std::data(vertices), GL_STATIC_DRAW);
}

void Application::InitIndexBuffer()
{
    constexpr std::array<uint16_t, 6> indices = {
        0, 1, 2,
        2, 3, 0
    };

    glGenBuffers(1, &mIndexBuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mIndexBuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint16_t) * std::size(indices),
        std::data(indices), GL_STATIC_DRAW);
}

void Application::InitTexture()
{
    glActiveTexture(GL_TEXTURE0);
    glGenTextures(1, &mTexture);
    glBindTexture(GL_TEXTURE_2D, mTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, mChip8.GetScreenWidth(), mChip8.GetScreenHeight(),
        0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTextureParameteri(mTexture, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTextureParameteri(mTexture, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
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
        "gl_Position = vec4(a_Position, 0.0, 1.0);\n"
        "}";

    constexpr char* fragmentSrc = "#version 450 core\n"
        "layout(location = 0) in vec2 v_TexCoord;\n"
        "out vec4 color;\n"
        "layout(binding = 0) uniform sampler2D u_Texture;\n"
        "void main() {\n"
        "color = texture(u_Texture, v_TexCoord);\n"
        "}";

    mShader.Create(vertexSrc, fragmentSrc);
    mShader.Bind();
    mShader.SetVertexAttribf("a_Position", 2, sizeof(Vertex));
    mShader.SetVertexAttribf("a_TexCoord", 2, sizeof(Vertex), sizeof(float) * 2);
}

void Application::InitImGui()
{
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

    ImGui::StyleColorsDark();

    ImGuiStyle& style = ImGui::GetStyle();
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        style.WindowRounding = 0.0f;
        style.Colors[ImGuiCol_WindowBg].w = 1.0f;
    }

    ImGui_ImplGlfw_InitForOpenGL(mWindow, true);
    ImGui_ImplOpenGL3_Init("#version 450 core");

    mImGuiInitialized = true;
}

void Application::ImGuiBeginFrame()
{
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

void Application::ImGuiEndFrame()
{
    int width = 0, height = 0;
    glfwGetWindowSize(mWindow, &width, &height);
    ImGuiIO& io = ImGui::GetIO();
    io.DisplaySize = ImVec2((float)width, (float)height);

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        GLFWwindow* backupCurrentContext = glfwGetCurrentContext();
        ImGui::UpdatePlatformWindows();
        ImGui::RenderPlatformWindowsDefault();
        glfwMakeContextCurrent(backupCurrentContext);
    }
}

void Application::ImGuiRender()
{
    static bool dockspaceOpen = true;
    static bool optFullscreen = true;
    static ImGuiDockNodeFlags dockspaceFlags = ImGuiDockNodeFlags_None;
    ImGuiWindowFlags windowFlags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;

    if (optFullscreen)
    {
        ImGuiViewport* viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(viewport->Pos);
        ImGui::SetNextWindowSize(viewport->Size);
        ImGui::SetNextWindowViewport(viewport->ID);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
        windowFlags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
        windowFlags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
    }

    if (dockspaceFlags & ImGuiDockNodeFlags_PassthruCentralNode)
        windowFlags |= ImGuiWindowFlags_NoBackground;

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    ImGui::Begin("DockSpace Demo", &dockspaceOpen, windowFlags);
    ImGui::PopStyleVar();

    if (optFullscreen)
        ImGui::PopStyleVar(2);

    ImGuiIO& io = ImGui::GetIO();
    if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
        ImGui::DockSpace(ImGui::GetID("MyDockSpace"), ImVec2(0.0f, 0.0f), dockspaceFlags);

    ImGuiMainMenuRender();

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{ 0.0f, 0.0f });
    ImGui::Begin("Game Viewport");

    auto viewportMinRegion = ImGui::GetWindowContentRegionMin();
    auto viewportMaxRegion = ImGui::GetWindowContentRegionMax();
    auto viewportOffset = ImGui::GetWindowPos();
    const ImVec2 viewportPanelSize = ImGui::GetContentRegionAvail();

    const uint32_t texId = mFrameBuffer.GetColorAttachmentRendererId();
    ImGui::Image((ImTextureID)texId, viewportPanelSize, ImVec2{ 0.0f, 1.0f }, ImVec2{ 1.0f, 0.0f });

    ImGui::End();
    ImGui::PopStyleVar();

    if (mIsMetricsWindowOpen)
        ImGui::ShowMetricsWindow(&mIsMetricsWindowOpen);

    if (mMemoryEditor.Open)
        mMemoryEditor.DrawWindow("Memory", std::data(mChip8.GetMemory()), std::size(mChip8.GetMemory()));

    if (mVramEditor.Open)
        mVramEditor.DrawWindow("VRAM", std::data(mChip8.GetVram()), std::size(mChip8.GetVram()));

    RenderChip8InfoPanel();

    mOpcodeLogPanel.Render();

    static bool demoOpen = true;
    ImGui::ShowDemoWindow(&demoOpen);

    ImGui::End();
}

void Application::DrawChip8(const std::vector<uint32_t>& vram)
{
    mFrameBuffer.Bind();
    glClear(GL_COLOR_BUFFER_BIT);
    glBindVertexArray(mVAO);

    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, mChip8.GetScreenWidth(), mChip8.GetScreenHeight(),
        GL_RGBA, GL_UNSIGNED_BYTE, std::data(vram));

    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, nullptr);
    mFrameBuffer.Unbind();
}

void Application::ImGuiMainMenuRender()
{
    if (ImGui::BeginMainMenuBar())
    {
        if (ImGui::BeginMenu("File"))
        {
            if (ImGui::MenuItem("Load Game", "Ctrl+O"))
                LoadGame();
            ImGui::Separator();
            if (ImGui::BeginMenu("Save State", !std::empty(mChip8.GetGameFile())))
            {
                if (ImGui::MenuItem("State 1", "Shift+F1"))
                    mChip8.SaveState(1);
                else if (ImGui::MenuItem("State 2", "Shift+F2"))
                    mChip8.SaveState(2);
                else if (ImGui::MenuItem("State 3", "Shift+F3"))
                    mChip8.SaveState(3);
                else if (ImGui::MenuItem("State 4", "Shift+F4"))
                    mChip8.SaveState(4);
                else if (ImGui::MenuItem("State 5", "Shift+F5"))
                    mChip8.SaveState(5);
                else if (ImGui::MenuItem("State 6", "Shift+F6"))
                    mChip8.SaveState(6);
                else if (ImGui::MenuItem("State 7", "Shift+F7"))
                    mChip8.SaveState(7);
                else if (ImGui::MenuItem("State 8", "Shift+F8"))
                    mChip8.SaveState(8);
                else if (ImGui::MenuItem("State 9", "Shift+F9"))
                    mChip8.SaveState(9);
                else if (ImGui::MenuItem("State 10", "Shift+F10"))
                    mChip8.SaveState(10);

                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Load State", !std::empty(mChip8.GetGameFile())))
            {
                if (ImGui::MenuItem("State 1", "F1"))
                    mChip8.LoadState(1);
                else if (ImGui::MenuItem("State 2", "F2"))
                    mChip8.LoadState(2);
                else if (ImGui::MenuItem("State 3", "F3"))
                    mChip8.LoadState(3);
                else if (ImGui::MenuItem("State 4", "F4"))
                    mChip8.LoadState(4);
                else if (ImGui::MenuItem("State 5", "F5"))
                    mChip8.LoadState(5);
                else if (ImGui::MenuItem("State 6", "F6"))
                    mChip8.LoadState(6);
                else if (ImGui::MenuItem("State 7", "F7"))
                    mChip8.LoadState(7);
                else if (ImGui::MenuItem("State 8", "F8"))
                    mChip8.LoadState(8);
                else if (ImGui::MenuItem("State 9", "F9"))
                    mChip8.LoadState(9);
                else if (ImGui::MenuItem("State 10", "F10"))
                    mChip8.LoadState(10);

                ImGui::EndMenu();
            }

            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Edit"))
        {
            if (ImGui::BeginMenu("Theme"))
            {
                if (ImGui::MenuItem("Light"))
                    ImGui::StyleColorsLight();
                else if (ImGui::MenuItem("Dark"))
                    ImGui::StyleColorsDark();

                ImGui::EndMenu();
            }

            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Windows"))
        {
            ImGui::MenuItem("ImGui Metrics", nullptr, &mIsMetricsWindowOpen);
            ImGui::MenuItem("Chip8 Info", nullptr, &mIsChip8InfoWindowOpen);
            ImGui::MenuItem("Memory", nullptr, &mMemoryEditor.Open);
            ImGui::MenuItem("VRAM", nullptr, &mVramEditor.Open);

            bool isOpcodeLogOpen = mOpcodeLogPanel.IsOpen();
            ImGui::MenuItem("Opcode Log", nullptr, &isOpcodeLogOpen);
            mOpcodeLogPanel.Open(isOpcodeLogOpen);

            ImGui::EndMenu();
        }

        ImGui::EndMainMenuBar();
    }
}

void Application::LoadGame()
{
    auto filepath = FileDialogs::OpenFile(mWindow, DialogFilter);
    if (filepath)
        mChip8.LoadGame(*filepath);
}

void Application::RenderChip8InfoPanel()
{
    if (!mIsChip8InfoWindowOpen)
        return;

    if (ImGui::Begin("Chip8 Info", &mIsChip8InfoWindowOpen))
    {
        ImVec4 drawnColor = ImGui::ColorConvertU32ToFloat4(mChip8.GetDrawnColor());
        ImGui::ColorEdit4("Drawn", &drawnColor.x);
        mChip8.SetDrawnColor(ImGui::ColorConvertFloat4ToU32(drawnColor));

        ImVec4 undrawnColor = ImGui::ColorConvertU32ToFloat4(mChip8.GetUndrawnColor());
        ImGui::ColorEdit4("Undrawn", &undrawnColor.x);
        mChip8.SetUndrawnColor(ImGui::ColorConvertFloat4ToU32(undrawnColor));

        int emuSpeed = (int)mChip8.GetEmuSpeedModifier();
        ImGui::SliderInt("Emu Speed Modifier", &emuSpeed, 1, 10);
        mChip8.SetEmuSpeedModifier((uint8_t)emuSpeed);

        ImGui::Separator();

        ImGui::Text("Quirk Flags");

        bool useVYShiftQuirk = mChip8.GetUseVYForShiftQuirk();
        ImGui::Checkbox("Use VY For Shift", &useVYShiftQuirk);
        mChip8.SetUseVYForShiftQuirk(useVYShiftQuirk);

        bool useBXNN = mChip8.GetUseBXNNQuirk();
        ImGui::Checkbox("Use BXNN", &useBXNN);
        mChip8.SetUseBXNNQuirk(useBXNN);

        bool useIndexIncrement = mChip8.GetUseIndexIncrementAfterStoreLoadQuirk();
        ImGui::Checkbox("Use Index Increment After Store and Load Memory", &useIndexIncrement);
        mChip8.SetUseIndexIncrementAfterStoreLoadQuirk(useIndexIncrement);

        ImGui::Separator();

        ImGui::Text("Opcode: %X", mChip8.GetOpcode());
        ImGui::Text("Index Reg: %X", mChip8.GetIndexReg());
        ImGui::Text("Program Counter: %X", mChip8.GetProgramCounter());
        ImGui::Text("Stack Pointer: %d", mChip8.GetStackPointer());
        ImGui::Text("Delay Timer: %d", mChip8.GetDelayTimer());
        ImGui::Text("Sound Timer: %d", mChip8.GetSoundTimer());

        ImGui::Separator();

        const auto vreg = mChip8.GetVReg();
        for (size_t i = 0; i < std::size(vreg); ++i)
            ImGui::Text("V%X: %d", i, vreg[i]);

        ImGui::Separator();

        const auto& keys = mChip8.GetKeys();
        for (size_t i = 0; i < std::size(keys); ++i)
            ImGui::Text("Key[%X] = %d", i, keys[i]);
    }

    ImGui::End();
}

void Application::LoadEmulatorSettings()
{
    std::ifstream file("emulator.ini");
    if (!file)
    {
        LOG_WARN("Failed to load emulator settings");
        return;
    }

    std::string line;
    while (std::getline(file, line))
    {
        const auto index = line.find_first_of('=');
        if (index == std::string::npos)
            continue;

        const auto key = line.substr(0, index);
        const auto value = line.substr(index + 1);
        if (key == "metricsWindowOpen")
            mIsMetricsWindowOpen = (bool)std::stoi(value);
        else if (key == "chip8InfoWindowOpen")
            mIsChip8InfoWindowOpen = (bool)std::stoi(value);
        else if (key == "memoryWindowOpen")
            mMemoryEditor.Open = (bool)std::stoi(value);
        else if (key == "vramWindowOpen")
            mVramEditor.Open = (bool)std::stoi(value);
        else if (key == "opcodesLogOpen")
            mOpcodeLogPanel.Open((bool)std::stoi(value));
        else if (key == "emuSpeedModifier")
            mChip8.SetEmuSpeedModifier((uint8_t)std::stoi(value));
        else if (key == "drawnColor")
            mChip8.SetDrawnColor((uint32_t)std::stoul(value));
        else if (key == "undrawnColor")
            mChip8.SetUndrawnColor((uint32_t)std::stoul(value));
    }
}

void Application::SaveEmulatorSettings()
{
    std::ofstream file("emulator.ini");
    if (!file)
    {
        LOG_ERROR("Failed to save emulator settings");
        return;
    }

    file << "metricsWindowOpen=" << mIsMetricsWindowOpen << '\n';
    file << "chip8InfoWindowOpen=" << mIsChip8InfoWindowOpen << '\n';
    file << "memoryWindowOpen=" << mMemoryEditor.Open << '\n';
    file << "vramWindowOpen=" << mVramEditor.Open << '\n';
    file << "opcodesLogOpen=" << mOpcodeLogPanel.IsOpen() << '\n';
    file << "emuSpeedModifier=" << (uint32_t)mChip8.GetEmuSpeedModifier() << '\n';
    file << "drawnColor=" << mChip8.GetDrawnColor() << '\n';
    file << "undrawnColor=" << mChip8.GetUndrawnColor() << '\n';
}

void Application::AddOpcodeLogLine(const std::string& line)
{
    if (mOpcodeLogPanel.IsOpen())
        mOpcodeLogPanel.AddLine(line);
}
