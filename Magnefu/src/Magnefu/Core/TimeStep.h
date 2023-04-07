#pragma once

namespace Magnefu
{
	class TimeStep
	{
		using Clock = std::chrono::steady_clock;

	public:
		TimeStep(float frameTarget = 1.f / 60.f) :
			m_FrameTarget(frameTarget)
		{
			
			m_CurrentTime = Clock::now();
			m_AccumulatedTime = 0.0;
		}

		void CalculateDeltaTime()
		{
			auto newTime = Clock::now();
			auto frameTime = newTime - m_CurrentTime;
			float deltaTime = std::chrono::duration<float>(frameTime).count();
			Maths::clamp(0.f, 0.25f, deltaTime);

			m_CurrentTime = newTime;

			m_AccumulatedTime += deltaTime;
		}

		inline void DecrementTime() { m_AccumulatedTime -= m_FrameTarget; }

		inline bool const TimeLeftInFrame() const { return m_AccumulatedTime >= m_FrameTarget; }

		inline float const GetDeltaTime() const { return m_AccumulatedTime; }

		inline float CalculateInterpolationCoeff() { return m_AccumulatedTime / m_FrameTarget; }


	private:

		// NOTE: This needs to adjust dynamically if outside conditions force a different frame target. 
		// For example, when I unplug my laptop my screen's refresh rate locks to 30fps to save on energy.
		const float m_FrameTarget;
		float m_AccumulatedTime;
		Clock::time_point m_CurrentTime;
	};
}