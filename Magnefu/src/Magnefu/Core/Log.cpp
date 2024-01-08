// -- PCH -- //
#include "mfpch.h"

// -- header -- //
#include "Log.h"

namespace Magnefu
{
	static LogService s_LogService;

	std::shared_ptr<spdlog::logger> LogService::s_ClientLogger;
	std::shared_ptr<spdlog::logger> LogService::s_CoreLogger;


	LogService* LogService::Instance()
	{
		return &s_LogService;
	}


	void LogService::Init(void* configuration) 
	{

		LogServiceConfiguration* log_configuration = static_cast<LogServiceConfiguration*>(configuration);
		

		spdlog::set_pattern("%^[%T] | [%n] | [thread %t] | %v%$");

		s_CoreLogger = spdlog::stdout_color_mt(log_configuration ? log_configuration->coreLogName : "Magnefu");
		s_CoreLogger->set_level(spdlog::level::trace);
		s_CoreLogger->trace("Magnefu Logger initialized");

		s_ClientLogger = spdlog::stdout_color_mt(log_configuration ? log_configuration->clientLogName : "App");
		s_ClientLogger->set_level(spdlog::level::trace);
		s_ClientLogger->trace("App Logger initialized");

	}

	void LogService::SetCallback(PrintCallback callback)
	{
		print_callback = callback;
		
	}
}

