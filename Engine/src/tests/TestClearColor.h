#pragma once

#include "Test.h"
#include "Core.h"

namespace test
{

	class TestClearColor : public Test
	{
	public:
		TestClearColor();
		~TestClearColor();

		void OnUpdate(GLFWwindow* window, float deltaTime) override;
		void OnRender()                override;
		void OnImGUIRender()           override;

	private:
		Maths::vec4 m_ClearColor;

	};
}