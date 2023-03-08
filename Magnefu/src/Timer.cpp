#include "Timer.h"
#include "Magnefu/Log.h"


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

	MF_CORE_DEBUG("{0} us | {1} ms", duration, ms);
}

void Timer::StartTimer()
{
	m_StartTimepoint = std::chrono::high_resolution_clock::now();
}