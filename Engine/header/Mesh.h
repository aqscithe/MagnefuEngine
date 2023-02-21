#pragma once

#include "PrimitiveCommon.h"

#include "Material.h"

#include <vector>
#include <string>
#include <map>
#include <unordered_map>
#include <fstream>
#include <sstream>


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
	BUMP
};

struct TextureLabel
{
	std::string Name;
	TextureType Type;
};

class Mesh
{
public:
	Mesh(const std::string& filepath, std::vector<MaterialData>& materialLibs);
	
	std::vector<Maths::vec3> m_Positions;
	std::vector<Maths::vec3> m_Normals;
	std::vector<Maths::vec2> m_TexCoords;
	std::vector<Face> m_Faces;
	std::vector<std::string> m_TexIDs;

	std::string m_Filepath;

	std::unordered_map<std::string, int> m_MaterialIndices;
	std::vector<TextureLabel> m_Textures;
	std::unordered_map<std::string, std::shared_ptr<Texture>> m_TextureCache;
	

private:

	Maths::vec3 GetVertexData(std::string& line, int elementCount);
	std::array<Maths::vec3i, 4>        GetFaceData(std::string& line);
	void ParseMaterial(const std::string& filepath, std::vector<SubMaterialStream>& ss);
	Material<std::shared_ptr<Texture>> CreateMaterial(const std::string& matStream, const std::string& matName);
};