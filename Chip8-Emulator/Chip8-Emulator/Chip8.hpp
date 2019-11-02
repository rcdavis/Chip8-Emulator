
#ifndef _CHIP8_H_
#define _CHIP8_H_

#include <array>

class Chip8
{
public:
	Chip8();

	void Init();

	static constexpr unsigned short SCREEN_WIDTH = 64;
	static constexpr unsigned short SCREEN_HEIGHT = 32;

private:

	std::array<unsigned char, 4096> mMemory;
	std::array<unsigned char, 16> mV;
	std::array<unsigned char, SCREEN_WIDTH * SCREEN_HEIGHT> mGfx;

	unsigned short mIndexReg;
	unsigned short mPC;

	std::array<unsigned short, 16> mStack;
	unsigned short mSP;

	std::array<unsigned char, 16> mKeys;
};

#endif
