#pragma once

#include "Camera.h"

namespace Magnefu
{
	class PerspectiveCamera : public Camera
	{
	public:
		PerspectiveCamera(const CameraOrientation& orientation, const CameraProps& props);
		~PerspectiveCamera();

		inline const Maths::mat4& GetView() const override { return m_View; }
		inline const Maths::mat4& GetProjection() const override { return m_Projection; }
		inline const Maths::mat4& GetVP() const override { return m_VP; }
		inline CameraData& GetData() override { return m_Data; }

		Maths::mat4& CalculateView() override;
		Maths::mat4& CalculateProjection() override;
		Maths::mat4& CalculateVP() override;
		void ProcessInput(float deltaTime) override;
		void OnImGuiRender() override;

	protected:
		void Init(const CameraOrientation& orientation, const CameraProps& props) override;


	private:
		Maths::vec3 m_Up;
		Maths::vec3 m_Right;
		Maths::vec3 m_Forward;
		Maths::mat4 m_View;
		Maths::mat4 m_Projection;
		Maths::mat4 m_VP;

		CameraData m_Data;

	};
}