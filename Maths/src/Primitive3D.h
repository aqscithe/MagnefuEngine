#pragma once

#include "PrimitiveCommon.h"
#include "Vectors.h"
#include <array>


namespace Primitive
{
    struct Cube
    {
        std::array<BasicVertex, 36> Vertices;
    };

    Cube CreateCube();
    //Cube CreateCube(float x, float y, float z, unsigned int texID = 99);
}