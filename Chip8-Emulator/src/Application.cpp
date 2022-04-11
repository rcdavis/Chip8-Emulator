#include "Application.h"

#include "Log.h"
#include "TimeStep.h"

#include <GLFW/glfw3.h>
#include <glad/glad.h>

#include <fstream>

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

    mWindow = glfwCreateWindow(640, 480, "Chip8 Emulator", nullptr, nullptr);
    if (!mWindow)
        return false;

    glfwMakeContextCurrent(mWindow);
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
        return false;

    glfwSwapInterval(1);

    return true;
}

void Application::Shutdown()
{
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
        glfwPollEvents();

        const TimeStep ts = (float)glfwGetTime();

        glfwSwapBuffers(mWindow);
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

    LOG_INFO("Game List:");
    for (const auto& game : games)
    {
        LOG_INFO("  Game {} at {}", game.name, game.filepath.string());
    }

    // TODO: Make able to choose
    return games.front().filepath;
}

void Application::ErrorCallback(int error, const char* description)
{
    LOG_ERROR("GLFW ERROR {}: {}", error, description);
}
