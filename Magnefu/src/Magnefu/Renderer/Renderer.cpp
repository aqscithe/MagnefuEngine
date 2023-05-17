#include "mfpch.h"

#include "Renderer.h"
#include "Buffer.h"
#include "VertexArray.h"
#include "Material.h"
#include "Shader.h"
#include "Light.h"
#include "Magnefu/Core/Maths/Quaternion.h"
#include "Magnefu/Application.h"
#include "Magnefu/Debug/Instrumentor.h"
#include "Magnefu/Core/MemoryAllocation/LinkedListAlloc.h"

#include "imgui.h"


namespace Magnefu
{
    // TODO: Could these work as bit flags?
    struct RenderSettings
    {
        bool FaceCulling;
        bool SeamlessCubeMap;
        bool Blending;
        bool DepthTest;
    };

    struct RenderMaterials
    {
        Ref<Material> Plane;
        Ref<Material> Cube;
        Ref<Material> RecPrism;
        Ref<Material> Sphere;
        Ref<Material> Skybox;

        RenderMaterials() :
            Sphere(Material::Create("res/shaders/Sphere.shader")), Plane(Material::Create("res/shaders/Plane.shader")),
            RecPrism(Material::Create("res/shaders/RecPrism.shader")), Cube(Material::Create("res/shaders/Cube.shader")),
            Skybox(Material::Create("res/shaders/Skybox.shader", MaterialOptions_Skybox))
        {

        }
    };

    struct RenderData
    {
        Maths::mat4 MVP;
        Maths::mat4 ModelMatrix;
        Maths::mat4 NormalMatrix;
        Maths::mat4 Inverted;
        DirectionalLight DefaultLight;
    };

    
    static RenderData* s_Data = nullptr;
    static RenderMaterials* s_Materials = nullptr;
    static RenderSettings* s_Settings = nullptr;

    void Renderer::Init()
    {
        RenderCommand::EnableFaceCulling();
        RenderCommand::EnableBlending();
        RenderCommand::EnableDepthTest();
        RenderCommand::EnableSeamlessCubeMap();

        s_Data = static_cast<RenderData*>(StackAllocator::Get()->Allocate(sizeof(RenderData), sizeof(Maths::mat4)));        
        s_Data->DefaultLight.Direction = { -1.f, -1.f, -1.f };
        s_Data->DefaultLight.Color = { 1.f, 0.99f, 0.96f };
        s_Data->DefaultLight.Flux = 3.f;
        s_Data->DefaultLight.Enabled = true;

        s_Materials = static_cast<RenderMaterials*>(StackAllocator::Get()->Allocate(sizeof(RenderMaterials), sizeof(Ref<Material>)));
        new (s_Materials) RenderMaterials();

        s_Settings = static_cast<RenderSettings*>(StackAllocator::Get()->Allocate(sizeof(RenderSettings)));
        s_Settings->FaceCulling = true;
        s_Settings->SeamlessCubeMap = true;
        s_Settings->Blending = true;
        s_Settings->DepthTest = true;

        // -- Loading Primitive Vertex Data -- //

        {
            MF_PROFILE_SCOPE("Load Plane Data");
            std::vector<ObjModelVertex> vertices;
            vertices.reserve(4); // square faces * vertices per face

            Maths::vec3 positions[4] = {
                { -1.f, 0.f, -1.f},
                {  1.f, 0.f, -1.f},
                {  1.f, 0.f,  1.f},
                { -1.f, 0.f,  1.f}
            };

            Maths::vec3 normals[1] = {
                { 0.f,   1.f,   0.f}
            };

            Maths::vec2 texCoords[4] = {
                {0.f, 0.f},
                {1.f, 0.f},
                {1.f, 1.f},
                {0.f, 1.f}
            };

            Maths::vec3i faces[4] = {
                {0,0,0},  {1, 1, 0},  {2,2,0},  {3,3,0}
            };

            int vertexData = sizeof(faces) / sizeof(Maths::vec3i);

            for (int j = 0; j < vertexData; j++)
            {
                vertices.emplace_back(positions[faces[j].x], texCoords[faces[j].y], normals[faces[j].z]);
            }

            uint32_t indices[] = {
                0, 2, 1,
                0, 3, 2
            };


            Ref<VertexBuffer> vbo = VertexBuffer::Create(vertices.size() * sizeof(ObjModelVertex), (float*)vertices.data());

            BufferLayout layout = {
                {ShaderDataType::Float3, "aPosition"},
                {ShaderDataType::Float2, "aTexCoords"},
                {ShaderDataType::Float3, "aNormal"}
            };

            vbo->SetLayout(layout);

            Ref<IndexBuffer> ibo = IndexBuffer::Create(sizeof(indices) / sizeof(uint32_t), indices);

            s_Materials->Plane->AddVertexBuffer(vbo);
            s_Materials->Plane->SetIndexBuffer(ibo);

            s_Materials->Plane->GetVertexArray()->Unbind();
        }

        {
            MF_PROFILE_SCOPE("Load Cube Data");
            std::vector<ObjModelVertex> vertices;
            vertices.reserve(6 * 4); // square faces * vertices per face


            Maths::vec3 positions[8] = {
                {1.f,   1.f, -1.f},
                {1.f,  -1.f, -1.f},
                {1.f,   1.f,  1.f},
                {1.f,  -1.f,  1.f},
                {-1.f,   1.f, -1.f},
                {-1.f,  -1.f, -1.f},
                {-1.f,   1.f,  1.f},
                {-1.f,  -1.f,  1.f}
            };

            Maths::vec3 normals[6] = {
                { 0.f,   1.f,   0.f},
                { 0.f,   0.f,   1.f},
                {-1.f,   0.f,   0.f},
                { 0.f,  -1.f,   0.f},
                { 1.f,   0.f,   0.f},
                { 0.f,   0.f,  -1.f}
            };

            Maths::vec2 texCoords[14] = {
                {0.625f, 0.50f},
                {0.375f, 0.50f},
                {0.625f, 0.75f},
                {0.375f, 0.75f},
                {0.875f, 0.50f},
                {0.625f, 0.25f},
                {0.125f, 0.50f},
                {0.375f, 0.25f},
                {0.875f, 0.75f},
                {0.625f, 1.00f},
                {0.625f, 0.00f},
                {0.375f, 1.00f},
                {0.375f, 0.00f},
                {0.125f, 0.75f}
            };

            Maths::vec3i faces[24] = {
                {0,0,0},  {4,4,0},  {6,8,0},  {2,2,0},   // Top
                {3,3,1},  {2,2,1},  {6,9,1},  {7,11,1},  // Front
                {7,12,2}, {6,10,2}, {4,5,2},  {5,7,2},   // Left
                {5,6,3},  {1,1,3},  {3,3,3},  {7,13,3},  // Bottom
                {1,1,4},  {0,0,4},  {2,2,4},  {3,3,4},   // Right
                {5,7,5},  {4,5,5},  {0,0,5},  {1,1,5}    // Back
            };

            int vertexData = sizeof(faces) / sizeof(Maths::vec3i);

            for (int j = 0; j < vertexData; j++)
            {
                vertices.emplace_back(positions[faces[j].x], texCoords[faces[j].y], normals[faces[j].z]);
            }

            uint32_t indices[] = {
                0,  1,  2,  // Top
                0,  2,  3,
                4,  5,  6,  // Front
                4,  6,  7,
                8,  9,  10, // Left
                8,  10, 11,
                12, 13, 14, // Bottom
                12, 14, 15,
                16, 17, 18, // Right
                16, 18, 19,
                20, 21, 22, // Back
                20, 22, 23
            };

            size_t vertexCount = vertices.size() * sizeof(ObjModelVertex);

            Ref<VertexBuffer> vbo = VertexBuffer::Create(vertices.size() * sizeof(ObjModelVertex), (float*)vertices.data());

            BufferLayout layout = {
                {ShaderDataType::Float3, "aPosition"},
                {ShaderDataType::Float2, "aTexCoords"},
                {ShaderDataType::Float3, "aNormal"},

            };

            vbo->SetLayout(layout);

            Ref<IndexBuffer> ibo = IndexBuffer::Create(sizeof(indices) / sizeof(uint32_t), indices);

            s_Materials->Cube->AddVertexBuffer(vbo);
            s_Materials->Cube->SetIndexBuffer(ibo);

            s_Materials->Cube->GetVertexArray()->Unbind();
        }

        {
            MF_PROFILE_SCOPE("Load Rec Prism Data");

            std::vector<ObjModelVertex> vertices;
            vertices.reserve(6 * 4); // square faces * vertices per face


            Maths::vec3 positions[8] = {
                {1.f,   1.f, -1.f},
                {1.f,  -1.f, -1.f},
                {1.f,   1.f,  1.f},
                {1.f,  -1.f,  1.f},
                {-1.f,   1.f, -1.f},
                {-1.f,  -1.f, -1.f},
                {-1.f,   1.f,  1.f},
                {-1.f,  -1.f,  1.f}
            };

            Maths::vec3 normals[6] = {
                { 0.f,   1.f,   0.f},
                { 0.f,   0.f,   1.f},
                {-1.f,   0.f,   0.f},
                { 0.f,  -1.f,   0.f},
                { 1.f,   0.f,   0.f},
                { 0.f,   0.f,  -1.f}
            };

            Maths::vec2 texCoords[14] = {
                {0.625f, 0.50f},
                {0.375f, 0.50f},
                {0.625f, 0.75f},
                {0.375f, 0.75f},
                {0.875f, 0.50f},
                {0.625f, 0.25f},
                {0.125f, 0.50f},
                {0.375f, 0.25f},
                {0.875f, 0.75f},
                {0.625f, 1.00f},
                {0.625f, 0.00f},
                {0.375f, 1.00f},
                {0.375f, 0.00f},
                {0.125f, 0.75f}
            };

            Maths::vec3i faces[24] = {
                {0,0,0},  {4,4,0},  {6,8,0},  {2,2,0},   // Top
                {3,3,1},  {2,2,1},  {6,9,1},  {7,11,1},  // Front
                {7,12,2}, {6,10,2}, {4,5,2},  {5,7,2},   // Left
                {5,6,3},  {1,1,3},  {3,3,3},  {7,13,3},  // Bottom
                {1,1,4},  {0,0,4},  {2,2,4},  {3,3,4},   // Right
                {5,7,5},  {4,5,5},  {0,0,5},  {1,1,5}    // Back
            };

            int vertexData = sizeof(faces) / sizeof(Maths::vec3i);

            for (int j = 0; j < vertexData; j++)
            {
                vertices.emplace_back(positions[faces[j].x], texCoords[faces[j].y], normals[faces[j].z]);
            }

            uint32_t indices[] = {
                0,  1,  2,  // Top
                0,  2,  3,
                4,  5,  6,  // Front
                4,  6,  7,
                8,  9,  10, // Left
                8,  10, 11,
                12, 13, 14, // Bottom
                12, 14, 15,
                16, 17, 18, // Right
                16, 18, 19,
                20, 21, 22, // Back
                20, 22, 23
            };

            size_t vertexCount = vertices.size() * sizeof(ObjModelVertex);

            Ref<VertexBuffer> vbo = VertexBuffer::Create(vertices.size() * sizeof(ObjModelVertex), (float*)vertices.data());

            BufferLayout layout = {
                {ShaderDataType::Float3, "aPosition"},
                {ShaderDataType::Float2, "aTexCoords"},
                {ShaderDataType::Float3, "aNormal"},

            };

            vbo->SetLayout(layout);

            Ref<IndexBuffer> ibo = IndexBuffer::Create(sizeof(indices) / sizeof(uint32_t), indices);

            s_Materials->RecPrism->AddVertexBuffer(vbo);
            s_Materials->RecPrism->SetIndexBuffer(ibo);

            s_Materials->RecPrism->GetVertexArray()->Unbind();
        }

        {
            MF_PROFILE_SCOPE("Load Sphere Data");

            float radius = 1.f;
            uint32_t sectorCount = 80;
            uint32_t stackCount = 80;
            size_t faces = static_cast<size_t>(sectorCount) * static_cast<size_t>(stackCount) + 1;

            std::vector<float> vertices;
            std::vector<float> normals;
            std::vector<float> texCoords;

            vertices.reserve(faces * 3);
            normals.reserve(faces * 3);
            texCoords.reserve(faces * 2);

            float x, y, z, xy;                              // vertex position
            float nx, ny, nz, lengthInv = 1.f / radius;    // vertex normal
            float s, t;                                     // vertex texCoord

            float sectorStep = Maths::TAU / sectorCount;
            float stackStep = Maths::PI / stackCount;
            float sectorAngle, stackAngle;

            for (int i = 0; i < stackCount; i++)
            {
                stackAngle = Maths::PI / 2 - i * stackStep;        // (phi) starting from pi/2 to -pi/2
                xy = radius * Maths::cos(stackAngle);             // r * cos(u)
                z = radius * Maths::sin(stackAngle);              // r * sin(u)

                // add (sectorCount+1) vertices per stack
                // the first and last vertices have same position and normal, but different tex coords
                for (int j = 0; j <= sectorCount; ++j)
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
                    s = (float)j / sectorCount;
                    t = (float)i / stackCount;
                    texCoords.push_back(s);
                    texCoords.push_back(t);
                }
            }

            std::vector<uint32_t> indices;
            //std::vector<int> lineIndices;
            indices.reserve(faces * 3);
            //lineIndices.reserve(faces * 3);

            int k1, k2;
            for (uint32_t i = 0; i < stackCount; ++i)
            {
                k1 = i * (sectorCount + 1);     // beginning of current stack
                k2 = k1 + sectorCount + 1;      // beginning of next stack

                for (uint32_t j = 0; j < sectorCount; ++j, ++k1, ++k2)
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
                    if (i != (stackCount - 1))
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

            s_Materials->Sphere->AddVertexBuffer(vbo);
            s_Materials->Sphere->SetIndexBuffer(ibo);

            s_Materials->Sphere->GetVertexArray()->Unbind();
        }

        {
            MF_PROFILE_SCOPE("Load Skybox Data");

            float vertices[24] = {
                //  position             
                    -1.f,  -1.f, -1.f,   // 0
                     1.f,  -1.f, -1.f,   // 1
                     1.f,   1.f, -1.f,   // 2
                    -1.f,   1.f, -1.f,   // 3
                    -1.f,  -1.f,  1.f,   // 4
                     1.f,  -1.f,  1.f,   // 5
                     1.f,   1.f,  1.f,   // 6
                    -1.f,   1.f,  1.f    // 7
            };

            uint32_t indices[] = {
                0, 2, 1,  // Back
                0, 3, 2,
                4, 5, 6,  // Front
                4, 6, 7,
                3, 6, 2,  // Top
                3, 7, 6,
                0, 1, 5,  // Bottom
                0, 5, 4,
                1, 6, 5,
                1, 2, 6,
                0, 4, 7,
                0, 7, 3
            };

            Ref<VertexBuffer> vbo = VertexBuffer::Create(sizeof(vertices), vertices);

            BufferLayout layout = {
                {ShaderDataType::Float3, "aPosition"}
            };

            vbo->SetLayout(layout);

            Ref<IndexBuffer> ibo = IndexBuffer::Create(sizeof(indices) / sizeof(uint32_t), indices);

            s_Materials->Skybox->AddVertexBuffer(vbo);
            s_Materials->Skybox->SetIndexBuffer(ibo);
            s_Materials->Skybox->GetVertexArray()->Unbind();
        }

        // -------------------------------- //
    }

    void Renderer::BeginScene()
    {
        Application::Get().GetWindow().GetSceneCamera()->CalculateVP();
    }

    void Renderer::EndScene()
    {
    }

    void Renderer::Submit(const Ref<VertexArray>& va, const Ref<Material>& material)
    {
        material->Bind();
        RenderCommand::DrawIndexed(va);
    }

    void Renderer::DrawPlane(const PrimitiveData& data)
    {
        MF_PROFILE_FUNCTION();
        
        auto& camera = Application::Get().GetWindow().GetSceneCamera();


        {
            MF_PROFILE_SCOPE("MVP Multiplication - PLANE");
            s_Data->ModelMatrix = Maths::translate(data.Translation) *
                Maths::Quaternion::CalculateRotationMatrix(data.Angle, data.Rotation) *
                Maths::scale(Maths::vec3(data.Size.x, 0.1f, data.Size.z));

            s_Data->MVP = camera->GetVP() * s_Data->ModelMatrix;

            s_Data->NormalMatrix = Maths::identity();
            if (Maths::invert(s_Data->ModelMatrix.e, s_Data->Inverted.e))
                s_Data->NormalMatrix = Maths::transpose(s_Data->Inverted);
        }
            
        {
            MF_PROFILE_SCOPE("Upload Uniforms - PLANE");
            s_Materials->Plane->SetUniformValue("u_MVP", s_Data->MVP);
            s_Materials->Plane->SetUniformValue("u_ModelMatrix", s_Data->ModelMatrix);
            s_Materials->Plane->SetUniformValue("u_NormalMatrix", s_Data->NormalMatrix);
            s_Materials->Plane->SetUniformValue("u_CameraPos", camera->GetData().Position);
            s_Materials->Plane->SetUniformValue("u_LightDirection", s_Data->DefaultLight.Direction);
            s_Materials->Plane->SetUniformValue("u_LightColor", s_Data->DefaultLight.Color);
            s_Materials->Plane->SetUniformValue("u_RadiantFlux", s_Data->DefaultLight.Flux);
            s_Materials->Plane->SetUniformValue("u_LightEnabled", s_Data->DefaultLight.Enabled);

            s_Materials->Plane->UpdateMaterialSpec();
            s_Materials->Plane->Bind();
        }

        {
            MF_PROFILE_SCOPE("Draw Call - PLANE");
            RenderCommand::DrawIndexed(s_Materials->Plane->GetVertexArray());
        }
    }


    void Renderer::DrawCube(const PrimitiveData& data)
    {
        MF_PROFILE_FUNCTION();
        
        auto& camera = Application::Get().GetWindow().GetSceneCamera();

        {
            MF_PROFILE_SCOPE("MVP Multiplication - CUBE");
            s_Data->ModelMatrix = Maths::translate(data.Translation) *
                Maths::Quaternion::CalculateRotationMatrix(data.Angle, data.Rotation) *
                Maths::scale(data.Size.x);

            s_Data->MVP = camera->GetVP() * s_Data->ModelMatrix;

            s_Data->NormalMatrix = Maths::identity();
            if (Maths::invert(s_Data->ModelMatrix.e, s_Data->Inverted.e))
                s_Data->NormalMatrix = Maths::transpose(s_Data->Inverted);
        }

        {
            MF_PROFILE_SCOPE("Upload Uniforms - CUBE");
            s_Materials->Cube->SetUniformValue("u_MVP", s_Data->MVP);
            s_Materials->Cube->SetUniformValue("u_ModelMatrix", s_Data->ModelMatrix);
            s_Materials->Cube->SetUniformValue("u_NormalMatrix", s_Data->NormalMatrix);
            s_Materials->Cube->SetUniformValue("u_CameraPos", camera->GetData().Position);
            s_Materials->Cube->SetUniformValue("u_LightDirection", s_Data->DefaultLight.Direction);
            s_Materials->Cube->SetUniformValue("u_LightColor", s_Data->DefaultLight.Color);
            s_Materials->Cube->SetUniformValue("u_RadiantFlux", s_Data->DefaultLight.Flux);
            s_Materials->Cube->SetUniformValue("u_LightEnabled", s_Data->DefaultLight.Enabled);

            s_Materials->Cube->UpdateMaterialSpec();
            s_Materials->Cube->Bind();
        }


        {
            MF_PROFILE_SCOPE("Draw Call - CUBE");
            RenderCommand::DrawIndexed(s_Materials->Cube->GetVertexArray());
        }
    }

    void Renderer::DrawRectangularPrism(const PrimitiveData& data)
    {
        MF_PROFILE_FUNCTION();

        auto& camera = Application::Get().GetWindow().GetSceneCamera();

        {
            MF_PROFILE_SCOPE("MVP Multiplication - CUBE");
            s_Data->ModelMatrix = Maths::translate(data.Translation) *
                Maths::Quaternion::CalculateRotationMatrix(data.Angle, data.Rotation) *
                Maths::scale(data.Size);

            s_Data->MVP = camera->CalculateVP() * s_Data->ModelMatrix;

            s_Data->NormalMatrix = Maths::identity();
            if (Maths::invert(s_Data->ModelMatrix.e, s_Data->Inverted.e))
                s_Data->NormalMatrix = Maths::transpose(s_Data->Inverted);
        }

        {
            MF_PROFILE_SCOPE("Upload Uniforms - CUBE");
            s_Materials->RecPrism->SetUniformValue("u_MVP", s_Data->MVP);
            s_Materials->RecPrism->SetUniformValue("u_ModelMatrix", s_Data->ModelMatrix);
            s_Materials->RecPrism->SetUniformValue("u_NormalMatrix", s_Data->NormalMatrix);
            s_Materials->RecPrism->SetUniformValue("u_CameraPos", camera->GetData().Position);
            s_Materials->RecPrism->SetUniformValue("u_LightDirection", s_Data->DefaultLight.Direction);
            s_Materials->RecPrism->SetUniformValue("u_LightColor", s_Data->DefaultLight.Color);
            s_Materials->RecPrism->SetUniformValue("u_RadiantFlux", s_Data->DefaultLight.Flux);
            s_Materials->RecPrism->SetUniformValue("u_LightEnabled", s_Data->DefaultLight.Enabled);

            s_Materials->RecPrism->UpdateMaterialSpec();
            s_Materials->RecPrism->Bind();
        }

        {
            MF_PROFILE_SCOPE("Draw Call - CUBE");
            RenderCommand::DrawIndexed(s_Materials->RecPrism->GetVertexArray());
        }
    }

    // This algorithm can also be used to make a cylinder, a cone and a pyramid.
    // http://www.songho.ca/opengl/gl_sphere.html
    void Renderer::DrawSphere(const SphereData& data)
    {
        MF_PROFILE_FUNCTION();
        
        auto& camera = Application::Get().GetWindow().GetSceneCamera();

        {
            MF_PROFILE_SCOPE("MVP Multiplication - SPHERE");
            s_Data->ModelMatrix = Maths::translate(data.Translation) *
                Maths::Quaternion::CalculateRotationMatrix(data.Angle, data.Rotation) *
                Maths::scale(data.Radius);

            s_Data->MVP = camera->GetVP() * s_Data->ModelMatrix;

            s_Data->NormalMatrix = Maths::identity();
            if (Maths::invert(s_Data->ModelMatrix.e, s_Data->Inverted.e))
                s_Data->NormalMatrix = Maths::transpose(s_Data->Inverted);
        }
            
        {
            MF_PROFILE_SCOPE("Upload Uniforms - SPHERE");
            s_Materials->Sphere->SetUniformValue("u_MVP", s_Data->MVP);
            s_Materials->Sphere->SetUniformValue("u_Color", data.Color);
            s_Materials->Sphere->SetUniformValue("u_ModelMatrix", s_Data->ModelMatrix);
            s_Materials->Sphere->SetUniformValue("u_NormalMatrix", s_Data->NormalMatrix);
            s_Materials->Sphere->SetUniformValue("u_CameraPos", camera->GetData().Position);
            s_Materials->Sphere->SetUniformValue("u_LightDirection", s_Data->DefaultLight.Direction);
            s_Materials->Sphere->SetUniformValue("u_LightColor", s_Data->DefaultLight.Color);
            s_Materials->Sphere->SetUniformValue("u_RadiantFlux", s_Data->DefaultLight.Flux);
            s_Materials->Sphere->SetUniformValue("u_LightEnabled", s_Data->DefaultLight.Enabled);

            s_Materials->Sphere->UpdateMaterialSpec();
            s_Materials->Sphere->Bind();
        }

        {
            MF_PROFILE_SCOPE("Draw Call - SPHERE");
            RenderCommand::DrawIndexed(s_Materials->Sphere->GetVertexArray());
        }
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

    void Renderer::DrawSkybox()
    {
        MF_PROFILE_FUNCTION();

        auto& camera = Application::Get().GetWindow().GetSceneCamera();
        Maths::mat4* view = static_cast<Maths::mat4*>(StackAllocator::Get()->Allocate(sizeof(Maths::mat4), sizeof(Maths::mat4)));
        *view = camera->GetView();
        view->c[0].w = 0.f;
        view->c[1].w = 0.f;
        view->c[2].w = 0.f;
        view->c[3].xyz = { 0.f, 0.f, 0.f };
        view->c[3].w = 1.f;

        RenderCommand::DepthFuncLEqual();
        RenderCommand::FrontFaceCW();
        {
            MF_PROFILE_SCOPE("Upload Uniforms - Skybox");
            s_Materials->Skybox->SetUniformValue("u_View", *view);
            s_Materials->Skybox->SetUniformValue("u_Projection", camera->GetProjection());
            s_Materials->Skybox->Bind();
        }

        {
            MF_PROFILE_SCOPE("Draw Call - Skybox");
            RenderCommand::DrawIndexed(s_Materials->Skybox->GetVertexArray());
        }
        RenderCommand::FrontFaceCCW();
        RenderCommand::DepthFuncLess();
        
        StackAllocator::Get()->FreeToMarker();
    }

    void Renderer::OnImGuiRender()
    {
        ImGui::Begin("Renderer");
        if (ImGui::BeginTabBar("Renderer", ImGuiTabBarFlags_None))
        {
            if (ImGui::BeginTabItem("Settings"))
            {
                if (ImGui::Checkbox("Face Culling", &s_Settings->FaceCulling))
                {
                    if (s_Settings->FaceCulling)
                    {
                        RenderCommand::EnableFaceCulling();
                        s_Settings->FaceCulling = true;
                    }
                    else
                    {
                        RenderCommand::DisableFaceCulling();
                        s_Settings->FaceCulling = false;
                    }
                }
                if (ImGui::Checkbox("Seamless Cube Map", &s_Settings->SeamlessCubeMap))
                {
                    if (s_Settings->SeamlessCubeMap)
                    {
                        RenderCommand::EnableSeamlessCubeMap();
                        s_Settings->SeamlessCubeMap = true;
                    }
                    else
                    {
                        RenderCommand::DisableSeamlessCubeMap();
                        s_Settings->SeamlessCubeMap = false;
                    }
                }
                if (ImGui::Checkbox("Blending", &s_Settings->Blending))
                {
                    if (s_Settings->Blending)
                    {
                        RenderCommand::EnableBlending();
                        s_Settings->Blending = true;
                    }
                    else
                    {
                        RenderCommand::DisableBlending();
                        s_Settings->Blending = false;
                    }
                }
                if (ImGui::Checkbox("Depth Test", &s_Settings->DepthTest))
                {
                    if (s_Settings->DepthTest)
                    {
                        RenderCommand::EnableDepthTest();
                        s_Settings->DepthTest = true;
                    }
                    else
                    {
                        RenderCommand::DisableDepthTest();
                        s_Settings->DepthTest = false;
                    }
                }

                ImGui::EndTabItem();
            }
            ImGui::EndTabBar();
        }
        ImGui::End();

        ImGui::Begin("Default Directional Light");
        ImGui::Checkbox("Enabled", &s_Data->DefaultLight.Enabled);
        ImGui::SliderFloat("Flux", &s_Data->DefaultLight.Flux, 0.f, 15.f);
        ImGui::SliderFloat3("Direction", s_Data->DefaultLight.Direction.e, -1.f, 1.f);
        ImGui::ColorEdit3("Color", s_Data->DefaultLight.Color.e);
        ImGui::End();


        s_Materials->Plane->OnImGuiRender();
        s_Materials->Cube->OnImGuiRender();
        s_Materials->RecPrism->OnImGuiRender();
        s_Materials->Sphere->OnImGuiRender();
        s_Materials->Skybox->OnImGuiRender();
    }

}
