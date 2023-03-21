#include "mfpch.h"

#include "Mesh.h"
#include "VertexBufferAttribsLayout.h"



static std::vector<unsigned int> SetIndices(std::vector<Face>& faces)
{
    std::unordered_map<uint8_t, std::vector<uint8_t>> IndexPattern;
    IndexPattern[3] = { 0, 1, 2 };
    IndexPattern[4] = { 0, 1, 2, 2, 3, 0 };

    //std::unordered_map<uint32_t, uint32_t> vertexCounts;
    //vertexCounts[3] = 0;
    //vertexCounts[4] = 0;


    std::vector<unsigned int> indices;
    for (int faceCount = 0; faceCount < faces.size(); faceCount++)
    {
        uint8_t vertexCount = faces[faceCount].VertexCount;
        uint8_t patternLength = IndexPattern[vertexCount].size();
        for (int i = 0; i < patternLength; i++)
        {
            // only works if all the faces have the same vertex count
            // need to modify to be more dynamic
            // probably keep track of the final index value for each face
            indices.push_back(IndexPattern[vertexCount][i] + vertexCount * faceCount);
            //uint32_t num = 6 * vertexCounts[4] + 3 * vertexCounts[3];
            //indices.push_back(IndexPattern[vertexCount][i] + num); 
            //vertexCounts[vertexCount]++;
        }
    }
    return indices;
}

Mesh::Mesh(MeshData& meshData) : m_MeshData(meshData) 
{
    MF_CORE_DEBUG(
        "MESH -- Vertex Postions: {0} | Vertex Normals: {1} | Texture Coords: {2} | Faces: {3}", 
        m_MeshData.Positions.size(), m_MeshData.Normals.size(), m_MeshData.TexCoords.size(), m_MeshData.Faces.size()
    );
}

void Mesh::Init()
{
    std::vector<ObjModelVertex> vertices;

    // CHANGES
    // Only declare indices.
    // Then in the Faces for loop, check the vertex count and set the indices based on that.
    // then append those indices to the array.
    std::vector<unsigned int> indices = SetIndices(m_MeshData.Faces);

    for (Face& face : m_MeshData.Faces)
    {
        int i = 1;
        for (Maths::vec3i& Index : face.Indices)
        {
            if (i > face.VertexCount) break;
            vertices.emplace_back(m_MeshData.Positions[Index.v], m_MeshData.Normals[Index.vn], m_MeshData.TexCoords[Index.vt], face.MatID);
            i++;
        }
            
    }

    m_VBO = std::make_unique<VertexBuffer>(sizeof(ObjModelVertex) * vertices.size(), vertices.data());

    // Setting Index buffer data fails due to memory access violation unless I do this...
    m_IBO = std::make_unique<IndexBuffer>(sizeof(unsigned int) * indices.size(), nullptr);

    m_IBO->Bind();
    glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, sizeof(unsigned int) * indices.size(), indices.data());

    VertexBufferAttribsLayout layout;
    layout.Push<float>(3);
    layout.Push<float>(3);
    layout.Push<float>(2);
    layout.Push<unsigned int>(1);

    m_VAO = std::make_unique<VertexArray>();
    m_VAO->AddBuffer(*m_VBO, layout);
}

void Mesh::Draw(Shader* shader)
{
    m_Renderer.Draw(*m_VAO, *m_IBO, *shader);
}

void Mesh::OnImGUIRender()
{

}



