#pragma once

#include "Magnefu.h"

#include "imgui/imgui.h"


// USE STACK ALLOCATOR HERE

class GameLayer : public Magnefu::Layer
{
public:
	GameLayer() :
		Layer("3D Primitives")
	{
		m_Plane.Size = { 20.f, 0.f, 20.f };
		m_Plane.Color = { 0.5f, 0.5f, 0.5f };
		m_Plane.Rotation = { 0.f, 0.f, 0.f };
		m_Plane.Translation = { 0.f, -1.f, 0.f };
		m_Plane.Angle = 0.f;

		m_Cube.Size = { 1.f, 1.f, 1.f };
		m_Cube.Color = { 1.f, 0.5f, 0.85f };
		m_Cube.Rotation = { 0.f, 0.f, 0.f };
		m_Cube.Translation = { -3.f, 0.f,0.f };
		m_Cube.Angle = 0.f;

		m_RecPrism.Size = { 2.5f, 1.f, 1.f };
		m_RecPrism.Color = { 0.f, 0.75f, 0.85f };
		m_RecPrism.Rotation = { 0.f, 0.f, 0.f };
		m_RecPrism.Translation = { 3.f, 0.f, 0.f };
		m_RecPrism.Angle = 0.f;

		m_Sphere.Radius = 1.f;
		m_Sphere.SectorCount = 32;
		m_Sphere.StackCount = 13;
		m_Sphere.Color = { 0.1f, 0.73f, 0.4f };
		m_Sphere.Rotation = { 0.f, 0.f, 0.f };
		m_Sphere.Translation = { 0.f, 3.f,0.f };
		m_Sphere.Angle = 0.f;

		m_Camera = std::static_pointer_cast<Magnefu::SceneCamera>(Magnefu::Application::Get().GetWindow().GetSceneCamera());
	}

	void OnAttach() override
	{
		m_Camera->SetDefaultProps();
	}

	void OnDetach() override
	{

	}

	void OnUpdate(float deltaTime) override
	{
		m_Camera->ProcessInput(deltaTime);
	}

	void OnRender() override
	{
		Magnefu::RenderCommand::ClearColor(0.08f, 0.08f, 0.08f, 1.f);
		Magnefu::RenderCommand::Clear();

		Magnefu::Renderer::BeginScene();
		Magnefu::Renderer::DrawPlane(m_Plane);
		Magnefu::Renderer::DrawCube(m_Cube);
		Magnefu::Renderer::DrawRectangularPrism(m_RecPrism);
		Magnefu::Renderer::DrawSphere(m_Sphere);
		Magnefu::Renderer::DrawSkybox();
		Magnefu::Renderer::EndScene();
	}

	void OnGUIRender() override
	{
		ImGui::Begin("Scene");
		ImGuiTabBarFlags tab_bar_flags = ImGuiTabBarFlags_None;
		if (ImGui::BeginTabBar("Scene", tab_bar_flags))
		{
			if (ImGui::BeginTabItem("PLANE"))
			{
				ImGui::SliderFloat3("Size", m_Plane.Size.e, 1.f, 20.f);
				ImGui::SliderFloat("Angle", &m_Plane.Angle, -360.f, 360.f);
				ImGui::SliderFloat3("Rotation", m_Plane.Rotation.e, -1.f, 1.f);
				ImGui::SliderFloat3("Translation", m_Plane.Translation.e, -20.f, 20.f);
				ImGui::ColorEdit3("Color", m_Plane.Color.e);
				ImGui::EndTabItem();
			}
			if (ImGui::BeginTabItem("CUBE"))
			{
				ImGui::SliderFloat("Size", m_Cube.Size.e, 0.1f, 20.f);
				ImGui::SliderFloat("Angle", &m_Cube.Angle, -360.f, 360.f);
				ImGui::SliderFloat3("Rotation", m_Cube.Rotation.e, -1.f, 1.f);
				ImGui::SliderFloat3("Translation", m_Cube.Translation.e, -20.f, 20.f);
				ImGui::EndTabItem();
			}
			if (ImGui::BeginTabItem("Rec Prism"))
			{
				ImGui::SliderFloat("Size", m_RecPrism.Size.e, 0.1f, 20.f);
				ImGui::SliderFloat("Angle", &m_RecPrism.Angle, -360.f, 360.f);
				ImGui::SliderFloat3("Rotation", m_RecPrism.Rotation.e, -1.f, 1.f);
				ImGui::SliderFloat3("Translation", m_RecPrism.Translation.e, -20.f, 20.f);
				ImGui::EndTabItem();
			}
			if (ImGui::BeginTabItem("SPHERE"))
			{
				ImGui::SliderFloat("Radius", &m_Sphere.Radius, 0.1f, 20.f);
				ImGui::SliderInt("Stack Count", (int*)&m_Sphere.StackCount, 2, 80);
				ImGui::SliderInt("Sector Count", (int*)&m_Sphere.SectorCount, 3, 100);
				ImGui::SliderFloat("Angle", &m_Sphere.Angle, -360.f, 360.f);
				ImGui::SliderFloat3("Rotation", m_Sphere.Rotation.e, -1.f, 1.f);
				ImGui::SliderFloat3("Translation", m_Sphere.Translation.e, -20.f, 20.f);
				ImGui::ColorEdit3("Color", m_Sphere.Color.e);
				ImGui::EndTabItem();
			}
			ImGui::EndTabBar();
		}
		ImGui::End();
	}

	void OnEvent(Magnefu::Event& e) 
	{

	}

private:
	Magnefu::Ref<Magnefu::SceneCamera> m_Camera;
	Magnefu::PrimitiveData m_Plane;
	Magnefu::PrimitiveData m_Cube;
	Magnefu::PrimitiveData m_RecPrism;
	Magnefu::SphereData m_Sphere;
};


class Sandbox : public Magnefu::Application
{
public:
	Sandbox();
	~Sandbox();

};