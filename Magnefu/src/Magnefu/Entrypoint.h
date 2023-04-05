#pragma once

#include "Application.h"


#ifdef MF_PLATFORM_WINDOWS



int main(int argc, char** argv)
{
	Magnefu::Log::Init();
	auto app = Magnefu::CreateApplication();
	app->Run();
}

#endif