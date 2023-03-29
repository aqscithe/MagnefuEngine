#include "mfpch.h"
#include "PerspectiveCamera.h"

#include "Magnefu/Input.h"
#include "Magnefu/KeyCodes.h"

#include "imgui.h"

namespace Magnefu
{
	PerspectiveCamera::PerspectiveCamera(const CameraOrientation& orientation, const CameraProps& props)
	{
		Init(orientation, props);
	}

	PerspectiveCamera::~PerspectiveCamera()
	{

	}

	void PerspectiveCamera::Init(const CameraOrientation& orientation, const CameraProps& props)
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
	}

	Maths::mat4& PerspectiveCamera::CalculateView()
	{
		m_View = Maths::rotateX(m_Data.Pitch) * Maths::rotateY(m_Data.Yaw) * Maths::axis(m_Right, m_Up, m_Forward) * Maths::translate(-m_Data.Position);
		return m_View;
	}

	Maths::mat4& PerspectiveCamera::CalculateProjection()
	{
		m_Projection = Maths::perspective(Maths::toRadians(m_Data.FOV), m_Data.AspectRatio, m_Data.Near, m_Data.Far);
		return m_Projection;
	}

	Maths::mat4& PerspectiveCamera::CalculateVP()
	{
		m_VP = CalculateProjection() * CalculateView();
		return m_VP;
	}

	void PerspectiveCamera::ProcessInput(float deltaTime)
	{
		float cameraSpeed = m_Data.Speed * deltaTime;
		float forwardMovement = 0.f;
		if (Input::IsKeyPressed(MF_KEY_W) || Input::IsKeyPressed(MF_KEY_UP))
			forwardMovement += cameraSpeed;
		if (Input::IsKeyPressed(MF_KEY_S) || Input::IsKeyPressed(MF_KEY_DOWN))
			forwardMovement -= cameraSpeed;


		float strafeMovement = 0.f;
		if (Input::IsKeyPressed(MF_KEY_A) || Input::IsKeyPressed(MF_KEY_LEFT))
			strafeMovement -= cameraSpeed;
		if (Input::IsKeyPressed(MF_KEY_D) || Input::IsKeyPressed(MF_KEY_RIGHT))
			strafeMovement += cameraSpeed;

		float verticalMovement = 0.f;
		if (Input::IsKeyPressed(MF_KEY_E))
			verticalMovement += cameraSpeed;
		if (Input::IsKeyPressed(MF_KEY_Q))
			verticalMovement -= cameraSpeed;


		m_Data.Position.x += Maths::sin(m_Data.Yaw) * forwardMovement;
		m_Data.Position.z -= Maths::cos(m_Data.Yaw) * forwardMovement;
		m_Data.Position.x += Maths::cos(m_Data.Yaw) * strafeMovement;
		m_Data.Position.z += Maths::sin(m_Data.Yaw) * strafeMovement;
		m_Data.Position.y -= Maths::sin(m_Data.Pitch) * forwardMovement;
		m_Data.Position.y += verticalMovement;
	}

	void PerspectiveCamera::OnImGuiRender()
	{
		if (ImGui::TreeNodeEx("Camera", ImGuiTreeNodeFlags_DefaultOpen))
		{
			ImGui::SeparatorText("MATRICES");

			ImGui::Text("View");
			ImGui::Text("\t%.1f\t%.1f\t%.1f\t%.1f", m_View.e[0], m_View.e[1], m_View.e[2], m_View.e[3]);
			ImGui::Text("\t%.1f\t%.1f\t%.1f\t%.1f", m_View.e[4], m_View.e[5], m_View.e[6], m_View.e[7]);
			ImGui::Text("\t%.1f\t%.1f\t%.1f\t%.1f", m_View.e[8], m_View.e[9], m_View.e[10], m_View.e[11]);
			ImGui::Text("\t%.1f\t%.1f\t%.1f\t%.1f", m_View.e[12], m_View.e[13], m_View.e[14], m_View.e[15]);

			ImGui::SeparatorText("ORIENTATION");

			ImGui::SliderFloat3("Position", m_Data.Position.e, -10.f, 10.f);
			ImGui::Text("Yaw: %.2f", m_Data.Yaw);
			ImGui::Text("Pitch: %.2f", m_Data.Pitch);

			ImGui::SeparatorText("PROPERTIES");

			ImGui::Text("Type: Perspective");
			ImGui::Text("FOV: %.1f", m_Data.FOV);
			ImGui::DragFloat("Speed", &m_Data.Speed, 1.f, 20.f);
			ImGui::SliderFloat("Near", &m_Data.Near, 0.01f, 10.f);
			ImGui::SliderFloat("Far", &m_Data.Far, 10.f, 100.f);

			ImGui::TreePop();
		}
		
		
	}
	
}