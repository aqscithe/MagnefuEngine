#include "Mesh.h"
#include "VertexBufferAttribsLayout.h"


static std::vector<unsigned int> SetIndices(int quadCount)
{
    int IndexPattern[6] = { 0, 1, 2, 2, 3, 0 };

    std::vector<unsigned int> indices;
    for (int quadNum = 0; quadNum < quadCount; quadNum++)
    {
        for (int i = 0; i < 6; i++)
            indices.push_back(IndexPattern[i] + 4 * quadNum);
    }
    return indices;
}

Mesh::Mesh(MeshData& meshData) : m_MeshData(meshData) 
{
    std::cout << "Mesh constructor called: " << std::endl;
    std::cout << "Vertex Postions: " << m_MeshData.Positions.size() << " | " << "Vertex Normals: " << m_MeshData.Normals.size() << " | " << "Texture Coords: " <<
        m_MeshData.TexCoords.size() << " | " << "Faces: " << m_MeshData.Faces.size() << std::endl;
}

void Mesh::Init()
{
    std::vector<ObjModelVertex> vertices;

    std::vector<unsigned int> indices = SetIndices(static_cast<int>(m_MeshData.Faces.size()));

    for (Face& face : m_MeshData.Faces)
    {
        int i = 1;
        for (Maths::vec3i& Index : face.Indices)
        {
            if (i > face.VertexCount) break;

            if (Index.v > m_MeshData.Positions.size() - 1 || Index.vn > m_MeshData.Normals.size() - 1 || Index.vt > m_MeshData.TexCoords.size() - 1)
            {
                std::cout << Index.v << "   " << Index.vn << "   " << Index.vt << std::endl;
            }
            vertices.emplace_back(m_MeshData.Positions[Index.v], m_MeshData.Normals[Index.vn], m_MeshData.TexCoords[Index.vt], face.MatID);
            i++;
        }
            
    }

    m_VBO = std::make_unique<VertexBuffer>(sizeof(ObjModelVertex) * vertices.size(), vertices.data());

    // Setting Index buffer data fails due to memory access violation unless I do this...
    m_IBO = std::make_unique<IndexBuffer>(sizeof(unsigned int) * indices.size(), nullptr);

    m_IBO->Bind();
    GLCall(glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, sizeof(unsigned int) * indices.size(), indices.data()));

    VertexBufferAttribsLayout layout;
    layout.Push<float>(3);
    layout.Push<float>(3);
    layout.Push<float>(2);
    layout.Push<unsigned int>(1);

    m_VAO = std::make_unique<VertexArray>();
    m_VAO->AddBuffer(*m_VBO, layout);

    std::cout << "Mesh Initialized " << m_MeshData.Faces.size() << std::endl;
}

void Mesh::Draw(std::unique_ptr<Shader>& shader)
{
    m_Renderer.Draw(*m_VAO, *m_IBO, *shader);
}

void Mesh::OnImGUIRender()
{

}



