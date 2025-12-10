#pragma once

struct GLFWwindow;

class Editor {
public:
	Editor();
	~Editor();

	bool Init();

	void Run();

private:
	GLFWwindow* mWindow = nullptr;
};
