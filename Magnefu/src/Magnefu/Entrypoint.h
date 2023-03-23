#pragma once

#include "Application.h"


#ifdef MF_PLATFORM_WINDOWS


extern "C"
{
	__declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;
	__declspec(dllexport) int NvOptimusEnablement = 0x00000001;
}

extern Magnefu::Application* Magnefu::CreateApplication();


int main(int argc, char** argv)
{
	Magnefu::Log::Init();
	auto app = Magnefu::CreateApplication();
	app->Run();
}

#endif