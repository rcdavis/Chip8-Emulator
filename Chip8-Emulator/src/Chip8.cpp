
#include "Chip8.h"

#include <algorithm>
#include <fstream>
#include <stdexcept>

Chip8::Chip8()
{
    srand((unsigned int)time(0));

    Init();
}

void Chip8::Init()
{
    mMemory.fill(0);
    mV.fill(0);
    mVram.fill(0);
    mStack.fill(0);
    mKeys.fill(0);

    mOpcode = 0;

    mIndexReg = 0;
    mSP = 0;
    mPC = 0x200;

    mDelayTimer = 0;
    mSoundTimer = 0;

    mFrameRate = 60;

    mRedraw = true;

    constexpr std::array<const uint8_t, 80> fontSet =
    {
        0xF0, 0x90, 0x90, 0x90, 0xF0, //0
        0x20, 0x60, 0x20, 0x20, 0x70, //1
        0xF0, 0x10, 0xF0, 0x80, 0xF0, //2
        0xF0, 0x10, 0xF0, 0x10, 0xF0, //3
        0x90, 0x90, 0xF0, 0x10, 0x10, //4
        0xF0, 0x80, 0xF0, 0x10, 0xF0, //5
        0xF0, 0x80, 0xF0, 0x90, 0xF0, //6
        0xF0, 0x10, 0x20, 0x40, 0x40, //7
        0xF0, 0x90, 0xF0, 0x90, 0xF0, //8
        0xF0, 0x90, 0xF0, 0x10, 0xF0, //9
        0xF0, 0x90, 0xF0, 0x90, 0x90, //A
        0xE0, 0x90, 0xE0, 0x90, 0xE0, //B
        0xF0, 0x80, 0x80, 0x80, 0xF0, //C
        0xE0, 0x90, 0x90, 0x90, 0xE0, //D
        0xF0, 0x80, 0xF0, 0x80, 0xF0, //E
        0xF0, 0x80, 0xF0, 0x80, 0x80  //F
    };

    std::copy(std::cbegin(fontSet), std::cend(fontSet), std::begin(mMemory));
}

void Chip8::EmulateCycle()
{
    mOpcode = GetOpcode();
    mPC += 2;

    switch (mOpcode & 0xF000)
    {
    case 0x0000:
    {
        switch (mOpcode & 0x000F)
        {
        case 0x0000: // 0x00E0: Clears the screen
        {
            mVram.fill(0);
            mRedraw = true;
        }
        break;

        case 0x000E: // 0x00EE: Returns from subroutine
            mPC = mStack[--mSP];
            break;
        }
    }
    break;

    case 0x1000: // 0x1NNN Jump to address NNN
        mPC = GetAddress();
        break;

    case 0x2000: // 0x2NNN Calls subroutine at NNN
    {
        mStack[mSP++] = mPC;
        mPC = GetAddress();
    }
    break;

    case 0x3000: // 0x3XNN Skips next instruction if VX equals NN
    {
        if (GetVX() == GetNN())
            mPC += 2;
    }
    break;

    case 0x4000: // 0x4XNN Skips next instruction if VX does not equal NN
    {
        if (GetVX() != GetNN())
            mPC += 2;
    }
    break;

    case 0x5000: // 0x5XY0 Skips next instruction if VX equals VY
    {
        if (GetVX() == GetVY())
            mPC += 2;
    }
    break;

    case 0x6000: // 0x6XNN Sets VX to NN
        SetVX(GetNN());
        break;

    case 0x7000: // 0x7XNN Adds NN to VX (Carry flag is not changed)
        SetVX(GetVX() + GetNN());
        break;

    case 0x8000:
    {
        switch (mOpcode & 0x000F)
        {
        case 0x0000: // 0x8XY0 Set VX to VY
            SetVX(GetVY());
            break;

        case 0x0001: // 0x8XY1 Set VX to VX or VY
            SetVX(GetVX() | GetVY());
            break;

        case 0x0002: // 0x8XY2 Set VX to VX and VY
            SetVX(GetVX() & GetVY());
            break;

        case 0x0003: // 0x8XY3 Set VX to VX xor VY
            SetVX(GetVX() ^ GetVY());
            break;

        case 0x0004: // 0x8XY4 Adds VY to VX. VF is set when there's a carry
        {
            if (GetVY() > (0xFF - GetVX()))
                SetVF(1);
            else
                SetVF(0);
            SetVX(GetVX() + GetVY());
        }
        break;

        case 0x0005: // 0x8XY5 VY is subtracted from VX. VF is set to 0 if borrow
        {
            if (GetVY() > GetVX())
                SetVF(0);
            else
                SetVF(1);
            SetVX(GetVX() - GetVY());
        }
        break;

        case 0x0006: // 0x8XY6 Stores the least significant bit of VX in VF and then shifts VX to the right by 1.
        {
            SetVF(GetVX() & 0x1);
            SetVX(GetVX() >> 1);
        }
        break;

        case 0x0007: // 0x8XY7 Sets VX to VY minus VX. VF is set to 0 when there's a borrow, and 1 when there isn't.
        {
            if (GetVX() > GetVY())
                SetVF(0);
            else
                SetVF(1);
            SetVX(GetVY() - GetVX());
        }
        break;

        case 0x000E: // 0x8XYE Stores the most significant bit of VX in VF and then shifts VX to the left by 1.
        {
            SetVF(GetVX() >> 7);
            SetVX(GetVX() << 1);
        }
        break;
        }
    }
    break;

    case 0x9000: // 0x9XY0 Skips the next instruction if VX does not equal VY
    {
        if (GetVX() != GetVY())
            mPC += 2;
    }
    break;

    case 0xA000: // 0xANNN Sets I to the address NNN
        mIndexReg = GetAddress();
        break;

    case 0xB000: // 0xBNNN Jumps to the address NNN plus V0
        mPC = GetAddress() + mV[0];
        break;

    case 0xC000: // 0xCXNN Sets VX to the result of bitwise and op on a random number and NN
        SetVX((rand() & 0xFF) & GetNN());
        break;

    /*
     * 0xDXYN
     * Draws a sprite at coordinate (VX, VY) that has a width of 8 pixels and a height of N pixels.
     * Each row of 8 pixels is read as bit-coded starting from memory location I; I value does not change
     * after the execution of this instruction. As described above, VF is set to 1 if any screen pixels
     * are flipped from set to unset when the sprite is drawn, and to 0 if that does not happen
     */
    case 0xD000:
    {
        const uint16_t height = mOpcode & 0x000F;
        constexpr uint16_t width = 8;
        mV[0xF] = 0;

        for (uint16_t y = 0; y < height; ++y)
        {
            const uint16_t pixel = mMemory[mIndexReg + y];
            for (uint16_t x = 0; x < width; ++x)
            {
                if ((pixel & (0x80 >> x)) != 0)
                {
                    const uint16_t index = GetVX() + x + ((GetVY() + y) * SCREEN_WIDTH);
                    // TODO: The index can access out of bounds. Should figure out why.
                    if (index >= std::size(mVram))
                        continue;

                    if (mVram[index] == 1)
                        mV[0xF] = 1;

                    mVram[index] ^= 1;
                }
            }
        }

        mRedraw = true;
    }
    break;

    case 0xE000:
    {
        switch (mOpcode & 0x00FF)
        {
        case 0x009E: // 0xEX9E Skips the next instruction if the key stored in VX is pressed.
        {
            if (mKeys[GetVX()] != 0)
                mPC += 2;
        }
        break;

        case 0x00A1: // 0xEXA1 Skips the next instruction if the key stored in VX is not pressed.
        {
            if (mKeys[GetVX()] == 0)
                mPC += 2;
        }
        break;
        }
    }
    break;

    case 0xF000:
    {
        switch (mOpcode & 0x00FF)
        {
        case 0x0007: // 0xFX07 Sets VX to the value of the delay timer.
            SetVX(mDelayTimer);
            break;

        /*
        * 0xFX0A A key press is awaited, and then stored in VX.
        * (Blocking Operation. All instruction halted until next key event);
        */
        case 0x000A:
        {
            bool keyPressed = false;

            for (uint8_t i = 0; i < std::size(mKeys); ++i)
            {
                if (mKeys[i])
                {
                    SetVX(i);
                    keyPressed = true;
                    break;
                }
            }

            if (!keyPressed)
                mPC -= 2;
        }
        break;

        case 0x0015: // 0xFX15 Sets delay timer to VX.
            mDelayTimer = GetVX();
            break;

        case 0x0018: // 0xFX18 Sets sound timer to VX.
            mSoundTimer = GetVX();
            break;

        case 0x001E: // 0xFX1E Adds VX to I
        {
            /*if (mIndexReg + GetVX() > 0xFFF)
                SetVF(1);
            else
                SetVF(0);*/
            mIndexReg += GetVX();
        }
        break;

        case 0x0029: // 0xFX29 Sets I to the location of the sprite for the character in VX. Characters 0-F (in hexadecimal) are represented by a 4x5 font.
            mIndexReg = GetVX() * 5;
            break;

        /*
         * 0xFX33
         *
         * Stores the binary-coded decimal representation of VX, with the most significant of three digits
         * at the address in I, the middle digit at I plus 1, and the least significant digit at I plus 2.
         * (In other words, take the decimal representation of VX, place the hundreds digit in memory at
         * location in I, the tens digit at location I+1, and the ones digit at location I+2.)
         */
        case 0x0033:
        {
            mMemory[mIndexReg] = GetVX() / 100;
            mMemory[mIndexReg + 1] = (GetVX() / 10) % 10;
            mMemory[mIndexReg + 2] = (GetVX() % 100) % 10;
        }
        break;

        case 0x0055: // 0xFX55 Stores V0 to VX (including VX) in memory starting at address I
        {
            memcpy(std::data(mMemory) + mIndexReg, std::data(mV), (mOpcode & 0x0F00) >> 8);

            // On the original interpreter, when the operation is done, I = I + X + 1.
            mIndexReg += ((mOpcode & 0x0F00) >> 8) + 1;
        }
        break;

        case 0x0065: // 0xFX65 Fills V0 to VX (including VX) with values from memory starting at address I
        {
            memcpy(std::data(mV), std::data(mMemory) + mIndexReg, (mOpcode & 0x0F00) >> 8);

            // On the original interpreter, when the operation is done, I = I + X + 1.
            mIndexReg += ((mOpcode & 0x0F00) >> 8) + 1;
        }
        break;
        }
    }
    break;
    }

    if (mDelayTimer > 0)
        --mDelayTimer;

    if (mSoundTimer > 0)
    {
        if (mSoundTimer == 1)
            printf("BEEP!\n");

        --mSoundTimer;
    }
}

void Chip8::LoadGame(const std::filesystem::path& game)
{
    Init();

    if (game.extension() != std::filesystem::path(".c8") &&
        game.extension() != std::filesystem::path(".ch8"))
    {
        const std::string error = game.generic_string() + " isn't a .c8 or .ch8 file";
        printf("%s\n", error.c_str());
        throw std::invalid_argument(error);
    }

    std::ifstream f(game, std::ios_base::in | std::ios_base::binary);
    if (!f)
    {
        const std::string error = "Unable to open game: " + game.generic_string();
        printf("%s\n", error.c_str());
        throw std::invalid_argument(error);
    }

    const size_t fileSize = std::filesystem::file_size(game);
    if (fileSize > std::size(mMemory) - 0x200)
    {
        const std::string error = "Rom " + game.string() + " is too large to fit in memory";
        printf("%s\n", error.c_str());
        throw std::invalid_argument(error);
    }

    f.read((char*)std::data(mMemory) + 0x200, fileSize);
    f.close();
}

std::array<uint32_t, Chip8::VRAM_SIZE> Chip8::GetVramImage()
{
    std::array<uint32_t, VRAM_SIZE> image = {};
    for (uint32_t i = 0; i < VRAM_SIZE; ++i)
        image[i] = (mVram[i] == 1) ? 0xFFFFFFFF : 0;

    return image;
}