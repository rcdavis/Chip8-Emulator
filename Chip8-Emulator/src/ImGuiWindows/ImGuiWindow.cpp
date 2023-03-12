#include "ImGuiWindow.h"

#include <imgui.h>

ImGuiWindow::ImGuiWindow(const std::string& title, const std::string& saveId, bool isOpen) :
    mTitle(title),
    mSaveId(saveId),
    mIsOpen(isOpen)
{}

bool ImGuiWindow::RenderMenuItem(const char* const shortcut)
{
    return ImGui::MenuItem(std::data(mTitle), shortcut, &mIsOpen);
}

void ImGuiWindow::Render()
{
    if (mIsOpen)
    {
        if (ImGui::Begin(std::data(mTitle), &mIsOpen))
            OnRender();

        ImGui::End();
    }
}
