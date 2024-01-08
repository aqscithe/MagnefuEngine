#pragma once


// -- Core Includes ---------------------------------- //
#include "Magnefu/Core/Log.h"


// -- vendor Includes ---------- //
#include <string>
#include <chrono>
#include <algorithm>
#include <fstream>
#include <sstream>
#include <mutex>
#include <thread>
#include <iomanip>




// TODO: Ability to start and stop a profile while running the application
// TODO: Ability to stop after a certain time period and/or number of frames
// maybe in ImGui, I can select which function(s) to run the profile on

// TODO: static Frame class/struct to track frame count  and other frame
// related data

namespace Magnefu
{
	using FloatMicroSeconds = std::chrono::duration<double, std::micro>;

	struct ProfileResult
	{
		std::string Name;
		FloatMicroSeconds Start;
		std::chrono::microseconds ElapsedTime;
		std::thread::id ThreadID;
	};

	struct InstrumentationSession
	{
		std::string Name;
	};

	class Instrumentor
	{
	public:
		Instrumentor(const Instrumentor&) = delete;
		Instrumentor(Instrumentor&&) = delete;

		void BeginSession(const std::string& name, const std::string& filepath = "results.json")
		{
			std::lock_guard lock(m_Mutex);
			if (m_CurrentSession)
			{
				if(LogService::GetCoreLogger())
					MF_CORE_ERROR("Instrumentor::BeginSession - Can't open session {0}. Session {1} already open.", name, m_CurrentSession->Name);

				InternalEndSession();
			}

			m_OutputStream.open(filepath);
			if (m_OutputStream.is_open())
			{
				WriteHeader();
				m_CurrentSession = new InstrumentationSession({ name });
			}
			else
			{
				if (LogService::GetCoreLogger())
					MF_CORE_ERROR("Instrumentor::BeginSession - Failed to open results file: {0}", filepath);

				InternalEndSession();
			}
			
		}

		void EndSession()
		{
			std::lock_guard lock(m_Mutex);
			InternalEndSession();
		}

		void WriteProfile(const ProfileResult& result)
		{
			std::stringstream json;

			json << std::setprecision(3) << std::fixed;
			json << ",{";
			json << "\"cat\":\"function\",";
			json << "\"dur\":" << (result.ElapsedTime.count()) << ',';
			json << "\"name\":\"" << result.Name << "\",";
			json << "\"ph\":\"X\",";
			json << "\"pid\":0,";
			json << "\"tid\":" << result.ThreadID << ",";
			json << "\"ts\":" << result.Start.count();
			json << "}";

			std::lock_guard lock(m_Mutex);
			if (m_CurrentSession)
			{
				m_OutputStream << json.str();
				m_OutputStream.flush();
			}
		}

		static Instrumentor& Get()
		{
			static Instrumentor instance;
			return instance;
		}

	private:
		Instrumentor()
			: m_CurrentSession(nullptr)
		{
		}

		~Instrumentor()
		{
			EndSession();
		}

		void WriteHeader()
		{
			m_OutputStream << "{\"otherData\": {},\"traceEvents\":[{}";
			m_OutputStream.flush();
		}

		void WriteFooter()
		{
			m_OutputStream << "]}";
			m_OutputStream.flush();
		}

		// Note: you must already own lock on m_Mutex before
		// calling InternalEndSession()
		void InternalEndSession()
		{
			if (m_CurrentSession)
			{
				WriteFooter();
				m_OutputStream.close();
				delete m_CurrentSession;
				m_CurrentSession = nullptr;
			}
		}

	private:
		std::ofstream m_OutputStream;
		std::mutex m_Mutex;
		InstrumentationSession* m_CurrentSession;

	};

	class InstrumentationTimer
	{
	public:
		InstrumentationTimer(const char* name) :
			m_Name(name), m_Running(true)
		{
			m_StartTimepoint = std::chrono::high_resolution_clock::now();
		}

		~InstrumentationTimer()
		{
			if (m_Running)
				Stop();
		}

		void Stop()
		{
			auto endTimePoint = std::chrono::high_resolution_clock::now();

			auto start = FloatMicroSeconds{ m_StartTimepoint.time_since_epoch() };
			auto elapsedTime = std::chrono::time_point_cast<std::chrono::microseconds>(endTimePoint).time_since_epoch() -
				std::chrono::time_point_cast<std::chrono::microseconds>(m_StartTimepoint).time_since_epoch();

			Instrumentor::Get().WriteProfile({m_Name, start, elapsedTime, std::this_thread::get_id()});

			m_Running = false;

		}

		std::chrono::steady_clock::time_point m_StartTimepoint;
		const char* m_Name;
		bool m_Running;
	};

	namespace InstrumentorUtils {

		template <size_t N>
		struct ChangeResult
		{
			char Data[N];
		};

		template <size_t N, size_t K>
		constexpr auto CleanupOutputString(const char(&expr)[N], const char(&remove)[K])
		{
			ChangeResult<N> result = {};

			size_t srcIndex = 0;
			size_t dstIndex = 0;
			while (srcIndex < N)
			{
				size_t matchIndex = 0;
				while (matchIndex < K - 1 && srcIndex + matchIndex < N - 1 && expr[srcIndex + matchIndex] == remove[matchIndex])
					matchIndex++;
				if (matchIndex == K - 1)
					srcIndex += matchIndex;
				result.Data[dstIndex++] = expr[srcIndex] == '"' ? '\'' : expr[srcIndex];
				srcIndex++;
			}
			return result;
		}
	}
}

#ifdef MF_DEBUG
	#define MF_PROFILE
#endif

#ifdef MF_PROFILE
	#define MF_PROFILE_BEGIN_SESSION(name, filepath) ::Magnefu::Instrumentor::Get().BeginSession(name, filepath)
	#define MF_PROFILE_END_SESSION()                 ::Magnefu::Instrumentor::Get().EndSession()
	#define MF_PROFILE_SCOPE_LINE2(name, line)		 constexpr auto fixedName##line = ::Magnefu::InstrumentorUtils::CleanupOutputString(name, "__cdecl ");\
															::Magnefu::InstrumentationTimer timer##line(fixedName##line.Data)
	#define MF_PROFILE_SCOPE_LINE(name, line)		 MF_PROFILE_SCOPE_LINE2(name, line)
	#define MF_PROFILE_SCOPE(name)					 MF_PROFILE_SCOPE_LINE(name, __LINE__)
	#define MF_PROFILE_FUNCTION()					 MF_PROFILE_SCOPE(__FUNCSIG__)
#else
	#define MF_PROFILE_BEGIN_SESSION(name, filepath)
	#define MF_PROFILE_END_SESSION()
	#define MF_PROFILE_SCOPE(name)
	#define MF_PROFILE_FUNCTION()
#endif