#pragma once

#include "Magnefu/Core/SmartPointers.h"
#include "Magnefu/Core/Maths/MathsCommon.h"
#include "Texture.h"
#include "Magnefu/Renderer/Scene.h"


namespace Magnefu
{
	using String = std::string;
	using TextureMap = std::unordered_map<TextureType, Ref<Texture>>;


	// all values that would be sent to the shader as a uniform
	struct MaterialProps
	{
		MaterialProps()
		{
			Name = "BlackHole";
			Library = "Universe";
			ID = 999;
			TextureMap[TextureType::DIFFUSE] = Texture::Create();
			Ka = { 0.01f, 0.01f, 0.01f};
			Kd = { 1.f, 1.f, 1.f};
			Ks = { 0.5f, 0.5f, 0.5f};
			Ni = 1.52f;
			Ns = 10.f;
			Opacity = 1.f;
		}

		String		Name;
		String		Library;
		uint32_t    ID;
		
		Maths::vec3 Ka;
		Maths::vec3 Kd;
		Maths::vec3 Ks;
		float Ni;
		float Ns;
		float Opacity;
		TextureMap  TextureMap;

	};

	class Material
	{
	public:
		virtual ~Material() = default;

		template <typename T>
		void SetUniformValue(const String& name, const T& value)
		{
			MF_CORE_ASSERT((std::is_same<T, int>::value || std::is_same<T, float>::value || std::is_same<T, bool>::value || 
				std::is_same<T, int*>::value || std::is_same<T, Maths::vec2>::value || std::is_same<T, Maths::vec3>::value || 
				std::is_same<T, Maths::vec4>::value || std::is_same<T, Maths::mat4>::value),
				"Unsupported uniform type."
			);

			SetUniformValueImpl(name, value);
		}

		// Pure virtual function for setting uniform values
		virtual void SetUniformValueImpl(const std::string& name, const bool& value) = 0;
		virtual void SetUniformValueImpl(const std::string& name, const int& value) = 0;
		virtual void SetUniformValueImpl(const std::string& name, const int* value) = 0;
		virtual void SetUniformValueImpl(const std::string& name, const float& value) = 0;
		virtual void SetUniformValueImpl(const std::string& name, const Maths::vec2& value) = 0;
		virtual void SetUniformValueImpl(const std::string& name, const Maths::vec3& value) = 0;
		virtual void SetUniformValueImpl(const std::string& name, const Maths::vec4& value) = 0;
		virtual void SetUniformValueImpl(const std::string& name, const Maths::mat4& value) = 0;

		virtual void Bind() = 0;
		virtual void Unbind() = 0;

		//virtual Ref<SceneData>& GetRenderData() = 0;
		virtual void OnImGuiRender() = 0;
		//virtual void InitRenderData(const Ref<SceneData>&) = 0;

		static Ref<Material> Create(const String& shaderPath = "res/shaders/Basic.shader");
	};
}