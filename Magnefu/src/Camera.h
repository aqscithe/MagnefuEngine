#pragma once

#include "Vectors.h"
#include "Matrices.h"

namespace Magnefu
{
	struct CameraProperties
	{
		float AspectRatio;
		float Near;
		float Far;
		float Top;
		float Bottom;
		float Right;
		float Left;
		bool  IsOrtho;
	};

	class  Camera
	{
	public:
		CameraProperties m_Properties;
		Maths::vec3 m_Position;
		Maths::vec3 m_Target;
		Maths::vec3 m_Forward;
		float m_Speed = 8.f;

	private:

		Maths::vec3 m_Up;
		Maths::vec3 m_Right;
		Maths::mat4 m_View;



	public:
		Camera();
		Camera(const Maths::vec3& position, const Maths::vec3& target = { 0.f, 0.f, 0.f });

		void CalculateView();

		void ProcessInput(float deltaTime);

		// just learned i don't need inline here - these functions are implicitly inline
		// https://en.cppreference.com/w/cpp/language/inline
		inline const Maths::mat4& GetView() const { return m_View; }
		inline const Maths::vec3& GetRight() const { return m_Right; }
		inline const Maths::vec3& GetUp() const { return m_Up; }
		inline const Maths::vec3& GetForward() const { return m_Forward; }
		inline const Maths::vec3& GetPosition() const { return m_Position; }

	private:
		void CreateCameraAxes();

		void SetPostion(Maths::vec3& position);

	};
}

