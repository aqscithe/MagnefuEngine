#pragma once

#include "Texture.h"
#include "Shader.h"


namespace Magnefu
{
	class IndexBuffer;
	class VertexArray;

	enum class RendererAPI
	{
		NONE = 0,
		OPENGL
	};

	class Renderer
	{

	private:

	public:

		void Draw(const std::shared_ptr<VertexArray>&) const;
		void DrawQuads(const VertexArray&, const IndexBuffer*, const Shader&) const;
		void DrawCube(const VertexArray&, const Shader&) const;
		void Clear() const;
		void ClearColor(const Maths::vec4& clearColor) const;
		void ClearColor(float r, float g, float b, float a) const;
		void EnableDepthTest() const;
		void DisableDepthTest() const;

		
		inline static RendererAPI GetAPI() { return s_RendererAPI; }
	private:

		static RendererAPI s_RendererAPI;
	};
}
