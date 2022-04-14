#include "Application.h"

#include "Log.h"
#include "TimeStep.h"

#include <GLFW/glfw3.h>
#include <glad/glad.h>

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include <fstream>
#include <array>

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
    const std::filesystem::path game = ChooseGame();
    if (std::empty(game))
        return false;

    mChip8.LoadGame(game);

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

    glClearColor(1.0f, 0.0f, 1.0f, 1.0f);
    glEnable(GL_DEPTH_TEST);

    mFrameBuffer.Create(Chip8::SCREEN_WIDTH, Chip8::SCREEN_HEIGHT);

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

    return true;
}

void Application::Shutdown()
{
    if (imGuiInitialized)
    {
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
        imGuiInitialized = false;
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
        const TimeStep ts = (float)glfwGetTime();

        UpdateInput();
        mChip8.EmulateCycle();

        if (mChip8.mRedraw)
        {
            DrawChip8();

            mChip8.mRedraw = false;
        }

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        ImGuiBeginFrame();

        ImGuiRender();

        ImGuiEndFrame();

        glfwSwapBuffers(mWindow);

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

void Application::KeyCallback(int key, int scancode, int action, int mods)
{
    if (action == GLFW_RELEASE)
    {
        if (key == GLFW_KEY_F1)
            mChip8.SaveState();
        else if (key == GLFW_KEY_F2)
            mChip8.LoadState();
    }
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

    // TODO: Make possible to choose a game
    return games[0].filepath;
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
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, Chip8::SCREEN_WIDTH, Chip8::SCREEN_HEIGHT,
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

    imGuiInitialized = true;
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
    io.DisplaySize = ImVec2(width, height);

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

    if (ImGui::BeginMenuBar())
    {
        if (ImGui::BeginMenu("File"))
        {
            if (ImGui::MenuItem("Save State"))
                mChip8.SaveState();

            ImGui::EndMenu();
        }

        ImGui::EndMenuBar();
    }

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{ 0.0f, 0.0f });
    ImGui::Begin("Viewport");

    auto viewportMinRegion = ImGui::GetWindowContentRegionMin();
    auto viewportMaxRegion = ImGui::GetWindowContentRegionMax();
    auto viewportOffset = ImGui::GetWindowPos();
    std::array<ImVec2, 2> viewportBounds = {{
        { viewportMinRegion.x + viewportOffset.x, viewportMinRegion.y + viewportOffset.y },
        { viewportMaxRegion.x + viewportOffset.x, viewportMaxRegion.y + viewportOffset.y }
    }};
    const ImVec2 viewportPanelSize = ImGui::GetContentRegionAvail();

    const uint32_t texId = mFrameBuffer.GetColorAttachmentRendererId();
    ImGui::Image((ImTextureID)texId, viewportPanelSize, ImVec2{ 0.0f, 1.0f }, ImVec2{ 1.0f, 0.0f });

    ImGui::End();
    ImGui::PopStyleVar();

    static bool isOpen = true;
    ImGui::ShowDemoWindow(&isOpen);

    ImGui::End();
}

void Application::DrawChip8()
{
    mFrameBuffer.Bind();
    glClearColor(1.0f, 0.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    glBindVertexArray(mVAO);

    const auto vramImage = mChip8.GetVramImage();
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, Chip8::SCREEN_WIDTH, Chip8::SCREEN_HEIGHT,
        GL_RGBA, GL_UNSIGNED_BYTE, std::data(vramImage));

    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, nullptr);
    mFrameBuffer.Unbind();
}
