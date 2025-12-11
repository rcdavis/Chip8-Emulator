
#include "Editor.h"
#include "Utils/Log.h"

int main(int argc, char** argv) {
	Log::Init();

	Editor editor;
	if (!editor.Init())
		return -1;

	editor.Run();

	return 0;
}
