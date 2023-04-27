#pragma once

#include "Magnefu/Core/SmartPointers.h"
#include "Magnefu/Core/Maths/MathsCommon.h"
#include "Texture.h"
#include "Magnefu/Renderer/Scene.h"


namespace Magnefu
{
	using String = std::string;
	using TextureMap = std::unordered_map<TextureType, Ref<Texture>>;

	enum MaterialOptions {
		MaterialOptions_None = 0,
		MaterialOptions_Skybox = BIT(0),
		MaterialOptions_All = 1
	};

	struct MaterialSpec
	{
		TextureMap  TextureMap;
		Maths::vec3 Ka;
		Maths::vec3 Kd;
		Maths::vec3 Ks;
		Maths::vec3 TintColor;
		float Reflectance;  // [0.0, 1.0] - Dielectrics
		float Opacity;
		//subsurface scattering variable;
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

		virtual void OnImGuiRender() = 0;

		virtual const MaterialSpec& GetMaterialSpec() const = 0;

		static Ref<Material> Create(const String& shaderPath = "res/shaders/Basic.shader", const MaterialOptions& options = MaterialOptions::MaterialOptions_None);
	};
}