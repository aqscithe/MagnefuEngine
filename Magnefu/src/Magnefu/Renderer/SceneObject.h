#pragma once
#include "Magnefu/ResourceManagement/Handle.h"
#include "Magnefu/Core/Span.h"
#include "Magnefu/Renderer/BindGroup.h"



namespace Magnefu
{
	class SceneObject
	{
	public:
		SceneObject() = default;
		~SceneObject();

		void Init(uint32_t index);

		inline Handle<Shader>& GetGraphicsPipelineShaderHandle() { return m_GraphicsPipelineShader; }

		inline void SetVertexBlock(DataBlock&& vertexBlock) { m_Vertices = vertexBlock; }
		inline void SetIndexBlock(DataBlock&& indexBlock) { m_Indices = indexBlock; }
		

	private:
		Handle<BindGroup> m_Material;
		Handle<Buffer>    m_VertexBuffer;
		Handle<Buffer>    m_IndexBuffer;
		Handle<Shader>    m_GraphicsPipelineShader;
		DataBlock         m_Vertices;
		DataBlock         m_Indices;

		//BindGroupDesc     m_MaterialDesc;
	};
}