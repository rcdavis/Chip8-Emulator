#include "BaseImGuiWindow.h"

#include <imgui.h>

BaseImGuiWindow::BaseImGuiWindow(const std::string& title, const std::string& saveId, bool isOpen) :
    mTitle(title),
    mSaveId(saveId),
    mIsOpen(isOpen)
{}

BaseImGuiWindow::~BaseImGuiWindow() {}

bool BaseImGuiWindow::RenderMenuItem(const char* const shortcut)
{
    return ImGui::MenuItem(std::data(mTitle), shortcut, &mIsOpen);
}

void BaseImGuiWindow::Render()
{
    if (mIsOpen)
    {
        if (ImGui::Begin(std::data(mTitle), &mIsOpen))
            OnRender();

        ImGui::End();
    }
}
