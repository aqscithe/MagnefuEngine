#include "Mesh.h"

#include <fstream>
#include <sstream>


Mesh::Mesh(const std::string& filepath)
    : m_Filepath(filepath)
{
    std::ifstream stream(m_Filepath);
    std::string line;
    

    while (std::getline(stream, line))
    {
        if (line.find("v ") == 0)
            m_Positions.emplace_back(GetVertexData(line, 3));
        else if (line.find("vn ") == 0)
            m_Normals.emplace_back(GetVertexData(line, 3));
        else if (line.find("vt ") == 0)
            m_TexCoords.emplace_back(GetVertexData(line, 2).uv);
        else if (line.find("f ") == 0)
            m_Faces.emplace_back(GetFaceData(line));
    }

    std::cout << "Vertex Postions: " << m_Positions.size() << " | " << "Vertex Normals: " << m_Normals.size() << " | " << "Texture Coords: " <<
        m_TexCoords.size() << " | " << "Faces: " << m_Faces.size() << std::endl;
}

Maths::vec3 Mesh::GetVertexData(std::string& line, int elementCount)
{
    std::string vertexLine = line.substr(3);
    Maths::vec3 v;

    std::stringstream ss;
    ss.str(vertexLine);

    int index = 0;
    std::string del;
    while (std::getline(ss, del, ' '))
    {
        v.e[index] = std::stof(del);
        index++;
    }
    return v;
}

Face Mesh::GetFaceData(std::string& line)
{
    std::string faceLine = line.substr(2);
    // maybe i shoudl make a 3x4 matrix
    int faceData[12];

    std::stringstream ss;
    ss.str(faceLine);

    int vertexIndex = 0;
    std::string del;
    while (std::getline(ss, del, ' '))
    {
        int elementIndex = 0;
        std::stringstream ssB(del);
        std::string delB;
        while (std::getline(ssB, delB, '/'))
        {
            faceData[vertexIndex * 3 + elementIndex] = std::stoi(delB);
            elementIndex++;
        }
        vertexIndex++;
    }

    return {faceData[0], faceData[3], faceData[6], faceData[9] };

}

