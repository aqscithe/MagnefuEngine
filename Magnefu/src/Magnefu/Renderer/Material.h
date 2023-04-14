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

		virtual void Bind() = 0;
		virtual void Unbind() = 0;

		virtual Ref<SceneData>& GetRenderData() = 0;
		virtual void OnImGuiRender() = 0;
		virtual void InitRenderData(const Ref<SceneData>&) = 0;

		static Ref<Material> Create(const String& shaderPath = "res/shaders/Basic.shader");
	};
}