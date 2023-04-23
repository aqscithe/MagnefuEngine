#pragma once

namespace Magnefu
{
	// TODO:
	// integration formulas to consult when you start physics simulation
	// https://gafferongames.com/post/fix_your_timestep/

	// render data = (current data * renderInterpCoeff) + previous data * (1.f - renderInterpCoeff);
	class TimeStep
	{
		using Clock = std::chrono::steady_clock;

	public:
		// TODO: Include frame target
		// TODO: Ability to adjust frame target 30, 45, 60, 120, 144, unlocked
		TimeStep(float frameTarget = 1.f / 144.f) :
			m_FrameTarget(frameTarget)
		{
			m_CurrentTime = Clock::now();
			//m_AccumulatedTime = 0.0;
		}

		void CalculateDeltaTime()
		{
			auto newTime = Clock::now();
			auto frameTime = newTime - m_CurrentTime;
			m_DeltaTime = std::chrono::duration<float>(frameTime).count();
			m_CurrentTime = newTime;
		}

		inline float GetDeltaTime() { return m_DeltaTime; }


		/*void CalculateDeltaTime()
		{
			auto newTime = Clock::now();
			auto frameTime = newTime - m_CurrentTime;
			float deltaTime = std::chrono::duration<float>(frameTime).count();
			Maths::clamp(0.f, 0.25f, deltaTime);

			m_CurrentTime = newTime;
			m_AccumulatedTime += deltaTime;
		}*/

		//inline void DecrementTime() { m_AccumulatedTime -= m_FrameTarget; }
		//
		//inline bool const TimeLeftInFrame() const { return m_AccumulatedTime >= m_FrameTarget; }
		//
		//inline float const GetDeltaTime() const { return m_AccumulatedTime; }
		//
		//inline float CalculateInterpolationCoeff() { return m_AccumulatedTime / m_FrameTarget; }


	private:

		// NOTE: This needs to adjust dynamically if outside conditions force a different frame target. 
		// For example, when I unplug my laptop my screen's refresh rate locks to 30fps to save on energy.
		Clock::time_point m_CurrentTime;
		const float m_FrameTarget;
		float m_DeltaTime;
		
	};
}