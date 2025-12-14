#pragma once

#include <array>
#include <filesystem>
#include <functional>

class Chip8 {
private:
	static constexpr unsigned int Width = 64;
	static constexpr unsigned int Height = 32;
	static constexpr unsigned int VramSize = Width * Height;

	static constexpr unsigned int ProgramStartOffset = 0x200;

public:
	Chip8() = default;

	bool LoadGame(const std::filesystem::path& gameFile);

	void EmulateCycle();

	void SetRedrawFunc(std::function<void()>& func) { mRedrawFunc = func; }

private:
	std::function<void()> mRedrawFunc;

	unsigned short mOpcode = 0;
	unsigned short mProgramCounter = 0;
	unsigned short mIndexRegister = 0;

	std::array<unsigned short, 16> mStack{};

	std::array<unsigned char, 16> mKeys{};

	std::array<unsigned char, 4096> mMemory{};
	std::array<unsigned char, 16> mV{};

	std::array<unsigned char, VramSize> mVram{};

	bool mShouldRedraw = false;

	unsigned char mStackPointer = 0;

	unsigned char mDelayTimer = 0;
	unsigned char mSoundTimer = 0;
};
