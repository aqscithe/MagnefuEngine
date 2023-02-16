#pragma once

#include "Vectors.h"

#include <vector>


struct Vertex
{
    Maths::vec3  Position;       // Pos
    Maths::vec3  Normal;
    Maths::vec4  Color;          // Color
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
};


struct Face
{
    Maths::vec3i Vertex1Indices;
    Maths::vec3i Vertex2Indices;
    Maths::vec3i Vertex3Indices;
    Maths::vec3i Vertex4Indices;
};
