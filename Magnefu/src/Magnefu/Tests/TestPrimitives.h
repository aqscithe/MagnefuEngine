#pragma once

#include "Test.h"
#include "Magnefu/Renderer/Renderer.h"
#include "Magnefu/Renderer/VertexArray.h"
#include "Magnefu/Renderer/Buffer.h"
#include "Magnefu/Renderer/Texture.h"
#include "Magnefu/Renderer/Shader.h"
#include "Magnefu/Renderer/Camera.h"

#include "Magnefu/Core/Maths/Quaternion.h"
#include "Magnefu/Renderer/Material.h"

#include "Magnefu/Renderer/Scene.h"

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
		Maths::vec2 m_PlaneSize;
		Maths::vec3 m_PlaneColor;
	};
}
