#pragma once

#include "Test.h"
#include "Magnefu/Renderer/Renderer.h"

#include "Magnefu/Core/Maths/PrimitiveCommon.h"

#include "GLAD/glad.h"


namespace Magnefu
{
	class TestPrimitives : public Test
	{
	public:
		TestPrimitives();
		~TestPrimitives();

		void OnUpdate(float deltaTime) override;
		void OnRender(float renderInterpCoeff) override;
		void OnEvent(Event&) override;
		void OnImGUIRender() override;

	private:
		PrimitiveData m_Plane;
		PrimitiveData m_Cube;
		SphereData m_Sphere;
	};
}
