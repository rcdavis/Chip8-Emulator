
#include "Log.h"
#include "Application.h"

#define GLAD_GL_IMPLEMENTATION
#include <glad/gl.h>

int main()
{
    Log::Init();

    Application app;
    if (!app.Init())
        return -1;

    app.Run();

    app.Shutdown();

    return 0;
}
