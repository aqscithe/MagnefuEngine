// -- PCH -- //
#include "mfpch.h"

// -- HEADER -- //
#include "SceneCamera.h"


// -- Application Includes ------------- //
#include "Magnefu/Application/Input/Input.h"
#include "Magnefu/Application/Input/KeyCodes.h"


// -- Vendor Includes -------------- //
#include "imgui.h"




namespace Magnefu
{
#define BIND_EVENT_FN(x) std::bind(&x, this, std::placeholders::_1)

	SceneCamera::SceneCamera(const CameraOrientation& orientation, const CameraProps& props)
	{
		MF_PROFILE_FUNCTION();
		Init(orientation, props);
	}

	SceneCamera::~SceneCamera()
	{

	}

	void SceneCamera::OnEvent(Event& e)
	{
		EventDispatcher dispatcher(e);
		dispatcher.Dispatch<WindowResizeEvent>(BIND_EVENT_FN(SceneCamera::OnWindowResize));
		dispatcher.Dispatch<MouseScrolledEvent>(BIND_EVENT_FN(SceneCamera::OnMouseScrolled));
	}

	bool SceneCamera::OnWindowResize(WindowResizeEvent& e)
	{
		//RenderCommand::SetWindowSize(e.GetWidth(), e.GetHeight());
		return false;
	}

	bool SceneCamera::OnMouseScrolled(MouseScrolledEvent& e)
	{
		
		return false;
	}

	void SceneCamera::Init(const CameraOrientation& orientation, const CameraProps& props)
	{
		m_Data.Position = orientation.Position;
		m_Data.Yaw = orientation.Yaw;
		m_Data.Pitch = orientation.Pitch;
		m_Data.AspectRatio = props.AspectRatio;
		m_Data.Type = props.Type;
		m_Data.Speed = props.Speed;
		m_Data.FOV = props.FOV;
		m_Data.Near = props.Near;
		m_Data.Far = props.Far;
		
		m_Forward = Maths::normalize(m_Data.Position - orientation.Target);
		m_Right = Maths::normalize(Maths::crossProduct(Maths::vec3(0.0f, 1.0f, 0.0f), m_Forward));
		m_Up = Maths::normalize(Maths::crossProduct(m_Forward, m_Right));

		m_IsOrtho = false;
		m_IsPersp = true;
	}

	Maths::mat4& SceneCamera::CalculateView()
	{
		m_View = Maths::rotateX(m_Data.Pitch) * Maths::rotateY(m_Data.Yaw) * Maths::axis(m_Right, m_Up, m_Forward) * Maths::translate(-m_Data.Position);
		return m_View;
	}

	Maths::mat4& SceneCamera::CalculateProjection()
	{
		switch (m_Data.Type)
		{
			case CameraType::None:
			{
				MF_CORE_ASSERT(false, "CameraType::None is not a valid camera type.");
				break;
			}
			case CameraType::Orthographic:
			{
				float top = m_Data.Near * tan(m_Data.FOV / 2.f);
				float right = top * m_Data.AspectRatio;
				m_Projection = Maths::orthographic(-right, right, -top, top, m_Data.Near, m_Data.Far);
				return m_Projection;
			}
			case CameraType::Perspective:
			{
				m_Projection = Maths::perspective(Maths::toRadians(m_Data.FOV), m_Data.AspectRatio, m_Data.Near, m_Data.Far);
				return m_Projection;
			}
		}
		
		MF_CORE_ASSERT(false, "Unknown camera type");
		return m_Projection;
	}

	Maths::mat4& SceneCamera::CalculateVP()
	{
		m_VP = CalculateProjection() * CalculateView();
		return m_VP;
	}

	void SceneCamera::ProcessInput(double deltaTime)
	{
		/*double cameraSpeed = m_Data.Speed * deltaTime;
		double forwardMovement = 0.0;
		if (Input::IsKeyPressed(MF_KEY_W) || Input::IsKeyPressed(MF_KEY_UP))
			forwardMovement += cameraSpeed;
		if (Input::IsKeyPressed(MF_KEY_S) || Input::IsKeyPressed(MF_KEY_DOWN))
			forwardMovement -= cameraSpeed;


		double strafeMovement = 0.0;
		if (Input::IsKeyPressed(MF_KEY_A) || Input::IsKeyPressed(MF_KEY_LEFT))
			strafeMovement -= cameraSpeed;
		if (Input::IsKeyPressed(MF_KEY_D) || Input::IsKeyPressed(MF_KEY_RIGHT))
			strafeMovement += cameraSpeed;

		double verticalMovement = 0.0;
		if (Input::IsKeyPressed(MF_KEY_E))
			verticalMovement += cameraSpeed;
		if (Input::IsKeyPressed(MF_KEY_Q))
			verticalMovement -= cameraSpeed;*/


		/*m_Data.Position.x += Maths::sin(m_Data.Yaw) * forwardMovement;
		m_Data.Position.z -= Maths::cos(m_Data.Yaw) * forwardMovement;
		m_Data.Position.x += Maths::cos(m_Data.Yaw) * strafeMovement;
		m_Data.Position.z += Maths::sin(m_Data.Yaw) * strafeMovement;
		m_Data.Position.y -= Maths::sin(m_Data.Pitch) * forwardMovement;
		m_Data.Position.y += verticalMovement;*/
	}


	void SceneCamera::SetDefaultProps()
	{	
		m_Data.Yaw = 1.3f;
		m_Data.Pitch = -0.3f;
		m_Data.FOV = 45.f;
		m_Data.Position = { 0.f, 0.f, 4.f };
	}
	
}