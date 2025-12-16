#include "Chip8.h"

#include <fstream>
#include "Utils/Log.h"

bool Chip8::LoadGame(const std::filesystem::path& gameFile) {
	if (!std::filesystem::exists(gameFile)) {
		LOG_ERROR("Failed to find Chip8 game {0}", gameFile.c_str());
		return false;
	}

	std::ifstream file(gameFile, std::ios::binary);
	if (!file) {
		LOG_ERROR("Failed to load Chip8 game {0}", gameFile.c_str());
		return false;
	}

	constexpr uint8_t fontset[80] = {
		0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
		0x20, 0x60, 0x20, 0x20, 0x70, // 1
		0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
		0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
		0x90, 0x90, 0xF0, 0x10, 0x10, // 4
		0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
		0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
		0xF0, 0x10, 0x20, 0x40, 0x40, // 7
		0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
		0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
		0xF0, 0x90, 0xF0, 0x90, 0x90, // A
		0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
		0xF0, 0x80, 0x80, 0x80, 0xF0, // C
		0xE0, 0x90, 0x90, 0x90, 0xE0, // D
		0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
		0xF0, 0x80, 0xF0, 0x80, 0x80  // F
	};

	memcpy(mMemory.data(), fontset, sizeof(fontset));

	const auto fileSize = std::filesystem::file_size(gameFile);
	file.read((char*)std::data(mMemory) + ProgramStartOffset, fileSize);

	memset(std::data(mStack), 0, std::size(mStack) * sizeof(uint16_t));
	memset(std::data(mKeys), 0, std::size(mKeys));
	memset(std::data(mV), 0, std::size(mV));
	memset(std::data(mVram), 0, std::size(mVram));

	mOpcode = 0;
	mIndexRegister = 0;
	mStackPointer = 0;
	mDelayTimer = 0;
	mSoundTimer = 0;
	mShouldRedraw = false;
	mProgramCounter = ProgramStartOffset;

	return true;
}

void Chip8::EmulateCycle() {
	mOpcode = mMemory[mProgramCounter] << 8 | mMemory[mProgramCounter + 1];

	switch (mOpcode & 0xF000) {
	case 0x0000:
	{
		switch (mOpcode) {
		case 0x00E0: // 0x00E0: Clears the screen
			memset(std::data(mVram), 0, std::size(mVram));
			mShouldRedraw = true;
			mProgramCounter += 2;
			break;

		case 0x00EE: // 0x00EE: Returns from subroutine
			mProgramCounter = mStack[--mStackPointer];
			mProgramCounter += 2;
			break;
		}
	}
	break;

	case 0x1000: // 0x1NNN: Jumps to address NNN
		mProgramCounter = mOpcode & 0x0FFF;
		break;

	case 0x2000: // 0x2NNN: Calls subroutine at NNN
		mStack[mStackPointer++] = mProgramCounter;
		mProgramCounter = mOpcode & 0x0FFF;
		break;

	case 0x3000: // 0x3XNN: Skips the next instruction if VX equals NN
		if (mV[(mOpcode & 0x0F00) >> 8] == (mOpcode & 0x00FF))
			mProgramCounter += 4;
		else
			mProgramCounter += 2;
		break;

	case 0x4000: // 0x4XNN: Skips the next instruction if VX doesn't equal NN
		if (mV[(mOpcode & 0x0F00) >> 8] != (mOpcode & 0x00FF))
			mProgramCounter += 4;
		else
			mProgramCounter += 2;
		break;

	case 0x5000: // 0x5XY0: Skips the next instruction if VX equals VY
		if (mV[(mOpcode & 0x0F00) >> 8] == mV[(mOpcode & 0x00F0) >> 4])
			mProgramCounter += 4;
		else
			mProgramCounter += 2;
		break;

	case 0x6000: // 0x6XNN: Sets VX to NN
		mV[(mOpcode & 0x0F00) >> 8] = (mOpcode & 0x00FF);
		mProgramCounter += 2;
		break;

	case 0x7000: // 0x7XNN: Adds NN to VX
		mV[(mOpcode & 0x0F00) >> 8] += (mOpcode & 0x00FF);
		mProgramCounter += 2;
		break;

	case 0x8000:
	{
		switch (mOpcode & 0x000F) {
		case 0x0000: // 0x8XY0: Sets VX to the value of VY
			mV[(mOpcode & 0x0F00) >> 8] = mV[(mOpcode & 0x00F0) >> 4];
			mProgramCounter += 2;
			break;

		case 0x0001: // 0x8XY1: Sets VX to "VX OR VY"
			mV[(mOpcode & 0x0F00) >> 8] |= mV[(mOpcode & 0x00F0) >> 4];
			mProgramCounter += 2;
			break;

		case 0x0002: // 0x8XY2: Sets VX to "VX AND VY"
			mV[(mOpcode & 0x0F00) >> 8] &= mV[(mOpcode & 0x00F0) >> 4];
			mProgramCounter += 2;
			break;

		case 0x0003: // 0x8XY3: Sets VX to "VX XOR VY"
			mV[(mOpcode & 0x0F00) >> 8] ^= mV[(mOpcode & 0x00F0) >> 4];
			mProgramCounter += 2;
			break;

		case 0x0004: // 0x8XY4: Adds VY to VX. VF is set to 1 when there's a carry, and to 0 when there isn't
		{
			const uint8_t x = (mOpcode & 0x0F00) >> 8;
			const uint8_t y = (mOpcode & 0x00F0) >> 4;
			const uint16_t sum = mV[x] + mV[y];

			mV[0xF] = (sum > 0xFF) ? 1 : 0;
			mV[x] += mV[y];
			mProgramCounter += 2;
		}
		break;

		case 0x0005: // 0x8XY5: VY is subtracted from VX. VF is set to 0 when there's a borrow, and to 1 when there isn't
		{
			const uint8_t x = (mOpcode & 0x0F00) >> 8;
			const uint8_t y = (mOpcode & 0x00F0) >> 4;
			const int16_t diff = mV[x] - mV[y];

			mV[0xF] = (diff < 0x0) ? 0 : 1;
			mV[x] -= mV[y];
			mProgramCounter += 2;
		}
		break;

		case 0x0006: // 0x8XY6: Shifts VX right by one. VF is set to the value of the least significant bit of VX before the shift.
			mV[0xF] = mV[(mOpcode & 0x0F00) >> 8] & 0x1;
			mV[(mOpcode & 0x0F00) >> 8] >>= 1;
			mProgramCounter += 2;
			break;

		case 0x0007: // 0x8XY7: Sets VX to VY minus VX. VF is set to 0 when there's a borrow, and 1 when there isn't
		{
			const uint8_t x = (mOpcode & 0x0F00) >> 8;
			const uint8_t y = (mOpcode & 0x00F0) >> 4;
			const int16_t diff = mV[y] - mV[x];

			mV[0xF] = (diff < 0x0) ? 0 : 1;
			mV[x] = mV[y] - mV[x];
			mProgramCounter += 2;
		}
		break;

		case 0x000E: // 0x8XYE: Shifts VX left by one. VF is set to the value of the least significant bit of VX before the shift.
			mV[0xF] = mV[(mOpcode & 0x0F00) >> 8] >> 7;
			mV[(mOpcode & 0x0F00) >> 8] <<= 1;
			mProgramCounter += 2;
			break;
		}
	}
	break;

	case 0x9000: // 0x9XY0: Skips the next instruction if VX doesn't equal VY
		if (mV[(mOpcode & 0x0F00) >> 8] != mV[(mOpcode & 0x00F0) >> 4])
			mProgramCounter += 4;
		else
			mProgramCounter += 2;
		break;

	case 0xA000: // 0xANNN: Sets I to the address NNN
		mIndexRegister = mOpcode & 0x0FFF;
		mProgramCounter += 2;
		break;

	case 0xB000: // 0xBNNN: Jumps to the address NNN plus V0
		mProgramCounter = (mOpcode & 0x0FFF) + mV[0];
		break;

	case 0xC000: // 0xCXNN: Sets VX to a random number and NN
		mV[(mOpcode & 0x0F00) >> 8] = (rand() % 0xFF) & (mOpcode & 0x00FF);
		mProgramCounter += 2;

	case 0xD000: // DXYN: Draws a sprite at coordinate (VX, VY) that has a width of 8 and height of N pixels.
	{
		const uint16_t x = mV[(mOpcode & 0x0F00) >> 8];
		const uint16_t y = mV[(mOpcode & 0x00F0) >> 4];
		const uint16_t height = (mOpcode & 0x000F);
		uint16_t pixel = 0;

		mV[0xF] = 0;
		for (uint16_t row = 0; row < height; ++row) {
			pixel = mMemory[mIndexRegister + row];
			for (uint16_t col = 0; col < 8; ++col) {
				if (pixel & (0x80 >> col) != 0) {
					if (mVram[(x + col + ((y + row) * Width))] == 1)
						mV[0xF] = 1;

					mVram[x + col + ((y + row) * Width)] ^= 1;
				}
			}
		}

		mShouldRedraw = true;
		mProgramCounter += 2;
	}
	break;

	case 0xE000:
	{
		switch (mOpcode & 0x00FF) {
		case 0x009E: // 0xEX9E: Skips the next instruction if the key stored in VX is pressed
			if (mKeys[mV[(mOpcode & 0x0F00) >> 8]] != 0)
				mProgramCounter += 4;
			else
				mProgramCounter += 2;
			break;

		case 0x00A1: // 0xEXA1: Skips the next instruction if the key stored in VX isn't pressed
			if (mKeys[mV[(mOpcode & 0x0F00) >> 8]] == 0)
				mProgramCounter += 4;
			else
				mProgramCounter += 2;
			break;
		}
	}
	break;

	case 0xF000:
	{
		switch (mOpcode & 0x00FF) {
		case 0x0007: // 0xFX07: Sets VX to the value of the delay timer
			mV[(mOpcode & 0x0F00) >> 8] = mDelayTimer;
			mProgramCounter += 2;
			break;

		case 0x000A: // FX0A: A key press is awaited, and then stored in VX
		{
			bool isKeyPressed = false;
			for (uint8_t i = 0; i < mKeys.size(); ++i) {
				if (mKeys[i] != 0) {
					mV[(mOpcode & 0x0F00) >> 8] = i;
					isKeyPressed = true;
				}
			}

			if (!isKeyPressed)
				return;

			mProgramCounter += 2;
		}
		break;

		case 0x0015: // FX15: Sets the delay timer to VX
			mDelayTimer = mV[(mOpcode & 0x0F00) >> 8];
			mProgramCounter += 2;
			break;

		case 0x0018: // FX18: Sets the sound timer to VX
			mSoundTimer = mV[(mOpcode & 0x0F00) >> 8];
			mProgramCounter += 2;
			break;

		case 0x001E: // 0xFX1E: Adds VX to I
			if (mIndexRegister + mV[(mOpcode & 0x0F00) >> 8] > 0x0FFF)
				mV[0xF] = 1;
			else
				mV[0xF] = 0;
			mIndexRegister += mV[(mOpcode & 0x0F00) >> 8];
			mProgramCounter += 2;
			break;
		}

		case 0x0029: // 0xFX29: Sets I to the location of the sprite for the character in VX. Characters 0-F (in hexadecimal) are represented by a 4x5 font
			mIndexRegister = mV[(mOpcode & 0x0F00) >> 8] * 0x5;
			mProgramCounter += 2;
			break;

		case 0x0033: // 0xFX33: Stores the binary-coded decimal representation of VX at the addresses I, I plus 1, and I plus 2;
			mMemory[mIndexRegister] = mV[(mOpcode & 0x0F00) >> 8] / 100;
			mMemory[mIndexRegister + 1] = (mV[(mOpcode & 0x0F00) >> 8] / 10) % 10;
			mMemory[mIndexRegister + 2] = (mV[(mOpcode & 0x0F00) >> 8] % 100) % 10;
			mProgramCounter += 2;
			break;

		case 0x0055: // 0xFX55: Stores V0 to VX in memory starting at address I
		{
			const uint8_t x = (mOpcode & 0x0F00) >> 8;
			for (uint8_t i = 0; i <= x; ++i)
				mMemory[mIndexRegister + i] = mV[i];

			mIndexRegister += x + 1;
			mProgramCounter += 2;
		}
		break;

		case 0x0065: // 0xFX65: Fills V0 to VX with values from memory starting at address I
		{
			const uint8_t x = (mOpcode & 0x0F00) >> 8;
			for (uint8_t i = 0; i <= x; ++i)
				mV[i] = mMemory[mIndexRegister + i];

			mIndexRegister += x + 1;
			mProgramCounter += 2;
		}
		break;
	}
	break;
	}

	if (mDelayTimer > 0)
		--mDelayTimer;

	if (mSoundTimer > 0) {
		if (--mSoundTimer == 0) {
			// TODO: Play Beep Sound
		}
	}
}
