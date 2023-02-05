#pragma once

#include "PrimitiveCommon.h"
#include "Vectors.h"
#include <array>


namespace Primitive
{
    struct Cube
    {
        std::array<Vertex, 8> Vertices;
    };

    Cube CreateCube(float x, float y, float z, unsigned int texID = 99);
}