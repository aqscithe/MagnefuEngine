#pragma once

#include "Camera.h"
#include "Magnefu/Core/Events/ApplicationEvent.h"
#include "Magnefu/Core/Events/MouseEvent.h"

namespace Magnefu
{
	class SceneCamera : public Camera
	{
	public:
		SceneCamera(const CameraOrientation& orientation, const CameraProps& props);
		~SceneCamera();

		void OnEvent(Event& e) override;

		inline const Maths::mat4& GetView() const override { return m_View; }
		inline const Maths::mat4& GetProjection() const override { return m_Projection; }
		inline const Maths::mat4& GetVP() const override { return m_VP; }
		inline CameraData& GetData() override { return m_Data; }
		inline bool IsOrtho() override { return m_IsOrtho; }
		inline bool IsPerspective() override { return m_IsPersp; }

		inline void SetOrtho(bool ortho) override { m_IsOrtho = ortho; }
		inline void SetPerspective(bool persp) override { m_IsPersp = persp; }


		Maths::mat4& CalculateView() override;
		Maths::mat4& CalculateProjection() override;
		Maths::mat4& CalculateVP() override;
		void ProcessInput(float deltaTime) override;
		void SetDefaultProps() override;
		void SetAspectRatio(float aspectRatio) { m_Data.AspectRatio = aspectRatio; }

	protected:
		void Init(const CameraOrientation& orientation, const CameraProps& props) override;

	private:
		bool OnWindowResize(WindowResizeEvent& e);
		bool OnMouseScrolled(MouseScrolledEvent& e);


	private:
		CameraData m_Data;
		Maths::mat4 m_View;
		Maths::mat4 m_Projection;
		Maths::mat4 m_VP;
		Maths::vec3 m_Up;
		Maths::vec3 m_Right;
		Maths::vec3 m_Forward;

		bool m_IsOrtho;
		bool m_IsPersp;

	};
}