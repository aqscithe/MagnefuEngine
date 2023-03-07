#pragma once

#include "Application.h"


#ifdef MF_PLATFORM_WINDOWS

#include <memory>

extern std::unique_ptr<Magnefu::Application> Magnefu::CreateApplication();


int main(int argc, char** argv)
{
	Magnefu::Log::Init();
	MF_CORE_DEBUG("Quantum Entanglement Engaged");

	auto app = Magnefu::CreateApplication();
	app->Run();
}

#endif