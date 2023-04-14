#pragma once

#include "Vectors.h"
#include <array>


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
    Maths::vec3 Normal;
    Maths::vec2  TextureCoords;
    unsigned int TextureID;
};

struct Face
{
    std::array< Maths::vec3i, 4> Indices;
    unsigned int MatID;
    uint32_t VertexCount;
};
