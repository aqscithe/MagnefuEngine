#pragma once

// NOTE: A material is a shader and a bunch of uniforms.

#include "Magnefu/Renderer/Texture.h"
#include "Magnefu/Renderer/Shader.h"
#include "Magnefu/Renderer/Material.h"
#include "Magnefu/ResourceManagement/CacheableResource.h"
#include "Magnefu/Renderer/Uniform.h"

#include "Magnefu/Core/Maths/MathsCommon.h"
#include <string>


namespace Magnefu
{
	//using TextureList = std::vector<std::pair<TextureType, String>>;


	class OpenGLMaterial : public Material, public CacheableResource
	{
	public:
		OpenGLMaterial(const String& shaderFile = "res/shaders/Basic.shader");

		void Bind() override;
		void Unbind() override;

		/*void InitRenderData(const Ref<SceneData>&) override;

		Ref<SceneData>& GetRenderData() override { return m_RenderData; }*/

		void OnImGuiRender() override;

		void SetUniformValueImpl(const std::string& name, const int& value) override;
		void SetUniformValueImpl(const std::string& name, const int* value) override;
		void SetUniformValueImpl(const std::string& name, const float& value) override;
		void SetUniformValueImpl(const std::string& name, const Maths::vec2& value) override;
		void SetUniformValueImpl(const std::string& name, const Maths::vec3& value) override;
		void SetUniformValueImpl(const std::string& name, const Maths::vec4& value) override;
		void SetUniformValueImpl(const std::string& name, const Maths::mat4& value) override;

		//void NullifyTextures();

	private:
		void InitUniforms();
		
	private: 
		std::unordered_map<String, Uniform> m_Uniforms;
		TextureMap  m_TextureMap;
		String		m_Name;
		String		m_Library;
		Ref<Shader> m_Shader;
		uint32_t m_ID;
	};


	/*struct MaterialData
	{
		String Library;
		String Name;
		Material Props;
	};*/
}
