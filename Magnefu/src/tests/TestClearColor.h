#pragma once

#include "Test.h"
#include "Vectors.h"

namespace Magnefu
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