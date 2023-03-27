#pragma once

#include "Test.h"
#include "Magnefu/Renderer/Renderer.h"
#include "Magnefu/Renderer/VertexArray.h"
#include "Magnefu/Renderer/Buffer.h"
#include "Texture.h"
#include "Shader.h"
#include "Camera.h"


#include "Quaternion.h"

#include <GLAD/glad.h>




namespace Magnefu
{

	class Test2DTexture : public Test
	{
		public:
			Test2DTexture();
			~Test2DTexture();

			void OnUpdate(float deltaTime) override;
			void OnRender()                override;
			void OnImGUIRender()           override;

		private:
			Renderer m_Renderer;
			Ref<VertexArray> m_VAO;
			Shader* m_Shader;
			Texture* m_Texture;


			Scope <Maths::Quaternion> m_Quat;
			float m_angleRot = 0.f;
			Maths::vec3 m_translation;
			Maths::vec3 m_rotationAxis;
			Maths::vec3 m_scaling;

			Scope<Camera> m_Camera;

			float m_aspectRatio;
			float m_near;
			float m_far;
			float m_top;
			float m_bottom;
			float m_right;
			float m_left;
			bool m_IsOrtho;

			Maths::mat4 m_MVP;


			
	};
}