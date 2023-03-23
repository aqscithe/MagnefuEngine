#pragma once

#include "Test.h"

#include <GLAD/glad.h>
#include "Renderer.h"
#include "Texture.h"
#include "Shader.h"
#include "Camera.h"
#include "VertexArray.h"
#include "IndexBuffer.h"
#include "Light.h"
#include "Material.h"

#include "Quaternion.h"


// TODO:
// Implement ability to have multiple of each light type
// Probably std::vector so num of lights is mutable

namespace Magnefu
{
	class TestLighting : public Test
	{
		public:
			TestLighting();
			~TestLighting();

			void OnUpdate(GLFWwindow* window, float deltaTime) override;
			void OnRender()                override;
			void OnImGUIRender()           override;

		private:
			void UpdateLights();
			void UpdateMVP();
			//void SetShaderUniforms();
			void SetTextureShaderUniforms();

			Maths::vec3 m_lightScaling;

			float m_RadiantFlux;

			float m_PointLightRadius;
			std::vector<PointLightModel> m_PointLights;
			std::vector<DirLightModel> m_DirectionLights;
			std::vector<SpotLightModel> m_SpotLights;



			float m_shininess;

			int m_ShadingTechnique;
			int m_ReflectionModel;

			float m_Reflectance;

			Renderer m_Renderer;
			std::unique_ptr<VertexBuffer> m_VBO;
			std::unique_ptr<VertexArray> m_ModelCubeVAO;
			std::unique_ptr<VertexArray> m_LightCubeVAO;
			std::unique_ptr<IndexBuffer> m_IBO;
			Shader* m_ModelCubeShader;
			Shader* m_LightCubeShader;
			Shader* m_TextureCubeShader;

			std::vector<Texture*> m_Textures;
			std::vector<std::future<void>> m_Futures;

			Material* m_ActiveMaterial;
			std::vector<Material*> m_Materials;


			std::unique_ptr <Maths::Quaternion> m_Quat;
			float m_angleRot = 0.f;
			Maths::vec3 m_translation;
			Maths::vec3 m_rotationAxis;
			Maths::vec3 m_scaling;

			std::unique_ptr<Camera> m_Camera;

			Maths::vec3 m_ObjectColor;


			Maths::mat4 m_MVP;

			int m_cubeCount;

			bool m_bShowTransform;


			
	};
}