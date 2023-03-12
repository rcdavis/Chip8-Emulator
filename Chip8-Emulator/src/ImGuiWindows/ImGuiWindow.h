#pragma once

#include <string>

class ImGuiWindow
{
public:
    ImGuiWindow(const std::string& title, const std::string& saveId, bool isOpen = false);
    virtual ~ImGuiWindow() {}

    const std::string& GetTitle() const { return mTitle; }
    void SetTitle(const std::string& title) { mTitle = title; }

    const std::string& GetSaveId() const { return mSaveId; }

    bool IsOpen() const { return mIsOpen; }
    void Open(bool isOpen) { mIsOpen = isOpen; }

    virtual bool RenderMenuItem(const char* const shortcut = nullptr);
    void Render();

protected:
    virtual void OnRender() = 0;

protected:
    std::string mTitle;
    std::string mSaveId;
    bool mIsOpen = false;
};
