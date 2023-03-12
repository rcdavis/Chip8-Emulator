#pragma once

#include <string>
#include <vector>

struct GLFWwindow;

class OpcodeLogPanel
{
public:
    OpcodeLogPanel() = default;

    void Render();

    void AddLine(const std::string& line) { mLines.push_back(line); }
    void Clear() { mLines.clear(); }

    bool IsOpen() const { return mIsOpen; }
    void Open(const bool isOpen) { mIsOpen = isOpen; }

    void SetWindow(GLFWwindow* const window) { mWindow = window; }

private:
    void SaveLogToFile();

private:
    std::vector<std::string> mLines;
    GLFWwindow* mWindow = nullptr;
    bool mIsOpen = false;
};
