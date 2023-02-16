#pragma once

#include "PrimitiveCommon.h"

#include <vector>


struct MeshSource
{

};

class Mesh
{
public:
	Mesh(const std::string& filepath);
	
	std::vector<Maths::vec3> m_Positions;
	std::vector<Maths::vec3> m_Normals;
	std::vector<Maths::vec2> m_TexCoords;
	std::vector<Face> m_Faces;
	//unsigned int m_MaterialIndex;

	std::string m_Filepath;

private:

	Maths::vec3 GetVertexData(std::string& line, int elementCount);
	Face        GetFaceData(std::string& line);
};