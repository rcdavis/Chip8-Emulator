
#ifndef _CHIP8_H_
#define _CHIP8_H_

#include <array>
#include <filesystem>
#include <functional>

class Chip8
{
public:
	static constexpr unsigned short SCREEN_WIDTH = 64;
	static constexpr unsigned short SCREEN_HEIGHT = 32;
	static constexpr unsigned short VRAM_SIZE = SCREEN_WIDTH * SCREEN_HEIGHT;

	Chip8();

	void SetInputFunc(std::function<void(std::array<unsigned char, 16>&)> func) { mInputFunc = func; }
	void SetRenderFunc(std::function<void(const std::array<unsigned char, VRAM_SIZE>&)> func) { mRenderFunc = func; }

	void LoadGame(std::filesystem::path game);

	void EmulateCycle();

	unsigned int GetFrameRate() const;
	void SetFrameRate(unsigned int fps);

	unsigned char* GetVram();
	unsigned char* GetKeys();

	bool mRedraw;

private:
	void Init();

	unsigned short GetOpcode() const;

	std::function<void(const std::array<unsigned char, VRAM_SIZE>&)> mRenderFunc;
	std::function<void(std::array<unsigned char, 16>&)> mInputFunc;

	std::array<unsigned char, 4096> mMemory;
	std::array<unsigned char, 16> mV;
	std::array<unsigned char, VRAM_SIZE> mVram;

	unsigned short mOpcode;
	unsigned short mIndexReg;
	unsigned short mPC;

	std::array<unsigned short, 16> mStack;
	unsigned short mSP;

	std::array<unsigned char, 16> mKeys;

	unsigned char mDelayTimer;
	unsigned char mSoundTimer;

	unsigned int mFrameRate;
};

#endif
