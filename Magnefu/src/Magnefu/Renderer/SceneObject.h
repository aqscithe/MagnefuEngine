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
		inline const size_t GetVerticesSize() { return m_Vertices.span.GetSize(); }
		inline const size_t GetIndicesSize() { return m_Indices.span.GetSize(); }
		inline const uint8_t* GetVerticesData() { return m_Vertices.span.GetData(); }
		inline const uint8_t* GetIndicesData() { return m_Indices.span.GetData(); }
		TextureDataBlock& GetTextureData(TextureType type);

		inline Material& GetMaterialData() { return m_MaterialData; }

		inline void SetVertexBlock(DataBlock&& vertexBlock) { m_Vertices = std::move(vertexBlock); }
		inline void SetIndexBlock(DataBlock&& indexBlock) { m_Indices = std::move(indexBlock); m_IndexCount = static_cast<uint32_t>(m_Indices.data.size()) / sizeof(uint32_t); }

		void SetTextureBlock(TextureType type, DataBlock&& dataBlock, int width, int height, int channels);

		inline void SetMaterialData(Material& materialData) { m_MaterialData = materialData; }

		inline void ClearIndexDataBlock() { m_Indices.data.clear(); m_Indices.data.shrink_to_fit(); }
		inline void ClearVertexDataBlock() { m_Vertices.data.clear(); m_Vertices.data.shrink_to_fit(); }

		

	private:
		Handle<BindGroup> m_Material;
		Handle<Buffer>    m_VertexBuffer;
		Handle<Buffer>    m_IndexBuffer;
		Handle<Shader>    m_GraphicsPipelineShader;
		
		DataBlock         m_Vertices;
		DataBlock         m_Indices;
		uint32_t          m_IndexCount;         

		Material m_MaterialData;


		TextureDataBlock    m_DiffuseTextureBlock;
		TextureDataBlock    m_ARMTextureBlock;
		TextureDataBlock    m_NormalTextureBlock;

	};
}