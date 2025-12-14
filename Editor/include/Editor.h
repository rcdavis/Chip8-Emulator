#pragma once

#include "Chip8.h"

struct GLFWwindow;

class Editor {
public:
	Editor();
	~Editor();

	bool Init(int argc, char** argv);

	void Run();

private:
	Chip8 mChip8;

	GLFWwindow* mWindow = nullptr;
};
