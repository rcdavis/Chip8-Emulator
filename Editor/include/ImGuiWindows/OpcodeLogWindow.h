#pragma once

#include "ImGuiWindows/BaseImGuiWindow.h"

#include <string>
#include <vector>

struct GLFWwindow;

class OpcodeLogWindow : public BaseImGuiWindow {
public:
	OpcodeLogWindow(bool isOpen = false);

	void AddLine(const std::string& line) { mLines.push_back(line); }
	void Clear() { mLines.clear(); }

	void SetWindow(GLFWwindow* const window) { mWindow = window; }

protected:
	void OnRender() override;

private:
	void SaveLogToFile();

private:
	std::vector<std::string> mLines;
	GLFWwindow* mWindow = nullptr;
};
