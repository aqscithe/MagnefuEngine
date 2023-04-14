#pragma once

#include "Magnefu/Renderer/Scene.h"


namespace Magnefu
{
	using String = std::string;

	class Shader
	{
	public:
		virtual ~Shader() = default;

		virtual void Bind() const = 0;
		virtual void Unbind() const = 0;

		virtual void SetUniformMatrix4fv(const String& name, const Maths::mat4& value) = 0;
		virtual void SetUniform4fv(const String& name, const Maths::vec4& value) = 0;
		virtual void SetUniform3fv(const String& name, const Maths::vec3& value) = 0;
		virtual void SetUniform1f(const String& name, const float value) = 0;
		virtual void SetUniform1i(const String& name, const int value) = 0;
		virtual void SetUniform1i(const String& name, const bool value) = 0;
		virtual void SetUniform1iv(const String& name, const int* value) = 0;
		virtual void SetUniform1ui(const String& name, const unsigned int value) = 0;

		virtual void UploadUniforms(const Ref<SceneData>& data) = 0;

		virtual void OnImGuiRender() = 0;
		virtual void Recompile() = 0;

		virtual const String& GetFilepath() const = 0;

		static Ref<Shader> Create(const String& filepath = "res/shaders/Basic.shader");
	};

	class ShaderLibrary
	{
	public:
		static Ref<Shader> Get(const String& name);
	};
}