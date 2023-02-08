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
#include <vector>

namespace test
{
	struct Light
	{
		float K_a;
		float K_d;
		float K_s;
		Maths::vec3 Position;
		Maths::vec4 Ambient;
		Maths::vec4 Diffuse;
		Maths::vec4 Specular;
	};

	enum class LightModel
	{
		PHONG,
		GORAUD
	};

	struct Material
	{
		bool Preset;
		Maths::vec3 Ambient;
		Maths::vec3 Diffuse;
		Maths::vec3 Specular;
		float       Shininess;
		//float       Roughness;
		//float       Opacity;
	};

	class TestLighting : public Test
	{
		public:
			TestLighting();
			~TestLighting();

			void OnUpdate(GLFWwindow* window, float deltaTime) override;
			void OnRender()                override;
			void OnImGUIRender()           override;

		private:
			void SetShaderUniforms();

			Light m_light;
			Maths::vec3 m_lightScaling;

			Material* m_ActiveMaterial;
			std::unordered_map<std::string, Material> m_AvailableMaterials;

			float m_shininess;

			int   m_LightModel;

			Renderer m_Renderer;
			std::unique_ptr<VertexBuffer> m_VBO;
			std::unique_ptr<VertexArray> m_ModelCubeVAO;
			std::unique_ptr<VertexArray> m_LightCubeVAO;
			std::unique_ptr<IndexBuffer> m_IBO;
			std::unique_ptr<Shader> m_ModelCubeShader;
			std::unique_ptr<Shader> m_LightCubeShader;


			std::unique_ptr <Maths::Quaternion> m_Quat;
			float m_angleRot = 0.f;
			Maths::vec3 m_translation;
			Maths::vec3 m_rotationAxis;
			Maths::vec3 m_scaling;

			std::unique_ptr<Camera> m_Camera;

			Maths::vec3 m_ObjectColor;


			// put this in a camera info struct that all
			// these test classes can access
			float m_aspectRatio;
			float m_near;
			float m_far;
			float m_top;
			float m_bottom;
			float m_right;
			float m_left;
			bool m_IsOrtho;

			Maths::mat4 m_MVP;

			int m_cubeCount;

			bool m_bShowTransform;


			
	};
}