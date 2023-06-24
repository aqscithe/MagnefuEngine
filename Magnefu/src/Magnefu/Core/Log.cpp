#include "mfpch.h"

namespace Magnefu
{
	std::shared_ptr<spdlog::logger> Log::s_ClientLogger;
	std::shared_ptr<spdlog::logger> Log::s_CoreLogger;


	void Log::Init()
	{
		spdlog::set_pattern("%^[%T] | [%n] | [thread %t] | %v%$");

		s_CoreLogger = spdlog::stdout_color_mt("Magnefu");
		s_CoreLogger->set_level(spdlog::level::trace);
		s_CoreLogger->trace("Magnefu Logger initialized");

		s_ClientLogger = spdlog::stdout_color_mt("App");
		s_ClientLogger->set_level(spdlog::level::trace);
		s_ClientLogger->trace("App Logger initialized");
	}
}

