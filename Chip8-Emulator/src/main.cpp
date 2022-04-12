
#include "Log.h"
#include "Application.h"

int main()
{
    Log::Init();

    Application app;
    if (!app.TestInit())
        return -1;

    app.TestRun();

    app.TestShutdown();

	return 0;
}
