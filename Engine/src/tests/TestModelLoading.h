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
#include "Light.h"
#include "Material.h"
#include "Mesh.h"

#include "Quaternion.h"
#include "Vectors.h"
#include "Matrices.h"

#include <memory>
#include <vector>
#include <future>


// TODO:
// Implement ability to have multiple of each light type
// Probably std::vector so num of lights is mutable

namespace test
{

	class TestModelLoading : public Test
	{
		public:
			TestModelLoading();
			~TestModelLoading();

			void OnUpdate(GLFWwindow* window, float deltaTime) override;
			void OnRender()                override;
			void OnImGUIRender()           override;

		private:
			void UpdateLights();
			void UpdateMVP();
			void SetTextureShaderUniforms();

			Maths::vec3 m_lightScaling;

			std::vector<PointLightModel> m_PointLights;
			std::vector<DirLightModel> m_DirectionLights;
			std::vector<SpotLightModel> m_SpotLights;


			Maths::vec3 m_AmbientIntensity;
			Maths::vec3 m_DiffusionIntensity;
			Maths::vec3 m_SpecularIntensity;

			std::vector<ObjModelVertex> m_TempVertices;
			std::vector<unsigned int>   m_TempIndices;
			std::unique_ptr<Mesh> m_Mesh;
			std::future<void > m_Future;
			bool m_bFutureAccessed;

			std::vector<std::unique_ptr<Texture>> m_Textures;
			Material* m_ActiveMaterial;
			std::unordered_map<std::string, Material> m_AvailableMaterials;

			Renderer m_Renderer;
			std::unique_ptr<VertexBuffer> m_VBO;
			std::unique_ptr<VertexArray> m_VAO;
			std::unique_ptr<IndexBuffer> m_IBO;
			std::unique_ptr<Shader> m_Shader;
			std::unique_ptr<Shader> m_LightCubeShader;


			std::unique_ptr <Maths::Quaternion> m_Quat;
			float m_angleRot = 0.f;
			Maths::vec3 m_translation;
			Maths::vec3 m_rotationAxis;
			Maths::vec3 m_scaling;

			Maths::mat4 m_MVP;
			std::unique_ptr<Camera> m_Camera;


			bool m_bShowTransform;

			ReflectionModel m_ReflectionModel;
			ShadingTechnique m_ShadingTechnique;


			
	};
}