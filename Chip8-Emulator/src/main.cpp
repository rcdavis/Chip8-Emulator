
#include "Log.h"
#include "Application.h"

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
