#pragma once

#include "Types.h"

#include <array>
#include <filesystem>
#include <functional>

class Chip8
{
public:
    // TODO: Super Chip is 128x64
    static constexpr uint16_t SCREEN_WIDTH = 64;
    static constexpr uint16_t SCREEN_HEIGHT = 32;
    static constexpr uint16_t VRAM_SIZE = SCREEN_WIDTH * SCREEN_HEIGHT;

    using UpdateInputFunc = std::function<void(std::array<uint8_t, 16>&)>;
    using RenderFunc = std::function<void(std::array<uint32_t, VRAM_SIZE>&)>;

public:
    Chip8();

    void LoadGame(const std::filesystem::path& game);

    void Emulate();

    void SaveState(const uint32_t slot = 0);
    void LoadState(const uint32_t slot = 0);

    uint32_t GetFrameRate() const { return mFrameRate; }
    void SetFrameRate(uint32_t fps) { mFrameRate = fps; }

    void SetUpdateInputFunc(UpdateInputFunc func) { mUpdateInputFunc = func; }
    void SetRenderFunc(RenderFunc func) { mRenderFunc = func; }

    const std::filesystem::path& GetGameFile() const { return mGameFile; }
    std::array<uint8_t, VRAM_SIZE>& GetVram() { return mVram; }
    std::array<uint8_t, 4096>& GetMemory() { return mMemory; }
    std::array<uint8_t, 16> GetVReg() const { return mV; }
    std::array<uint8_t, 16>& GetKeys() { return mKeys; }
    std::array<uint16_t, 16> GetStack() const { return mStack; }

    std::array<uint32_t, VRAM_SIZE> GetVramImage();

    uint16_t GetOpcode() const { return (mMemory[mPC] << 8) | mMemory[mPC + 1]; }
    uint16_t GetIndexReg() const { return mIndexReg; }
    uint16_t GetProgramCounter() const { return mPC; }
    uint16_t GetStackPointer() const { return mSP; }

    uint8_t GetDelayTimer() const { return mDelayTimer; }
    uint8_t GetSoundTimer() const { return mSoundTimer; }

    uint8_t GetVX() const { return mV[(mOpcode & 0x0F00) >> 8]; }
    void SetVX(const uint8_t v) { mV[(mOpcode & 0x0F00) >> 8] = v; }

    uint8_t GetVY() const { return mV[(mOpcode & 0x00F0) >> 4]; }
    void SetVY(const uint8_t v) { mV[(mOpcode & 0x00F0) >> 4] = v; }

    uint8_t GetVF() const { return mV[0xF]; }
    void SetVF(const uint8_t v) { mV[0xF] = v; }

    uint8_t GetNN() const { return mOpcode & 0x00FF; }
    uint16_t GetAddress() const { return mOpcode & 0x0FFF; }

    uint8_t GetEmuSpeedModifier() const { return mEmuSpeedModifier; }
    void SetEmuSpeedModifier(const uint8_t modifier) { mEmuSpeedModifier = modifier; }

    uint32_t GetDrawnColor() const { return mDrawnColor; }
    void SetDrawnColor(const uint32_t color) { mDrawnColor = color; }

    uint32_t GetUndrawnColor() const { return mUndrawnColor; }
    void SetUndrawnColor(const uint32_t color) { mUndrawnColor = color; }

private:
    void Init();

    void EmulateCycle();

private:
    UpdateInputFunc mUpdateInputFunc;
    RenderFunc mRenderFunc;

    std::filesystem::path mGameFile;

    std::array<uint8_t, 4096> mMemory;
    std::array<uint8_t, 16> mV;
    std::array<uint8_t, VRAM_SIZE> mVram;

    uint16_t mOpcode;
    uint16_t mIndexReg;
    uint16_t mPC;
    uint16_t mSP;

    std::array<uint16_t, 16> mStack;
    std::array<uint8_t, 16> mKeys;

    uint8_t mDelayTimer;
    uint8_t mSoundTimer;

    uint8_t mEmuSpeedModifier = 1;

    bool mRedraw;

    uint32_t mFrameRate;

    // Colors for monochrome screen
    uint32_t mDrawnColor = 0xFFFFFFFF; // White
    uint32_t mUndrawnColor = 0xFF000000; // Black
};
