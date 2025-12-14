#pragma once

#include <cstdint>
#include <array>
#include <filesystem>

class Chip8 {
private:
	static constexpr uint32_t Width = 64;
	static constexpr uint32_t Height = 32;
	static constexpr uint32_t VramSize = Width * Height;

	static constexpr uint32_t ProgramStartOffset = 0x200;

public:
	Chip8() = default;

	bool LoadGame(const std::filesystem::path& gameFile);

	void EmulateCycle();

	const std::array<uint8_t, VramSize>& GetVram() const { return mVram; }

private:
	uint16_t mOpcode = 0;
	uint16_t mProgramCounter = 0;
	uint16_t mIndexRegister = 0;

	std::array<uint16_t, 16> mStack{};

public:
	std::array<uint8_t, 16> mKeys{};

private:
	std::array<uint8_t, 4096> mMemory{};
	std::array<uint8_t, 16> mV{};

	std::array<uint8_t, VramSize> mVram{};

	uint8_t mStackPointer = 0;

	uint8_t mDelayTimer = 0;
	uint8_t mSoundTimer = 0;

public:
	bool mShouldRedraw = false;
};
