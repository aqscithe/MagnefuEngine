#pragma once

#include <vector>
#include <string>
#include <functional>
#include <utility>

struct GLFWwindow;

namespace Magnefu
{
	class Test
	{
	public:
		Test() {}
		virtual ~Test() {}

		virtual void OnUpdate(GLFWwindow* window, float deltaTime) {}
		virtual void OnRender() {}
		virtual void OnImGUIRender() {}
	};

	// contain a collection of all our tests
	class TestMenu : public Test
	{
	public:
		TestMenu(Test*& activeTestPtr);
		~TestMenu() {}

		void OnImGUIRender() override;
		
		template<typename T>
		void RegisterTest(const std::string& name)
		{
			m_Tests.push_back(std::make_pair(name, []() { return new T(); }));
		}

	private:
		Test*& m_ActiveTest;
		std::vector<std::pair<std::string, std::function<Test*()>>> m_Tests;
	};
}