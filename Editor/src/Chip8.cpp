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

	auto fileSize = std::filesystem::file_size(gameFile);
	file.read((char*)mMemory.data() + ProgramStartOffset, fileSize);

	mStack.fill(0);
	mKeys.fill(0);
	mV.fill(0);
	mVram.fill(0);

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

	if (mShouldRedraw) {
		if (mRedrawFunc)
			mRedrawFunc();

		mShouldRedraw = false;
	}
}
