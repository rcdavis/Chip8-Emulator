#pragma once

#include "Chip8.h"

#include "OpenGL/GLShader.h"
#include <vector>

struct GLFWwindow;

class Editor {
public:
	Editor();
	~Editor();

	bool Init(int argc, char** argv);

	void Run();

private:
	void UpdateInput();

	void SetupRenderQuad();

private:
	Chip8 mChip8;

	std::vector<uint32_t> mScreenData;

	GLFWwindow* mWindow = nullptr;

	GLShader mShader;

	uint32_t mVao = 0;
	uint32_t mVb = 0;
	uint32_t mIb = 0;
	uint32_t mTexture = 0;
};
