#pragma once

#include "Core.h"

struct GLFWwindow;

class Camera
{
public:
	Maths::vec3 m_Position;
	Maths::vec3 m_Target;
	Maths::vec3 m_Forward;
	float m_Speed = 0.5f;
	
private:
	
	Maths::vec3 m_Up;
	Maths::vec3 m_Right;
	Maths::mat4 m_View;

	

public:
	Camera();
	Camera(const Maths::vec3& position, const Maths::vec3& target = { 0.f, 0.f, 0.f });

	void CalculateView();

	void ProcessInput(GLFWwindow* window, float deltaTime);

	inline const Maths::mat4& GetView() const { return m_View; }
	inline const Maths::vec3& GetRight() const { return m_Right; }
	inline const Maths::vec3& GetUp() const { return m_Up; }
	inline const Maths::vec3& GetForward() const { return m_Forward; }

private:
	void CreateCameraAxes();

	void SetPostion(Maths::vec3& position);

};