#pragma once

#include "Types.h"

#include <array>
#include <vector>
#include <string>
#include <filesystem>
#include <functional>

class Chip8
{
public:
    static constexpr uint32_t MEM_SIZE = 64 * 1024;

    using UpdateInputFunc = std::function<void(std::array<uint8_t, 16>&)>;
    using RenderFunc = std::function<void(const std::vector<uint32_t>&)>;
    using OpcodeLogFunc = std::function<void(const std::string&)>;

    enum class GraphicsMode
    {
        e64x32,
        e128x64
    };

public:
    Chip8();

    void LoadGame(const std::filesystem::path& game);
    std::string Disassemble();

    void Emulate();

    void SaveState(const uint32_t slot = 0);
    void LoadState(const uint32_t slot = 0);

    uint32_t GetFrameRate() const { return mFrameRate; }
    void SetFrameRate(uint32_t fps) { mFrameRate = fps; }

    void SetUpdateInputFunc(const UpdateInputFunc& func) { mUpdateInputFunc = func; }
    void SetRenderFunc(const RenderFunc& func) { mRenderFunc = func; }
    void SetOpcodeLogFunc(const OpcodeLogFunc& func) { mOpcodeLogFunc = func; }

    uint16_t GetScreenWidth() const;
    uint16_t GetScreenHeight() const;

    GraphicsMode GetGraphicsMode() const { return mGraphicsMode; }
    void SetGraphicsMode(const GraphicsMode mode) { mGraphicsMode = mode; }
    void ChangeGraphicsMode(const GraphicsMode mode);

    const std::filesystem::path& GetGameFile() const { return mGameFile; }
    std::vector<uint8_t>& GetVram() { return mVram; }
    std::array<uint8_t, MEM_SIZE>& GetMemory() { return mMemory; }
    std::array<uint8_t, 16> GetVReg() const { return mV; }
    std::array<uint8_t, 16>& GetKeys() { return mKeys; }
    std::array<uint16_t, 16> GetStack() const { return mStack; }

    std::vector<uint32_t> GetVramImage();

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

    bool GetUseVYForShiftQuirk() const { return mUseVYForShiftQuirk; }
    void SetUseVYForShiftQuirk(const bool b) { mUseVYForShiftQuirk = b; }

    bool GetUseBXNNQuirk() const { return mUseBXNNQuirk; }
    void SetUseBXNNQuirk(const bool b) { mUseBXNNQuirk = b; }

    bool GetUseIndexIncrementAfterStoreLoadQuirk() const { return mUseIndexIncrementAfterStoreLoadQuirk; }
    void SetUseIndexIncrementAfterStoreLoadQuirk(const bool b) { mUseIndexIncrementAfterStoreLoadQuirk = b; }

    void CloseGame();

private:
    void Init();

    void EmulateCycle();

    std::string DisassembleOpcode(const uint8_t* const buffer, uint16_t opcode);

private:
    UpdateInputFunc mUpdateInputFunc;
    RenderFunc mRenderFunc;
    OpcodeLogFunc mOpcodeLogFunc;

    std::filesystem::path mGameFile;

    std::array<uint8_t, MEM_SIZE> mMemory;
    std::array<uint8_t, 16> mV;
    std::array<uint8_t, 8> mRpl;
    std::vector<uint8_t> mVram;

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

    bool mUseVYForShiftQuirk = false;
    bool mUseBXNNQuirk = false;
    bool mUseIndexIncrementAfterStoreLoadQuirk = false;

    uint32_t mFrameRate;

    // Colors for monochrome screen
    uint32_t mDrawnColor = 0xFFFFFFFF; // White
    uint32_t mUndrawnColor = 0xFF000000; // Black

    GraphicsMode mGraphicsMode = GraphicsMode::e64x32;
};
