#pragma once

#include <vector>
#include <string>
#include <memory>

#include "Mesh.h"
#include "Shader.h"



class Model
{
	using Cache = std::unordered_map<std::string, int>;

public:
	Model(std::string& filepath, Cache& matCache);

	void Draw(std::unique_ptr<Shader>& shader);

	void Init(std::unique_ptr<Shader>& shader, Cache& textureCache, Cache& materialCache);

	std::string m_Filepath;

	std::vector<std::unique_ptr<Mesh>> m_Meshes;

	std::vector<MaterialData>            m_MaterialList;
	std::unordered_map<std::string, int> m_MaterialIndices;

	std::vector<TextureLabel> m_Textures;
	std::unordered_map<std::string, std::shared_ptr<Texture>> m_TextureCache;

private:
	void ParseMaterial(const std::string& filepath, std::vector<SubMaterialStream>& ss);
	Material<std::shared_ptr<Texture>> CreateMaterial(const std::string& matFile, const std::string& matData, const std::string& matName, unsigned int matID);

	Maths::vec3 GetVertexData(std::string& line, int elementCount);
	std::array<Maths::vec3i, 4> GetFaceData(std::string& line);

	void LoadMesh(std::vector<Maths::vec3>& tempPositions, std::vector<Maths::vec3>& tempNormals, std::vector<Maths::vec2>& tempTexCoords, std::vector<Face>& tempFaces);

};