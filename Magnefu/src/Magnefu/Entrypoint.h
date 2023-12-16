#pragma once

#include "Application.h"


#ifdef MF_PLATFORM_WINDOWS

int main(int argc, char** argv)
{
	MF_PROFILE_BEGIN_SESSION("Magnefu Startup", "Magnefu-Startup.json");

	// -- Init Services ------------------------------------------- //

	Magnefu::LogService::Instance()->Init(nullptr);
	Magnefu::MemoryService::Instance()->Init(nullptr);


	auto app = Magnefu::CreateApplication();

	MF_PROFILE_END_SESSION();



	MF_PROFILE_BEGIN_SESSION("Magnefu Runtime", "Magnefu-Runtime.json");

	app->Run();

	MF_PROFILE_END_SESSION();



	MF_PROFILE_BEGIN_SESSION("Magnefu Shutdown", "Magnefu-Shutdown.json");

	delete app;

	Magnefu::MemoryService::Instance()->Shutdown();
	Magnefu::LogService::Instance()->Shutdown();

	MF_PROFILE_END_SESSION();
}

#endif