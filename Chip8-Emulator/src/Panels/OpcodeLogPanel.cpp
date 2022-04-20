#include "OpcodeLogPanel.h"

#include "Utils/FileUtils.h"
#include "Utils/PlatformUtils.h"

#include <imgui.h>

void OpcodeLogPanel::Render()
{
    if (!mIsOpen)
        return;

    ImGui::Begin("Opcode Log", &mIsOpen);

    if (ImGui::Button("Clear"))
        mLines.clear();
    ImGui::SameLine();
    if (ImGui::Button("Save To File"))
        SaveLogToFile();

    ImGui::BeginChild("OpcodeLog");
    for (const auto& line : mLines)
        ImGui::Text(line.c_str());
    ImGui::EndChild();

    ImGui::End();
}

void OpcodeLogPanel::SaveLogToFile()
{
    constexpr char* DialogFilter = "Log File (*.log)\0*.log\0\0";
    auto filepath = FileDialogs::SaveFile(mWindow, DialogFilter);
    if (!filepath)
        return;

    FileUtils::WriteLines(*filepath, mLines);
}
