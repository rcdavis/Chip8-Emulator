#include "PlatformUtils.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <commdlg.h>
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

namespace FileDialogs
{
    std::optional<std::filesystem::path> OpenFile(GLFWwindow* window, const char* filter)
    {
        CHAR szFile[260] = {};
        CHAR curDir[256] = {};
        OPENFILENAMEA ofn = {};
        ofn.lStructSize = sizeof(OPENFILENAMEA);
        ofn.hwndOwner = glfwGetWin32Window(window);
        ofn.lpstrFile = szFile;
        ofn.nMaxFile = sizeof(szFile);
        ofn.lpstrFilter = filter;
        ofn.nFilterIndex = 1;
        ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;
        if (GetCurrentDirectoryA(sizeof(curDir), curDir))
            ofn.lpstrInitialDir = curDir;

        if (GetOpenFileNameA(&ofn) == TRUE)
            return ofn.lpstrFile;

        return {};
    }

    std::optional<std::filesystem::path> SaveFile(GLFWwindow* window, const char* filter)
    {
        CHAR szFile[260] = {};
        CHAR curDir[256] = {};
        OPENFILENAMEA ofn = {};
        ofn.lStructSize = sizeof(OPENFILENAMEA);
        ofn.hwndOwner = glfwGetWin32Window(window);
        ofn.lpstrFile = szFile;
        ofn.nMaxFile = sizeof(szFile);
        ofn.lpstrFilter = filter;
        ofn.nFilterIndex = 1;
        ofn.Flags = OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT | OFN_NOCHANGEDIR;
        if (GetCurrentDirectoryA(sizeof(curDir), curDir))
            ofn.lpstrInitialDir = curDir;

        if (GetSaveFileNameA(&ofn) == TRUE)
            return ofn.lpstrFile;

        return {};
    }
}
