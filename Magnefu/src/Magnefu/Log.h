#pragma once

#include "Core.h"
#include "spdlog/spdlog.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/fmt/ostr.h"


namespace Magnefu
{
	class  Log
	{
	public:
		Log() = delete;
		Log(const Log&) = delete;

		static void Init();

		inline static std::shared_ptr<spdlog::logger>& GetCoreLogger() { return s_CoreLogger; }
		inline static std::shared_ptr<spdlog::logger>& GetClientLogger() { return s_ClientLogger; }

	private:
		static std::shared_ptr<spdlog::logger> s_CoreLogger;
		static std::shared_ptr<spdlog::logger> s_ClientLogger;

	};

}

#define MF_CORE_ERROR(...)     ::Magnefu::Log::GetCoreLogger()->error(__VA_ARGS__)
#define MF_CORE_WARN(...)      ::Magnefu::Log::GetCoreLogger()->warn(__VA_ARGS__)
#define MF_CORE_INFO(...)      ::Magnefu::Log::GetCoreLogger()->info(__VA_ARGS__)
#define MF_CORE_DEBUG(...)     ::Magnefu::Log::GetCoreLogger()->debug(__VA_ARGS__)
#define MF_CORE_TRACE(...)     ::Magnefu::Log::GetCoreLogger()->trace(__VA_ARGS__)
#define MF_CORE_CRITICAL(...)  ::Magnefu::Log::GetCoreLogger()->critical(__VA_ARGS__)


#define MF_ERROR(...)          ::Magnefu::Log::GetClientLogger()->error(__VA_ARGS__)
#define MF_WARN(...)           ::Magnefu::Log::GetClientLogger()->warn(__VA_ARGS__)
#define MF_INFO(...)           ::Magnefu::Log::GetClientLogger()->info(__VA_ARGS__)
#define MF_DEBUG(...)          ::Magnefu::Log::GetClientLogger()->debug(__VA_ARGS__)
#define MF_TRACE(...)          ::Magnefu::Log::GetClientLogger()->trace(__VA_ARGS__)
#define MF_CRITICAL(...)       ::Magnefu::Log::GetClientLogger()->critical(__VA_ARGS__)
