#include "OpcodeLogWindow.h"

#include "Utils/FileUtils.h"

#include <imgui.h>
#include <ImGuiFileDialog.h>

constexpr char* SaveLogFileDialogKey = "SaveLogFile";

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

    if (ImGuiFileDialog::Instance()->Display(SaveLogFileDialogKey, ImGuiWindowFlags_NoCollapse, ImVec2(600.0f, 400.0f)))
    {
        if (ImGuiFileDialog::Instance()->IsOk())
        {
            const std::filesystem::path file = ImGuiFileDialog::Instance()->GetFilePathName();
            FileUtils::WriteLines(file, mLines);
        }

        ImGuiFileDialog::Instance()->Close();
    }
}

void OpcodeLogWindow::SaveLogToFile()
{
    ImGuiFileDialog::Instance()->OpenDialog(SaveLogFileDialogKey, "Save Log", "Log (*.log){.log}", "Resources");
}
