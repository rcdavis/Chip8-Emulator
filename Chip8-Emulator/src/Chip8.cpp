
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
	std::fill(std::begin(mMemory), std::end(mMemory), 0);
	std::fill(std::begin(mV), std::end(mV), 0);
	std::fill(std::begin(mVram), std::end(mVram), 0);
	std::fill(std::begin(mStack), std::end(mStack), 0);
	std::fill(std::begin(mKeys), std::end(mKeys), 0);

	mOpcode = 0;

	mIndexReg = 0;
	mSP = 0;
	mPC = 512;

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

	switch (mOpcode & 0xF000)
	{
	case 0x0000:
	{
		switch (mOpcode & 0x000F)
		{
		case 0x0000: // 0x00E0: Clears the screen
		{
			std::fill(std::begin(mVram), std::end(mVram), 0);
			mRedraw = true;
			mPC += 2;
		}
		break;

		case 0x000E: // 0x00EE: Returns from subroutine
		{
			mPC = mStack[--mSP];
			mPC += 2;
		}
		break;
		}
	}
	break;

	case 0x1000: // 0x1NNN Jump to address NNN
	{
		mPC = mOpcode & 0x0FFF;
	}
	break;

	case 0x2000: // 0x2NNN Calls subroutine at NNN
	{
		mStack[mSP++] = mPC;
		mPC = mOpcode & 0x0FFF;
	}
	break;

	case 0x3000: // 0x3XNN Skips next instruction if VX equals NN
	{
		if (mV[(mOpcode & 0x0F00) >> 8] == (mOpcode & 0x00FF))
			mPC += 4;
		else
			mPC += 2;
	}
	break;

	case 0x4000: // 0x4XNN Skips next instruction if VX doesn't equals NN
	{
		if (mV[(mOpcode & 0x0F00) >> 8] != (mOpcode & 0x00FF))
			mPC += 4;
		else
			mPC += 2;
	}
	break;

	case 0x5000: // 0x5XY0 Skips next instruction if VX equals VY
	{
		if (mV[(mOpcode & 0x0F00) >> 8] == mV[(mOpcode & 0x00F0) >> 4])
			mPC += 4;
		else
			mPC += 2;
	}
	break;

	case 0x6000: // 0x6XNN Sets VX to NN
	{
		mV[(mOpcode & 0x0F00) >> 8] = (mOpcode & 0x00FF);
		mPC += 2;
	}
	break;

	case 0x7000: // 0x7XNN Adds NN to VX
	{
		mV[(mOpcode & 0x0F00) >> 8] += (mOpcode & 0x00FF);
		mPC += 2;
	}
	break;

	case 0x8000:
	{
		switch (mOpcode & 0x000F)
		{
		case 0x0000: // 0x8XY0 Set VX to VY
		{
			mV[(mOpcode & 0x0F00) >> 8] = mV[(mOpcode & 0x00F0) >> 4];
			mPC += 2;
		}
		break;

		case 0x0001: // 0x8XY1 Set VX to VX or VY
		{
			mV[(mOpcode & 0x0F00) >> 8] |= mV[(mOpcode & 0x00F0) >> 4];
			mPC += 2;
		}
		break;

		case 0x0002: // 0x8XY2 Set VX to VX and VY
		{
			mV[(mOpcode & 0x0F00) >> 8] &= mV[(mOpcode & 0x00F0) >> 4];
			mPC += 2;
		}
		break;

		case 0x0003: // 0x8XY3 Set VX to VX xor VY
		{
			mV[(mOpcode & 0x0F00) >> 8] ^= mV[(mOpcode & 0x00F0) >> 4];
			mPC += 2;
		}
		break;

		case 0x0004: // 0x8XY4 Adds VY to VX. VF is set when there's a carry
		{
			mV[0xF] = (mV[(mOpcode & 0x0F00) >> 8] + mV[(mOpcode & 0x00F0) >> 4] > 0xFF) ? 1 : 0;
			mV[(mOpcode & 0x0F00) >> 8] += mV[(mOpcode & 0x00F0) >> 4];
			mPC += 2;
			//if (mV[(mOpcode & 0x0F00) >> 8] + mV[(mOpcode & 0x00F0) >> 4] > 255)
		}
		break;

		case 0x0005: // 0x8XY5 VY is subtracted from VX. VF is set to 0 if borrow
		{
			mV[0xF] = (mV[(mOpcode & 0x0F00) >> 8] < mV[(mOpcode & 0x00F0) >> 4]) ? 0 : 1;
			mV[(mOpcode & 0x0F00) >> 8] -= mV[(mOpcode & 0x00F0) >> 4];
			mPC += 2;
		}
		break;

		case 0x0006: // 0x8XY6 Shifts VX right by one. VF set to the value of least significant bit of VX before shift
		{
			mV[0xF] = mV[(mOpcode & 0x0F00) >> 8] & 0x1;
			mV[(mOpcode & 0x0F00) >> 8] >>= 1;
			mPC += 2;
		}
		break;

		case 0x0007: // 0x8XY7 Sets VX to VY minus VX. VF is set to 0 when there's a borrow, and 1 when there isn't.
		{
			mV[0xF] = (mV[(mOpcode & 0x0F00) >> 8] > mV[(mOpcode & 0x00F0) >> 4]) ? 0 : 1;
			mV[(mOpcode & 0x0F00) >> 8] = mV[(mOpcode & 0x00F0) >> 4] - mV[(mOpcode & 0x0F00) >> 8];
			mPC += 2;
		}
		break;

		case 0x000E: // 0x8XYE Shifts VX left by one. VF is set to the value of the most significant bit of VX before the shift.
		{
			mV[0xF] = mV[(mOpcode & 0x0F00) >> 8] & 0x8000;
			mV[(mOpcode & 0x0F00) >> 8] <<= 1;
			mPC += 2;
		}
		break;
		}
	}
	break;

	case 0x9000: // 0x9XY0 Skips the next instruction if VX doesn't equal VY
	{
		if (mV[(mOpcode & 0x0F00) >> 8] != mV[(mOpcode & 0x00F0) >> 4])
			mPC += 4;
		else
			mPC += 2;
	}
	break;

	case 0xA000: // 0xANNN Sets I to the address NNN
	{
		mIndexReg = mOpcode & 0x0FFF;
		mPC += 2;
	}
	break;

	case 0xB000: // 0xBNNN Jumps to the address NNN plus V0
	{
		mPC = (mOpcode & 0x0FFF) + mV[0];
	}
	break;

	case 0xC000: // 0xCXNN Sets VX to the result of bitwise and op on a random number and NN
	{
		mV[(mOpcode & 0x0F00) >> 8] = (rand() & 0xFF) & (mOpcode & 0x00FF);
		mPC += 2;
	}
	break;

	/*
	 * 0xDXYN
	 * Draws a sprite at coordinate (VX, VY) that has a width of 8 pixels and a height of N pixels.
	 * Each row of 8 pixels is read as bit-coded starting from memory location I; I value doesn’t change
	 * after the execution of this instruction. As described above, VF is set to 1 if any screen pixels
	 * are flipped from set to unset when the sprite is drawn, and to 0 if that doesn’t happen
	 */
	case 0xD000:
	{
		const uint16_t posX = mV[(mOpcode & 0x0F00) >> 8];
		const uint16_t posY = mV[(mOpcode & 0x00F0) >> 4];
		const uint16_t width = 8;
		const uint16_t height = mOpcode & 0x000F;
		mV[0xF] = 0;

		for (uint16_t y = 0; y < height; ++y)
		{
			const uint16_t pixel = mMemory[mIndexReg + y];
			for (uint16_t x = 0; x < width; ++x)
			{
				if (pixel & (0x80 >> x))
				{
					const uint16_t index = posX + x + ((posY + y) * SCREEN_WIDTH);
					// TODO: The index can access out of bounds. Should figure out why.
					if (index < 0 || index >= std::size(mVram))
						continue;

					if (mVram[index])
						mV[0xF] = 1;

					mVram[index] ^= 1;
				}
			}
		}

		mRedraw = true;
		mPC += 2;
	}
	break;

	case 0xE000:
	{
		switch (mOpcode & 0x00FF)
		{
		case 0x009E: // 0xEX9E Skips the next instruction if the key stored in VX is pressed.
		{
			if (mKeys[mV[(mOpcode & 0x0F00) >> 8]])
				mPC += 4;
			else
				mPC += 2;
		}
		break;

		case 0x00A1: // 0xEXA1 Skips the next instruction if the key stored in VX is not pressed.
		{
			if (mKeys[mV[(mOpcode & 0x0F00) >> 8]] != 0)
				mPC += 4;
			else
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
		{
			mV[(mOpcode & 0x0F00) >> 8] = mDelayTimer;
			mPC += 2;
		}
		break;

		case 0x000A: // 0xFX0A Key press is awaited and stored in VX
		{
			bool keyPressed = false;

			for (uint8_t i = 0; i < std::size(mKeys); ++i)
			{
				if (mKeys[i])
				{
					mV[(mOpcode & 0x0F00) >> 8] = i;
					keyPressed = true;
				}
			}

			if (!keyPressed)
				return;

			mPC += 2;
		}
		break;

		case 0x0015: // 0xFX15 Sets delay timer to VX.
		{
			mDelayTimer = mV[(mOpcode & 0x0F00) >> 8];
			mPC += 2;
		}
		break;

		case 0x0018: // 0xFX18 Sets sound timer to VX.
		{
			mSoundTimer = mV[(mOpcode & 0x0F00) >> 8];
			mPC += 2;
		}
		break;

		case 0x001E: // 0xFX1E Adds VX to I
		{
			mV[0xF] = (mIndexReg + mV[(mOpcode & 0x0F00) >> 8] > 0x0FFF) ? 1 : 0;
			mIndexReg += mV[(mOpcode & 0x0F00) >> 8];
			mPC += 2;
		}
		break;

		case 0x0029: // 0xFX29 Sets I to the location of the sprite for the character in VX. Characters 0-F (in hexadecimal) are represented by a 4x5 font.
		{
			mIndexReg = mV[(mOpcode & 0x0F00) >> 8] * 5;
			mPC += 2;
		}
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
			mMemory[mIndexReg] = mV[(mOpcode & 0x0F00) >> 8] / 100;
			mMemory[mIndexReg + 1] = (mV[(mOpcode & 0x0F00) >> 8] / 10) % 10;
			mMemory[mIndexReg + 2] = (mV[(mOpcode & 0x0F00) >> 8] % 100) % 10;
			mPC += 2;
		}
		break;

		case 0x0055: // 0xFX55 Stores V0 to VX (including VX) in memory starting at address I
		{
			memcpy(std::data(mMemory) + mIndexReg, std::data(mV), (mOpcode & 0x0F00) >> 8);

			// On the original interpreter, when the operation is done, I = I + X + 1.
			mIndexReg += ((mOpcode & 0x0F00) >> 8) + 1;
			mPC += 2;
		}
		break;

		case 0x0065: // 0xFX65 Fills V0 to VX (including VX) with values from memory starting at address I
		{
			memcpy(std::data(mV), std::data(mMemory) + mIndexReg, (mOpcode & 0x0F00) >> 8);

			// On the original interpreter, when the operation is done, I = I + X + 1.
			mIndexReg += ((mOpcode & 0x0F00) >> 8) + 1;
			mPC += 2;
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

	if (game.extension() != std::filesystem::path(".c8"))
	{
		const std::string error = game.generic_string() + " isn't a .c8 file";
		throw std::invalid_argument(error);
	}

	std::ifstream f(game, std::ios_base::in | std::ios_base::binary);
	if (!f.is_open())
	{
		const std::string error = "Unable to open game: " + game.generic_string();
		throw std::invalid_argument(error);
	}

	const size_t fileSize = std::filesystem::file_size(game);

	f.read((char*)std::data(mMemory) + 512, fileSize);

	f.close();
}

uint16_t Chip8::GetOpcode() const
{
	return (mMemory[mPC] << 8) | mMemory[mPC + 1];
}

uint32_t Chip8::GetFrameRate() const
{
	return mFrameRate;
}

void Chip8::SetFrameRate(uint32_t fps)
{
	mFrameRate = fps;
}

uint8_t* Chip8::GetVram()
{
	return std::data(mVram);
}

uint8_t* Chip8::GetKeys()
{
	return std::data(mKeys);
}
