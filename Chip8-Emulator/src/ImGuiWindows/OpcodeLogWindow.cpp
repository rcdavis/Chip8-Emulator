#include "OpcodeLogWindow.h"

#include "Utils/FileUtils.h"
#include "Utils/PlatformUtils.h"

#include <imgui.h>

OpcodeLogWindow::OpcodeLogWindow(bool isOpen) :
    ImGuiWindow("Opcode Log", "opcodesLogOpen", isOpen)
{}

void OpcodeLogWindow::OnRender()
{
    if (ImGui::Button("Clear"))
        mLines.clear();
    ImGui::SameLine();
    if (ImGui::Button("Save To File"))
        SaveLogToFile();

    ImGui::BeginChild("OpcodeLog");
    for (const auto& line : mLines)
        ImGui::Text(line.c_str());
    ImGui::EndChild();
}

void OpcodeLogWindow::SaveLogToFile()
{
    constexpr char* DialogFilter = "Log File (*.log)\0*.log\0\0";
    auto filepath = FileDialogs::SaveFile(mWindow, DialogFilter);
    if (!filepath)
        return;

    FileUtils::WriteLines(*filepath, mLines);
}
