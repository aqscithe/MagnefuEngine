#include "mfpch.h"

#include "Test.h"

#include <GLAD/glad.h>
//#include "imgui/imgui.h"

namespace Magnefu
{
	TestMenu::TestMenu(Test*& activeTestPtr)
		: m_ActiveTest(activeTestPtr)
	{
	}

	void TestMenu::OnImGUIRender()
	{
		for (auto& test : m_Tests)
		{
			//if (ImGui::Button(test.first.c_str()))
			//	m_ActiveTest = test.second();
		}
	}

}


