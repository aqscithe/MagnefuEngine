#pragma once
#include "Magnefu/ResourceManagement/Handle.h"
#include "Magnefu/Core/Span.h"
#include "Magnefu/Renderer/BindGroup.h"
#include "Magnefu/Renderer/Material.h"



namespace Magnefu
{
	class SceneObject
	{
	public:
		SceneObject() = default;
		~SceneObject();

		void Init(uint32_t index);

		inline Handle<Shader>& GetGraphicsPipelineShaderHandle() { return m_GraphicsPipelineShader; }
		inline Handle<BindGroup>& GetMaterialBindGroup() { return m_Material; }
		inline Handle<Buffer>& GetVertexBufferHandle() { return m_VertexBuffer; }
		inline Handle<Buffer>& GetIndexBufferHandle() { return m_IndexBuffer; }
		inline uint32_t GetIndexCount() { return m_IndexCount; }

		inline Material& GetMaterialData() { return m_MaterialData; }

		inline void SetVertexBlock(DataBlock&& vertexBlock) { m_Vertices = std::move(vertexBlock); }
		inline void SetIndexBlock(DataBlock&& indexBlock) { m_Indices = std::move(indexBlock); m_IndexCount = static_cast<uint32_t>(m_Indices.data.size()) / sizeof(uint32_t); }

		inline void SetMaterialData(Material& materialData) { m_MaterialData = materialData; }

		//void UpdateUniformBuffer();
		DataBlock         m_Vertices;

	private:
		Handle<BindGroup> m_Material;
		Handle<Buffer>    m_VertexBuffer;
		Handle<Buffer>    m_IndexBuffer;
		Handle<Shader>    m_GraphicsPipelineShader;
		
		DataBlock         m_Indices;
		uint32_t          m_IndexCount;

		Material m_MaterialData;

	};
}