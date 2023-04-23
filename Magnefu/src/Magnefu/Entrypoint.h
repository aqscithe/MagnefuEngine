#pragma once

#include "Application.h"


#ifdef MF_PLATFORM_WINDOWS

int main(int argc, char** argv)
{
	MF_PROFILE_BEGIN_SESSION("Magnefu Startup", "Magnefu-Startup.json");
	Magnefu::Log::Init();
	auto app = Magnefu::CreateApplication();
	MF_PROFILE_END_SESSION();

	MF_PROFILE_BEGIN_SESSION("Magnefu Runtime", "Magnefu-Runtime.json");
	app->Run();
	MF_PROFILE_END_SESSION();

	MF_PROFILE_BEGIN_SESSION("Magnefu Shutdown", "Magnefu-Shutdown.json");
	delete app;
	MF_PROFILE_END_SESSION();
}

#endif