#pragma once

#include <optional>
#include <filesystem>

struct GLFWwindow;

namespace FileDialogs
{
    std::optional<std::filesystem::path> OpenFile(GLFWwindow* window, const char* filter);
    std::optional<std::filesystem::path> SaveFile(GLFWwindow* window, const char* filter);
}
