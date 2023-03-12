#pragma once

#include "PrimitiveCommon.h"

#include "VertexBuffer.h"
#include "VertexArray.h"
#include "IndexBuffer.h"
#include "Material.h"
#include "Shader.h"
#include "Renderer.h"

#include <vector>
#include <fstream>


struct SubMaterialStream
{
	std::string SubMatName;
	std::string StrData;
};

enum TextureType
{
	NONE = -1,
	AMBIENT,
	DIFFUSE,
	SPECULAR,
	EMISSIVE,
	BUMP,
	ROUGHNESS,
	METALLIC
};

struct TextureLabel
{
	std::string Name;
	TextureType Type;
};

struct MeshData
{
	std::vector<Maths::vec3> Positions;
	std::vector<Maths::vec3> Normals;
	std::vector<Maths::vec2> TexCoords;
	std::vector<Face> Faces;
};

class Mesh
{
public:
	Mesh(MeshData& meshData);

	void Draw(std::unique_ptr<Shader>& shader);
	void OnImGUIRender();

	void Init();

	
	MeshData m_MeshData;

	std::string m_Filepath;

	std::unique_ptr<VertexBuffer> m_VBO;
	std::unique_ptr<VertexArray> m_VAO;
	std::unique_ptr<IndexBuffer> m_IBO;

	Renderer m_Renderer;
	

private:
};