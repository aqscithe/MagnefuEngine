#include "Camera.h"
#include <GLFW/glfw3.h>
#include "Globals.h"

#include "Common.h"



Camera::Camera()
{
	m_Position = { 0.f, 0.f, 3.f };
	m_Target = { 0.f,0.f,0.f };
	CreateCameraAxes();
}

Camera::Camera(const Maths::vec3& position, const Maths::vec3& target)
	: m_Position(position), m_Target(target)
{
	CreateCameraAxes();
}

void Camera::CreateCameraAxes()
{
	m_Forward = Maths::normalize(m_Position - m_Target);
	m_Right = Maths::normalize(Maths::crossProduct(Maths::vec3(0.0f, 1.0f, 0.0f), m_Forward));
	m_Up = Maths::normalize(Maths::crossProduct(m_Forward, m_Right));
	CalculateView();
}

void Camera::SetPostion(Maths::vec3& position)
{
	m_Position = position;
}

void Camera::CalculateView()
{
	Globals& global = Globals::Get();
	m_View = Maths::rotateX(global.pitch) * Maths::rotateY(global.yaw) * Maths::axis(m_Right, m_Up, m_Forward) * Maths::translate(-m_Position);

	//Maths::vec3 direction;
	//direction.x = Maths::cos(Maths::toRadians(yaw)) * Maths::cos(Maths::toRadians(pitch));
	//direction.y = Maths::sin(Maths::toRadians(pitch));
	//direction.z = Maths::sin(Maths::toRadians(yaw)) * Maths::cos(Maths::toRadians(pitch));
	//camera.m_Forward = Maths::normalize(direction);
}

void Camera::ProcessInput(GLFWwindow* window, float deltaTime)
{

	Globals& global = Globals::Get();
	
	float cameraSpeed = m_Speed * deltaTime;
	float forwardMovement = 0.f;
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
		forwardMovement += cameraSpeed;
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
		forwardMovement -= cameraSpeed;


	float strafeMovement = 0.f;
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
		strafeMovement -= cameraSpeed;
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
		strafeMovement += cameraSpeed;

	float verticalMovement = 0.f;
	if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
		verticalMovement += cameraSpeed;
	if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
		verticalMovement -= cameraSpeed;


	m_Position.x += Maths::sin(global.yaw) * forwardMovement;
	m_Position.z -= Maths::cos(global.yaw) * forwardMovement;					   
	m_Position.x += Maths::cos(global.yaw) * strafeMovement;
	m_Position.z += Maths::sin(global.yaw) * strafeMovement;			   
	m_Position.y -= Maths::sin(global.pitch) * forwardMovement;
	m_Position.y += verticalMovement;


	/*
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		m_Position -= cameraSpeed * m_Forward;
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		m_Position += cameraSpeed * m_Forward;
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		m_Position += cameraSpeed * Maths::normalize(Maths::crossProduct(m_Forward, m_Up));
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		m_Position -= cameraSpeed * Maths::normalize(Maths::crossProduct(m_Forward, m_Up));*/


	

}
