#pragma once

#include "Test.h"
#include "Magnefu/Renderer/Renderer.h"

#include "Magnefu/Core/Maths/Primitive2D.h"

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
		Plane m_Plane;
	};
}
