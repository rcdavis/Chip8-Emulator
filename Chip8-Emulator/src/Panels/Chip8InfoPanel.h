#pragma once

class Chip8;

class Chip8InfoPanel
{
public:
    Chip8InfoPanel() = default;

    void Render();

    void SetChip8(Chip8* const chip8) { mChip8 = chip8; }

private:
    Chip8* mChip8 = nullptr;
    bool mIsOpen = true;
};
