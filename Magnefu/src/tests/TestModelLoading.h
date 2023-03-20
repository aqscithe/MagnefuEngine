#pragma once

#include "Test.h"

#include <GLAD/glad.h>
#include "Renderer.h"
#include "Texture.h"
#include "Shader.h"
#include "Camera.h"

#include "Light.h"
#include "Material.h"
#include "Model.h"
#include "Mesh.h"

#include "Quaternion.h"




// TODO:
// Implement ability to have multiple of each light type
// Probably std::vector so num of lights is mutable

namespace Magnefu
{
	template <typename T>
	struct Worker
	{
		bool WasAccessed;
		std::future<std::unique_ptr<T>> Thread;
	};

	class TestModelLoading : public Test
	{
		public:
			TestModelLoading();
			~TestModelLoading();

			void OnUpdate(GLFWwindow* window, float deltaTime) override;
			void OnHandleThreads();
			void OnRender()                override;
			void OnImGUIRender()           override;

		private:
			void UpdateLights();
			void UpdateMVP();
			void SetShaderUniforms();

			char* buf = (char*)malloc(sizeof(char) * 0);

			Maths::vec3 m_lightScaling;

			std::vector<PointLightModel> m_PointLights;
			std::vector<DirLightModel> m_DirectionLights;
			std::vector<SpotLightModel> m_SpotLights;

			std::vector<ObjModelVertex> m_TempVertices;
			std::vector<unsigned int>   m_TempIndices;
			std::unique_ptr<Mesh> m_Mesh;

			// Containers for scene worker threads

			uint32_t m_InactiveThreads = 0;
			std::unordered_map<uint32_t, Worker<Model>> m_ModelWorkers;
			std::vector<std::unique_ptr<Model>> m_Models;

			std::mutex m_ModelMutex;

			std::vector<std::string> m_Objs;


			std::unordered_map<std::string, int> m_TextureCache;
			std::unordered_map<std::string, int> m_MaterialCache;

			std::string m_MatFilePath;

			Renderer m_Renderer;
			
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

			int m_ReflectionModel;
			int m_ShadingTechnique;

			float m_RadiantFlux;
			float m_Reflectance;
	};
}