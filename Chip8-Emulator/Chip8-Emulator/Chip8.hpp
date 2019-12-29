
#ifndef _CHIP8_H_
#define _CHIP8_H_

#include <array>
#include <filesystem>
#include <functional>
#include <cstdint>

using std::uint8_t;
using std::uint16_t;
using std::uint32_t;

class Chip8
{
public:
	static constexpr uint16_t SCREEN_WIDTH = 64;
	static constexpr uint16_t SCREEN_HEIGHT = 32;
	static constexpr uint16_t VRAM_SIZE = SCREEN_WIDTH * SCREEN_HEIGHT;

	Chip8();

	void SetInputFunc(std::function<void(std::array<uint8_t, 16>&)> func) { mInputFunc = func; }
	void SetRenderFunc(std::function<void(const std::array<uint8_t, VRAM_SIZE>&)> func) { mRenderFunc = func; }

	void LoadGame(std::filesystem::path game);

	void EmulateCycle();

	uint32_t GetFrameRate() const;
	void SetFrameRate(uint32_t fps);

	uint8_t* GetVram();
	uint8_t* GetKeys();

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
