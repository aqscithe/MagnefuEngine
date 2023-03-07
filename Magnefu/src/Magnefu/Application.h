#pragma once

#include "Core.h"
#include <memory>

namespace Magnefu
{
	class MAGNEFU_API Application
	{
	public:
		Application();
		virtual ~Application();

		virtual void Run();

	};

	// to be defined in client
	std::unique_ptr<Application> CreateApplication();
}