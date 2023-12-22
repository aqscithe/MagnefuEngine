#pragma once

#include "Application.h"
#include "Magnefu/Core/Debug/Instrumentor.h"
#include "Magnefu/Core/Time.hpp"


#ifdef MF_PLATFORM_WINDOWS

int main(int argc, char** argv)
{
	MF_PROFILE_BEGIN_SESSION("Magnefu Startup", "Magnefu-Startup.json");

	// -- Init Services ------------------------------------------- //

	Magnefu::time_service_init();
	Magnefu::LogService::Instance()->Init(nullptr);
	Magnefu::MemoryService::Instance()->Init(nullptr);


	auto app = Magnefu::CreateApplication();

	MF_PROFILE_END_SESSION();



	MF_PROFILE_BEGIN_SESSION("Magnefu Runtime", "Magnefu-Runtime.json");

	app->Run();

	MF_PROFILE_END_SESSION();



	MF_PROFILE_BEGIN_SESSION("Magnefu Shutdown", "Magnefu-Shutdown.json");

	delete app;

	// -- Shutdown Services ------------------------------------------- //

	Magnefu::MemoryService::Instance()->Shutdown();
	Magnefu::LogService::Instance()->Shutdown();

	MF_PROFILE_END_SESSION();
}

#endif