#pragma once

#include "Vectors.h"
#include <array>


namespace Magnefu
{
    struct PrimitiveData
    {
        Maths::vec3 Size;
        Maths::vec3 Color;
        Maths::vec3 Rotation;
        Maths::vec3 Translation;
        float       Angle;
    };

    struct SphereData
    {
        Maths::vec3 Color;
        Maths::vec3 Rotation;
        Maths::vec3 Translation;
        float       Angle;
        float       Radius;
        uint32_t    SectorCount;  //32
        uint32_t    StackCount;   //13
    };

    struct IcoSphereData
    {
        float       Radius;
        uint32_t    Subdivisions;
        Maths::vec3 Color;
        Maths::vec3 Rotation;
        Maths::vec3 Translation;
        float       Angle;
    };

    struct CubeSphere
    {

    };

    struct Vertex
    {
        Maths::vec4  Color;          // Color
        Maths::vec3  Position;       // Pos
        Maths::vec3  Normal;

        Maths::vec2  TextureCoords;  // Texture coordinates
        unsigned int TextureID;      // Texture ID
    };

    struct BasicVertex
    {
        Maths::vec3  Position;       // Pos
        Maths::vec3  Normal;
    };

    struct TextureVertex
    {
        Maths::vec3  Position;
        Maths::vec3  Normal;
        Maths::vec2  TextureCoords;
        unsigned int TextureID;
    };

    struct ObjModelVertex
    {
        Maths::vec3 Position;
        Maths::vec2 TextureCoords;
        Maths::vec3 Normal;
        
    };

    struct Face
    {
        std::array< Maths::vec3i, 4> Indices;
        unsigned int MatID;
        uint32_t VertexCount;
    };
}

