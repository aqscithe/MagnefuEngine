#pragma once

#include "Test.h"

#include <GL/glew.h>
#include "Renderer.h"
#include "Texture.h"
#include "Shader.h"
#include "Quaternion.h"
#include "Camera.h"
#include "VertexArray.h"
#include "IndexBuffer.h"

#include "Quaternion.h"
#include "Vectors.h"
#include "Matrices.h"

#include <memory>

namespace test
{

	class TestBatchRendering : public Test
	{
		public:
			TestBatchRendering();
			~TestBatchRendering();

			void OnUpdate(GLFWwindow* window, float deltaTime) override;
			void OnRender()                override;
			void OnImGUIRender()           override;

		private:
			Renderer m_Renderer;
			std::unique_ptr<VertexBuffer> m_VBO;
			std::unique_ptr<VertexArray> m_VAO;
			std::unique_ptr<IndexBuffer> m_IBO;
			std::unique_ptr<Shader> m_Shader;
			std::unique_ptr<Texture> m_Texture0;
			std::unique_ptr<Texture> m_Texture1;


			std::unique_ptr <Maths::Quaternion> m_Quat;
			float m_angleRot = 0.f;
			Maths::vec3 m_translation;
			Maths::vec3 m_rotationAxis;
			Maths::vec3 m_scaling;

			std::unique_ptr<Camera> m_Camera;

			float m_aspectRatio;
			float m_near;
			float m_far;
			float m_top;
			float m_bottom;
			float m_right;
			float m_left;
			bool m_IsOrtho;

			Maths::mat4 m_MVP;

			int m_QuadCount;


			
	};
}