
#include "Editor.h"

int main(int argc, char** argv) {
	Editor editor;
	if (!editor.Init())
		return -1;

	editor.Run();

	return 0;
}
