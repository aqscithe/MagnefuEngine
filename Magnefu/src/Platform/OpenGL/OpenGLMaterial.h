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
		OpenGLMaterial(const String& shaderFile, const MaterialOptions& options);

		void SetUniformValueImpl(const std::string& name, const bool& value) override;
		void SetUniformValueImpl(const std::string& name, const int& value) override;
		void SetUniformValueImpl(const std::string& name, const int* value) override;
		void SetUniformValueImpl(const std::string& name, const float& value) override;
		void SetUniformValueImpl(const std::string& name, const Maths::vec2& value) override;
		void SetUniformValueImpl(const std::string& name, const Maths::vec3& value) override;
		void SetUniformValueImpl(const std::string& name, const Maths::vec4& value) override;
		void SetUniformValueImpl(const std::string& name, const Maths::mat4& value) override;

		void Bind() override;
		void Unbind() override;

		void OnImGuiRender() override;

		void UpdateMaterialSpec() override;

		inline const MaterialSpec& GetMaterialSpec() const override { return m_Spec; }
		inline const Ref<VertexArray>& GetVertexArray() const override { return m_VertexArray; }

		void AddVertexBuffer(const Ref<VertexBuffer>& vbo) override { m_VertexArray->AddVertexBuffer(vbo); }
		void SetIndexBuffer(const Ref<IndexBuffer>& ibo) override { m_VertexArray->SetIndexBuffer(ibo); }

	private:
		void InitUniforms();
		
	private: 
		std::unordered_map<String, Uniform> m_Uniforms;
		MaterialSpec m_Spec;
		String		m_Name;
		String		m_Library;
		Ref<Shader> m_Shader;
		Ref<VertexArray> m_VertexArray;
		uint32_t m_ID;
		MaterialOptions m_Options;
	};


	/*struct MaterialData
	{
		String Library;
		String Name;
		Material Props;
	};*/
}
