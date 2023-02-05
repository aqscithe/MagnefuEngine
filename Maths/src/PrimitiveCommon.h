#pragma once

#include "Vectors.h"


struct Vertex
{
    Maths::vec3  Position;       // Pos
    Maths::vec4  Color;          // Color
    Maths::vec2  TextureCoords;  // Texture coordinates
    unsigned int TextureID;      // Texture ID
};
