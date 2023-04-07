#pragma once

#include "Test.h"
#include "Magnefu/Renderer/Renderer.h"
#include "Magnefu/Renderer/VertexArray.h"
#include "Magnefu/Renderer/Buffer.h"
#include "Magnefu/Renderer/Texture.h"
#include "Magnefu/Renderer/Shader.h"
#include "Magnefu/Scene/Camera.h"

#include "Quaternion.h"
#include "Magnefu/Renderer/Material.h"

#include "Magnefu/Scene/Scene.h"

#include "GLAD/glad.h"



namespace Magnefu
{
	class Test2DTexture : public Test
	{
		public:
			Test2DTexture();
			~Test2DTexture();

			void OnUpdate(float deltaTime) override;
			void OnRender(float renderInterpCoeff) override;
			void OnImGUIRender()           override;

		private:
			Renderer m_Renderer;
			Ref<VertexArray> m_VAO;
			Ref<Material> m_Material;


			Scope <Maths::Quaternion> m_Quat;
			float m_angleRot = 0.f;
			Maths::vec3 m_translation;
			Maths::vec3 m_rotationAxis;
			Maths::vec3 m_scaling;

			Ref<Camera> m_SceneCamera;
			Ref<SceneData> m_RenderData;
			Scope<SceneData> m_SceneData;
			SceneData m_PrevSceneData;
	};
}