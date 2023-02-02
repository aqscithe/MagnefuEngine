#pragma once


#include <chrono>

class Timer
{
	public:
		Timer();
		~Timer();

		void StartTimer();

		std::chrono::steady_clock::time_point m_StartTimepoint;
};