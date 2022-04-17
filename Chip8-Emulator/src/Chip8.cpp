
#include "Chip8.h"

#include "Log.h"
#include "Utils/StringUtils.h"

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
    mStack.fill(0);
    mKeys.fill(0);

    switch (mGraphicsMode)
    {
    case Chip8::GraphicsMode::e64x32:
        mVram.resize(64 * 32);
        break;

    case Chip8::GraphicsMode::e128x64:
        mVram.resize(128 * 64);
        break;
    }
    memset(std::data(mVram), 0, std::size(mVram));

    mOpcode = 0;

    mIndexReg = 0;
    mSP = 0;
    mPC = 0x200;

    mDelayTimer = 0;
    mSoundTimer = 0;

    mFrameRate = 60;

    mRedraw = true;

    constexpr std::array<const uint8_t, 180> fontSet =
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
        0xF0, 0x80, 0xF0, 0x80, 0x80, //F

        // high-res mode font sprites
        0x3C, 0x7E, 0xE7, 0xC3, 0xC3, 0xC3, 0xC3, 0xE7, 0x7E, 0x3C, //0
        0x18, 0x38, 0x58, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x3C, //1
        0x3E, 0x7F, 0xC3, 0x06, 0x0C, 0x18, 0x30, 0x60, 0xFF, 0xFF, //2
        0x3C, 0x7E, 0xC3, 0x03, 0x0E, 0x0E, 0x03, 0xC3, 0x7E, 0x3C, //3
        0x06, 0x0E, 0x1E, 0x36, 0x66, 0xC6, 0xFF, 0xFF, 0x06, 0x06, //4
        0xFF, 0xFF, 0xC0, 0xC0, 0xFC, 0xFE, 0x03, 0xC3, 0x7E, 0x3C, //5
        0x3E, 0x7C, 0xC0, 0xC0, 0xFC, 0xFE, 0xC3, 0xC3, 0x7E, 0x3C, //6
        0xFF, 0xFF, 0x03, 0x06, 0x0C, 0x18, 0x30, 0x60, 0x60, 0x60, //7
        0x3C, 0x7E, 0xC3, 0xC3, 0x7E, 0x7E, 0xC3, 0xC3, 0x7E, 0x3C, //8
        0x3C, 0x7E, 0xC3, 0xC3, 0x7F, 0x3F, 0x03, 0x03, 0x3E, 0x7C  //9
    };

    memcpy(std::data(mMemory), std::data(fontSet), std::size(fontSet));
}

void Chip8::Emulate()
{
    if (std::empty(mGameFile))
        return;

    // Loop 9 times since the Chip8 runs at around ~500-600Hz
    // and the app runs at ~60Hz
    for (uint8_t i = 0; i < 9 * GetEmuSpeedModifier(); ++i)
    {
        if (mUpdateInputFunc)
            mUpdateInputFunc(mKeys);

        EmulateCycle();

        if (mRedraw)
        {
            if (mRenderFunc)
                mRenderFunc(GetVramImage());

            mRedraw = false;
        }
    }
}

void Chip8::EmulateCycle()
{
    mOpcode = GetOpcode();
    mPC += 2;

    switch (mOpcode & 0xF000)
    {
    case 0x0000:
    {
        switch (mOpcode)
        {
        case 0x00E0: // 0x00E0: Clears the screen
        {
            memset(std::data(mVram), 0, std::size(mVram));
            mRedraw = true;
        }
        break;

        case 0x00EE: // 0x00EE: Returns from subroutine
            mPC = mStack[--mSP];
            break;

        case 0x00FF: // 0x00FF: Enable 128x64 hi res graphics mode
            ChangeGraphicsMode(GraphicsMode::e128x64);
            break;

        case 0x00FE: // 0x00FE: Disable 128x64 hi res graphics mode
            ChangeGraphicsMode(GraphicsMode::e64x32);
            break;

        case 0x00FB: // 0x00FB: Scroll the display right by 4 pixels
            // TODO
            break;

        case 0x00FC: // 0x00FC: Scroll the display left by 4 pixels
            // TODO
            break;

        case 0x00FD: // 0x00FD Exit the Chip8/SuperChip interpreter
            Init();
            mGameFile = "";
            return;
        }

        // 0x00CN Scroll the display down by 0 to 15 pixels
        if ((mOpcode & 0x00F0) == 0x00C0)
        {
            // TODO
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
            if ((GetVY() + GetVX()) > 0xFF)
                SetVF(1);
            else
                SetVF(0);
            SetVX(GetVX() + GetVY());
        }
        break;

        case 0x0005: // 0x8XY5 VY is subtracted from VX. VF is set to 0 if borrow
        {
            if (GetVX() > GetVY())
                SetVF(1);
            else
                SetVF(0);
            SetVX(GetVX() - GetVY());
        }
        break;

        case 0x0006: // 0x8XY6 Stores the least significant bit of VX in VF and then shifts VX to the right by 1.
        {
            if (mUseVYForShiftQuirk)
                SetVX(GetVY());

            SetVF(GetVX() & 0x1);
            SetVX(GetVX() >> 1);
        }
        break;

        case 0x0007: // 0x8XY7 Sets VX to VY minus VX. VF is set to 0 when there's a borrow, and 1 when there isn't.
        {
            if (GetVY() > GetVX())
                SetVF(1);
            else
                SetVF(0);
            SetVX(GetVY() - GetVX());
        }
        break;

        case 0x000E: // 0x8XYE Stores the most significant bit of VX in VF and then shifts VX to the left by 1.
        {
            if (mUseVYForShiftQuirk)
                SetVX(GetVY());

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
        if (mUseBXNNQuirk)
            mPC = GetAddress() + GetVX();
        else
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
        if (mGraphicsMode == GraphicsMode::e128x64 && (mOpcode & 0x000F) == 0)
        {
            // TODO: 0xDXY0: Draw 16x16 sprite
        }
        else
        {
            const uint16_t xPos = GetVX();
            const uint16_t yPos = GetVY();
            const uint16_t numRows = mOpcode & 0x000F;
            constexpr uint16_t numCols = 8;
            SetVF(0);

            for (uint16_t row = 0; row < numRows; ++row)
            {
                const uint8_t pixel = mMemory[mIndexReg + row];
                for (uint16_t col = 0; col < numCols; ++col)
                {
                    if (pixel & (0x80 >> col))
                    {
                        const uint16_t index = (xPos + col) + ((yPos + row) * GetScreenWidth());
                        // TODO: The index can access out of bounds. Should figure out why.
                        if (index >= std::size(mVram))
                            continue;

                        if (mVram[index] == 1)
                            SetVF(1);

                        mVram[index] ^= 1;
                    }
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
            mIndexReg += GetVX();
            break;

        case 0x0029: // 0xFX29 Sets I to the location of the sprite for the character in VX. Characters 0-F (in hexadecimal) are represented by a 4x5 font.
            mIndexReg = GetVX() * 5;
            break;

        case 0x0030: // 0xFX30 Set I to a large hex character based on the value of VX.
            mIndexReg = GetVX() * 10 + 80; // 80 is the start of the hi res font
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
            memcpy(std::data(mMemory) + mIndexReg, std::data(mV), ((mOpcode & 0x0F00) >> 8) + 1);

            // On the original interpreter, when the operation is done, I = I + X + 1.
            // Newer implementations do not increment the I
            if (mUseIndexIncrementAfterStoreLoadQuirk)
                mIndexReg += ((mOpcode & 0x0F00) >> 8) + 1;
        }
        break;

        case 0x0065: // 0xFX65 Fills V0 to VX (including VX) with values from memory starting at address I
        {
            memcpy(std::data(mV), std::data(mMemory) + mIndexReg, ((mOpcode & 0x0F00) >> 8) + 1);

            // On the original interpreter, when the operation is done, I = I + X + 1.
            // Newer implementations do not increment the I
            if (mUseIndexIncrementAfterStoreLoadQuirk)
                mIndexReg += ((mOpcode & 0x0F00) >> 8) + 1;
        }
        break;

        case 0x0075: // 0xFX75 Save V0-VX to flag registers
            // TODO
            break;

        case 0x0085: // 0xFX85 Restore V0-VX from flag registers
            // TODO
            break;
        }
    }
    break;
    }

    if (mDelayTimer > 0)
        --mDelayTimer;

    if (mSoundTimer > 0)
    {
        if (--mSoundTimer == 0)
        {
            LOG_INFO("BEEP!");
        }
    }
}

void Chip8::LoadGame(const std::filesystem::path& game)
{
    if (game.extension() != std::filesystem::path(".c8") &&
        game.extension() != std::filesystem::path(".ch8"))
    {
        const std::string error = game.generic_string() + " isn't a .c8 or .ch8 file";
        LOG_ERROR(error.c_str());
        throw std::invalid_argument(error);
    }

    std::ifstream f(game, std::ios_base::in | std::ios_base::binary);
    if (!f)
    {
        const std::string error = "Unable to open game: " + game.generic_string();
        LOG_ERROR(error.c_str());
        throw std::invalid_argument(error);
    }

    const size_t fileSize = std::filesystem::file_size(game);
    if (fileSize > std::size(mMemory) - 0x200)
    {
        const std::string error = "Rom " + game.string() + " is too large to fit in memory";
        LOG_ERROR(error.c_str());
        throw std::invalid_argument(error);
    }

    Init();

    f.read((char*)std::data(mMemory) + 0x200, fileSize);

    mGameFile = game;
}

std::vector<uint32_t> Chip8::GetVramImage()
{
    std::vector<uint32_t> image(GetScreenWidth() * GetScreenHeight());
    for (uint32_t i = 0; i < std::size(mVram); ++i)
        image[i] = (mVram[i] == 1) ? mDrawnColor : mUndrawnColor;

    return image;
}

void Chip8::SaveState(const uint32_t slot)
{
    std::filesystem::create_directories("Resources/SaveStates");

    if (std::empty(mGameFile))
        return;

    const std::string filepath = StringUtils::Format("Resources/SaveStates/%s_%i.c8state",
        mGameFile.stem().string().c_str(), slot);
    std::ofstream file(filepath, std::ios::out | std::ios::binary);
    if (!file)
    {
        LOG_ERROR("Failed to write save state for {}", filepath);
        return;
    }

    file.write((const char*)&mOpcode, sizeof(uint16_t));
    file.write((const char*)&mIndexReg, sizeof(uint16_t));
    file.write((const char*)&mPC, sizeof(uint16_t));
    file.write((const char*)&mSP, sizeof(uint16_t));

    file.write((const char*)&mDelayTimer, sizeof(uint8_t));
    file.write((const char*)&mSoundTimer, sizeof(uint8_t));

    file.write((const char*)&mRedraw, sizeof(bool));

    file.write((const char*)&mGraphicsMode, sizeof(uint8_t));

    file.write((const char*)std::data(mMemory), sizeof(uint8_t) * std::size(mMemory));
    file.write((const char*)std::data(mVram), sizeof(uint8_t) * std::size(mVram));
    file.write((const char*)std::data(mV), sizeof(uint8_t) * std::size(mV));

    file.write((const char*)std::data(mStack), sizeof(uint16_t) * std::size(mStack));

    LOG_INFO("Saved State: {}", filepath);
}

void Chip8::LoadState(const uint32_t slot)
{
    if (std::empty(mGameFile))
        return;

    const std::string filepath = StringUtils::Format("Resources/SaveStates/%s_%i.c8state",
        mGameFile.stem().string().c_str(), slot);
    std::ifstream file(filepath, std::ios::in | std::ios::binary);
    if (!file)
    {
        LOG_ERROR("Failed to load save state for {}", filepath);
        return;
    }

    file.read((char*)&mOpcode, sizeof(uint16_t));
    file.read((char*)&mIndexReg, sizeof(uint16_t));
    file.read((char*)&mPC, sizeof(uint16_t));
    file.read((char*)&mSP, sizeof(uint16_t));

    file.read((char*)&mDelayTimer, sizeof(uint8_t));
    file.read((char*)&mSoundTimer, sizeof(uint8_t));

    file.read((char*)&mRedraw, sizeof(bool));

    file.read((char*)&mGraphicsMode, sizeof(uint8_t));

    mVram.resize(GetScreenWidth() * GetScreenHeight());

    file.read((char*)std::data(mMemory), sizeof(uint8_t) * std::size(mMemory));
    file.read((char*)std::data(mVram), sizeof(uint8_t) * std::size(mVram));
    file.read((char*)std::data(mV), sizeof(uint8_t) * std::size(mV));

    file.read((char*)std::data(mStack), sizeof(uint16_t) * std::size(mStack));

    LOG_INFO("Loaded State: {}", filepath);
}

uint16_t Chip8::GetScreenWidth() const
{
    switch (mGraphicsMode)
    {
    case GraphicsMode::e64x32:
        return 64;

    case GraphicsMode::e128x64:
        return 128;
    }

    return 64;
}

uint16_t Chip8::GetScreenHeight() const
{
    switch (mGraphicsMode)
    {
    case GraphicsMode::e64x32:
        return 32;

    case GraphicsMode::e128x64:
        return 64;
    }

    return 32;
}

void Chip8::ChangeGraphicsMode(const GraphicsMode mode)
{
    switch (mGraphicsMode)
    {
    case GraphicsMode::e64x32:
    {
        if (mode == GraphicsMode::e64x32)
            return;

        mVram.resize(64 * 32);
    }
    break;

    case GraphicsMode::e128x64:
    {
        if (mode == GraphicsMode::e128x64)
            return;

        mVram.resize(128 * 64);
    }
    break;
    }
}
