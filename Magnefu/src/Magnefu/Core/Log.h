#pragma once

// -- Core Includes -- //
#include "Service.hpp"
#include "Platform.hpp"


// -- vendor Indcludes -- //
#include "spdlog/spdlog.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/fmt/ostr.h"


namespace Magnefu
{

	struct LogServiceConfiguration
	{
		cstring coreLogName;
		cstring clientLogName;

	};

	typedef void                        (*PrintCallback)(const char*);  // Additional callback for printing


	struct LogService : public Service 
	{

		MF_DECLARE_SERVICE(LogService);

		public:

			void Init(void* configuration);

			inline static std::shared_ptr<spdlog::logger>& GetCoreLogger() { return s_CoreLogger; }
			inline static std::shared_ptr<spdlog::logger>& GetClientLogger() { return s_ClientLogger; }

			void                            SetCallback(PrintCallback callback);

			
	

		private:
			PrintCallback                   print_callback = nullptr;

			static constexpr cstring        k_name = "Magnefu_Log_Service";
			static std::shared_ptr<spdlog::logger> s_CoreLogger;
			static std::shared_ptr<spdlog::logger> s_ClientLogger;
	};


	

}

#ifdef MF_DEBUG
	#define MF_ENABLE_LOGS
#endif

#ifdef MF_ENABLE_LOGS
	#define MF_CORE_ERROR(...)     ::Magnefu::LogService::GetCoreLogger()->error(__VA_ARGS__)
	#define MF_CORE_WARN(...)      ::Magnefu::LogService::GetCoreLogger()->warn(__VA_ARGS__)
	#define MF_CORE_INFO(...)      ::Magnefu::LogService::GetCoreLogger()->info(__VA_ARGS__)
	#define MF_CORE_DEBUG(...)     ::Magnefu::LogService::GetCoreLogger()->debug(__VA_ARGS__)
	#define MF_CORE_TRACE(...)     ::Magnefu::LogService::GetCoreLogger()->trace(__VA_ARGS__)
	#define MF_CORE_CRITICAL(...)  ::Magnefu::LogService::GetCoreLogger()->critical(__VA_ARGS__)


	#define MF_APP_ERROR(...)          ::Magnefu::LogService::GetClientLogger()->error(__VA_ARGS__)
	#define MF_APP_WARN(...)           ::Magnefu::LogService::GetClientLogger()->warn(__VA_ARGS__)
	#define MF_APP_INFO(...)           ::Magnefu::LogService::GetClientLogger()->info(__VA_ARGS__)
	#define MF_APP_DEBUG(...)          ::Magnefu::LogService::GetClientLogger()->debug(__VA_ARGS__)
	#define MF_APP_TRACE(...)          ::Magnefu::LogService::GetClientLogger()->trace(__VA_ARGS__)
	#define MF_APP_CRITICAL(...)       ::Magnefu::LogService::GetClientLogger()->critical(__VA_ARGS__)
#else
	#define MF_CORE_ERROR(...)   
	#define MF_CORE_WARN(...)      
	#define MF_CORE_INFO(...)      ::Magnefu::LogService::GetCoreLogger()->info(__VA_ARGS__)
	#define MF_CORE_DEBUG(...)   
	#define MF_CORE_TRACE(...)   
	#define MF_CORE_CRITICAL(...)


	#define MF_APP_ERROR(...)        
	#define MF_APP_WARN(...)         
	#define MF_APP_INFO(...)         
	#define MF_APP_DEBUG(...)        
	#define MF_APP_TRACE(...)        
	#define MF_APP_CRITICAL(...)     
#endif
