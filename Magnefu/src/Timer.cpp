#include "Timer.h"
#include <iostream>


Timer::Timer()
{
	StartTimer();
}

Timer::~Timer()
{
	auto EndTimepoint = std::chrono::high_resolution_clock::now();

	auto Start = std::chrono::time_point_cast<std::chrono::microseconds>(m_StartTimepoint).time_since_epoch().count();
	auto End = std::chrono::time_point_cast<std::chrono::microseconds>(EndTimepoint).time_since_epoch().count();

	auto duration = End - Start;
	auto ms = duration * .001;

	std::cout << duration << " us | " << ms << " ms" << std::endl;
}

void Timer::StartTimer()
{
	m_StartTimepoint = std::chrono::high_resolution_clock::now();
}