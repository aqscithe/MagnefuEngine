#include "mfpch.h"

#include "Renderer.h"
#include "Buffer.h"
#include "VertexArray.h"
#include "Material.h"
#include "Shader.h"
#include "Magnefu/Core/Maths/Quaternion.h"
#include "Magnefu/Application.h"
#include "Magnefu/Core/Timer.h"
#include "Magnefu/Core/MemoryAllocation/LinkedListAlloc.h"


namespace Magnefu
{
    struct RenderData
    {
        Maths::mat4 MVP;
    };

    static RenderData* s_Data = nullptr;

    void Renderer::Init()
    {
        s_Data = static_cast<RenderData*>(StackAllocator::Get()->Allocate(sizeof(RenderData), sizeof(Maths::mat4)));
    }

    void Renderer::BeginScene()
    {
        
    }

    void Renderer::EndScene()
    {
    }

    void Renderer::Submit(const Ref<VertexArray>& va, const Ref<Material>& material )
    {
        // When a renderer binds a material, all the necessary uniforms from the renderer and the material
        // in question and upload the values in one contiguous buffer.
        material->Bind();
        RenderCommand::DrawIndexed(va);
    }

    void Renderer::DrawPlane(const PrimitiveData& data)
    {
        float vertices[28] = {
            //  position    color                                        texture coords
            -0.5f, -0.5f,   data.Color.r, data.Color.g, data.Color.b,    0.f,  0.f,       // 0  BL
             0.5f, -0.5f,   data.Color.r, data.Color.g, data.Color.b,    1.f,  0.f,       // 1  BR
             0.5f,  0.5f,   data.Color.r, data.Color.g, data.Color.b,    1.f,  1.f,       // 2  TR
            -0.5f,  0.5f,   data.Color.r, data.Color.g, data.Color.b,    0.f,  1.f,       // 3  TL
        };

        uint32_t indices[] = {
            0, 1, 2,
            2, 3, 0
        };

        Ref<VertexBuffer> vbo = VertexBuffer::Create(sizeof(float) * 28, vertices);

        BufferLayout layout = {
            {ShaderDataType::Float2, "aPosition"},
            {ShaderDataType::Float3, "aColor"},
            {ShaderDataType::Float2, "aTexCoords"}
        };

        vbo->SetLayout(layout);

        Ref<IndexBuffer> ibo = IndexBuffer::Create(sizeof(indices) / sizeof(uint32_t), indices);

        Ref<VertexArray> vao = VertexArray::Create();
        vao->AddVertexBuffer(vbo);
        vao->SetIndexBuffer(ibo);

        Ref<Material> material = Material::Create("res/shaders/Plane.shader");

        s_Data->MVP = 
            Application::Get().GetWindow().GetSceneCamera()->CalculateVP() * 
            Maths::translate(data.Translation) * 
            Maths::Quaternion::CalculateRotationMatrix(data.Angle, data.Rotation) * 
            Maths::scale(Maths::vec3(data.Size.xy, 0.1f));

        material->SetUniformValue("u_MVP", s_Data->MVP);
        material->Bind();

        RenderCommand::DrawIndexed(vao);
    }

    void Renderer::DrawCube(const PrimitiveData& data)
    {
        float vertices[64] = {
            //  position               color                                        texture coords
                -0.5f, -0.5f,  0.5f,   data.Color.r, data.Color.g, data.Color.b,    0.f,  0.f,       // 0  BL
                 0.5f, -0.5f,  0.5f,   data.Color.r, data.Color.g, data.Color.b,    1.f,  0.f,       // 1  BR
                 0.5f,  0.5f,  0.5f,   data.Color.r, data.Color.g, data.Color.b,    1.f,  1.f,       // 2  TR
                -0.5f,  0.5f,  0.5f,   data.Color.r, data.Color.g, data.Color.b,    0.f,  1.f,       // 3  TL
                -0.5,  -0.5f, -0.5f,   data.Color.r, data.Color.g, data.Color.b,    0.f,  0.f,       // 4  
                -0.5f,  0.5f, -0.5f,   data.Color.r, data.Color.g, data.Color.b,    1.f,  0.f,       // 5
                 0.5f,  0.5f, -0.5f,   data.Color.r, data.Color.g, data.Color.b,    1.f,  1.f,       // 6
                 0.5f, -0.5f, -0.5f,   data.Color.r, data.Color.g, data.Color.b,    0.f,  1.f        // 7
        };

        uint32_t indices[] = {
            0, 1, 2,
            2, 3, 0,
            2, 1, 7,
            7, 6, 2,
            2, 6, 5,
            5, 3, 2,
            3, 0, 4,
            4, 5, 3,
            0, 4, 7,
            7, 1, 0,
            5, 4, 7,
            7, 6, 5
        };

        Ref<VertexBuffer> vbo = VertexBuffer::Create(sizeof(vertices), vertices);

        BufferLayout layout = {
            {ShaderDataType::Float3, "aPosition"},
            {ShaderDataType::Float3, "aColor"},
            {ShaderDataType::Float2, "aTexCoords"}
        };

        vbo->SetLayout(layout);

        Ref<IndexBuffer> ibo = IndexBuffer::Create(sizeof(indices) / sizeof(uint32_t), indices);

        Ref<VertexArray> vao = VertexArray::Create();
        vao->AddVertexBuffer(vbo);
        vao->SetIndexBuffer(ibo);

        Ref<Material> material = Material::Create("res/shaders/Plane.shader");

        s_Data->MVP =
            Application::Get().GetWindow().GetSceneCamera()->CalculateVP() *
            Maths::translate(data.Translation) *
            Maths::Quaternion::CalculateRotationMatrix(data.Angle, data.Rotation) *
            Maths::scale(data.Size.x);
            
        material->SetUniformValue("u_MVP", s_Data->MVP);
        material->Bind();
        
        RenderCommand::DrawIndexed(vao);
    }

    // http://www.songho.ca/opengl/gl_sphere.html
    void Renderer::DrawSphere(const SphereData& data)
    {
        size_t faces = static_cast<size_t>(data.SectorCount) * static_cast<size_t>(data.StackCount) + 1;

        std::vector<float> vertices;
        std::vector<float> normals;
        std::vector<float> texCoords;

        vertices.reserve(faces * 3);
        normals.reserve(faces * 3);
        texCoords.reserve(faces * 2);

        float x, y, z, xy;                              // vertex position
        float nx, ny, nz, lengthInv = 1.f / data.Radius;    // vertex normal
        float s, t;                                     // vertex texCoord

        float sectorStep = Maths::TAU / data.SectorCount;
        float stackStep = Maths::PI / data.StackCount;
        float sectorAngle, stackAngle;

        for (int i = 0; i < data.StackCount; i++)
        {
            stackAngle = Maths::PI / 2 - i * stackStep;        // (phi) starting from pi/2 to -pi/2
            xy = data.Radius * Maths::cos(stackAngle);             // r * cos(u)
            z = data.Radius * Maths::sin(stackAngle);              // r * sin(u)

            // add (sectorCount+1) vertices per stack
            // the first and last vertices have same position and normal, but different tex coords
            for (int j = 0; j <= data.SectorCount; ++j)
            {
                sectorAngle = j * sectorStep;           // starting from 0 to 2pi

                // vertex position (x, y, z)
                x = xy * cosf(sectorAngle);             // r * cos(u) * cos(v)
                y = xy * sinf(sectorAngle);             // r * cos(u) * sin(v)
                vertices.push_back(x);
                vertices.push_back(y);
                vertices.push_back(z);

                // normalized vertex normal (nx, ny, nz)
                nx = x * lengthInv;
                ny = y * lengthInv;
                nz = z * lengthInv;
                normals.push_back(nx);
                normals.push_back(ny);
                normals.push_back(nz);

                // vertex tex coord (s, t) range between [0, 1]
                s = (float)j / data.SectorCount;
                t = (float)i / data.StackCount;
                texCoords.push_back(s);
                texCoords.push_back(t);
            }
        }

        std::vector<uint32_t> indices;
        //std::vector<int> lineIndices;
        indices.reserve(faces * 3);
        //lineIndices.reserve(faces * 3);

        int k1, k2;
        for (uint32_t i = 0; i < data.StackCount; ++i)
        {
            k1 = i * (data.SectorCount + 1);     // beginning of current stack
            k2 = k1 + data.SectorCount + 1;      // beginning of next stack

            for (uint32_t j = 0; j < data.SectorCount; ++j, ++k1, ++k2)
            {
                // 2 triangles per sector excluding first and last stacks
                // k1 => k2 => k1+1
                if (i != 0)
                {
                    indices.push_back(k1);
                    indices.push_back(k2);
                    indices.push_back(k1 + 1);
                }

                // k1+1 => k2 => k2+1
                if (i != (data.StackCount - 1))
                {
                    indices.push_back(k1 + 1);
                    indices.push_back(k2);
                    indices.push_back(k2 + 1);
                }

                // store indices for lines
                // vertical lines for all stacks, k1 => k2
                //lineIndices.push_back(k1);
                //lineIndices.push_back(k2);
                //if (i != 0)  // horizontal lines except 1st stack, k1 => k+1
                //{
                //    lineIndices.push_back(k1);
                //    lineIndices.push_back(k1 + 1);
                //}
            }
        }

        
        std::size_t i, j;
        std::size_t count = vertices.size();
        std::vector<float> vertexBufferData;
        vertexBufferData.reserve(count * 8);
        for (i = 0, j = 0; i < count; i += 3, j += 2)
        {
            vertexBufferData.push_back(vertices[i]);
            vertexBufferData.push_back(vertices[i + 1]);
            vertexBufferData.push_back(vertices[i + 2]);

            vertexBufferData.push_back(normals[i]);
            vertexBufferData.push_back(normals[i + 1]);
            vertexBufferData.push_back(normals[i + 2]);

            vertexBufferData.push_back(texCoords[j]);
            vertexBufferData.push_back(texCoords[j + 1]);
        }

        Ref<VertexBuffer> vbo = VertexBuffer::Create(sizeof(float) * vertexBufferData.size(), vertexBufferData.data());

        BufferLayout layout = {
            {ShaderDataType::Float3, "aPosition"},
            {ShaderDataType::Float3, "aNormal"},
            {ShaderDataType::Float2, "aTexCoords"}
        };

        vbo->SetLayout(layout);

        Ref<IndexBuffer> ibo = IndexBuffer::Create(indices.size(), indices.data());

        Ref<VertexArray> vao = VertexArray::Create();
        vao->AddVertexBuffer(vbo);
        vao->SetIndexBuffer(ibo);

        Ref<Material> material = Material::Create("res/shaders/Sphere.shader");

        s_Data->MVP =
            Application::Get().GetWindow().GetSceneCamera()->CalculateVP() *
            Maths::translate(data.Translation) *
            Maths::Quaternion::CalculateRotationMatrix(data.Angle, data.Rotation) *
            Maths::scale(data.Radius);

        material->SetUniformValue("u_MVP", s_Data->MVP);
        material->SetUniformValue("u_Color", data.Color);
        material->Bind();

        RenderCommand::DrawIndexed(vao);
    }

    void Renderer::DrawIcoSphere(const IcoSphereData& data)
    {
    
    }

    void Renderer::DrawCubeSphere()
    {
    }

    void Renderer::DrawTriangularPyramid()
    {
    }


}


