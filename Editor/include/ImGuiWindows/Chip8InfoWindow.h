#pragma once

#include "ImGuiWindow.h"

class Chip8;

class Chip8InfoWindow : public ImGuiWindow
{
public:
    Chip8InfoWindow(bool isOpen = false);

    void SetChip8(Chip8* const chip8) { mChip8 = chip8; }

protected:
    void OnRender() override;

private:
    Chip8* mChip8 = nullptr;
};
