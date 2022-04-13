
#ifndef _CHIP8_H_
#define _CHIP8_H_

#include "Types.h"

#include <array>
#include <filesystem>
#include <functional>

class Chip8
{
public:
	static constexpr uint16_t SCREEN_WIDTH = 64;
	static constexpr uint16_t SCREEN_HEIGHT = 32;
	static constexpr uint16_t VRAM_SIZE = SCREEN_WIDTH * SCREEN_HEIGHT;

	Chip8();

	void SetInputFunc(std::function<void(std::array<uint8_t, 16>&)> func) { mInputFunc = func; }
	void SetRenderFunc(std::function<void(const std::array<uint8_t, VRAM_SIZE>&)> func) { mRenderFunc = func; }

	void LoadGame(const std::filesystem::path& game);

	void EmulateCycle();

	uint32_t GetFrameRate() const;
	void SetFrameRate(uint32_t fps);

	uint8_t* GetVram();
	uint8_t* GetKeys();

    std::array<uint32_t, VRAM_SIZE> GetVramImage();

    uint8_t GetVX() const { return mV[(mOpcode & 0x0F00) >> 8]; }
    void SetVX(const uint8_t v) { mV[(mOpcode & 0x0F00) >> 8] = v; }

    uint8_t GetVY() const { return mV[(mOpcode & 0x00F0) >> 4]; }
    void SetVY(const uint8_t v) { mV[(mOpcode & 0x00F0) >> 4] = v; }

    uint8_t GetVF() const { return mV[0xF]; }
    void SetVF(const uint8_t v) { mV[0xF] = v; }

    uint8_t GetNN() const { return mOpcode & 0x00FF; }
    uint16_t GetAddress() const { return mOpcode & 0x0FFF; }

	bool mRedraw;

private:
	void Init();

	uint16_t GetOpcode() const;

	std::function<void(const std::array<uint8_t, VRAM_SIZE>&)> mRenderFunc;
	std::function<void(std::array<uint8_t, 16>&)> mInputFunc;

	std::array<uint8_t, 4096> mMemory;
	std::array<uint8_t, 16> mV;
	std::array<uint8_t, VRAM_SIZE> mVram;

	uint16_t mOpcode;
	uint16_t mIndexReg;
	uint16_t mPC;

	std::array<uint16_t, 16> mStack;
	uint16_t mSP;

	std::array<uint8_t, 16> mKeys;

	uint8_t mDelayTimer;
	uint8_t mSoundTimer;

	uint32_t mFrameRate;
};

#endif
