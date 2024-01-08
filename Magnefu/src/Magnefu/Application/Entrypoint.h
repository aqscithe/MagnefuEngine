#pragma once

#include "Application.h"
#include "Magnefu/Core/Debug/Instrumentor.h"
#include "Magnefu/Core/Time.hpp"


#ifdef MF_PLATFORM_WINDOWS

int main(int argc, char** argv)
{
	MF_PROFILE_BEGIN_SESSION("Magnefu Startup", "Magnefu-Startup.json");
	auto app = Magnefu::CreateApplication();
	MF_PROFILE_END_SESSION();


	MF_PROFILE_BEGIN_SESSION("Magnefu Runtime", "Magnefu-Runtime.json");
	app->Run({1920, 1080, "Magnefu", true});
	MF_PROFILE_END_SESSION();


	MF_PROFILE_BEGIN_SESSION("Magnefu Shutdown", "Magnefu-Shutdown.json");

	delete app;

	MF_PROFILE_END_SESSION();
}

#endif