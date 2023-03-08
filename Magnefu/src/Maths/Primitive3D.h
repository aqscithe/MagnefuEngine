#pragma once

#include "PrimitiveCommon.h"
#include "Vectors.h"
#include <array>


namespace Primitive
{
    struct BasicCube
    {
        std::array<BasicVertex, 36> Vertices;
    };

    struct TextureCube
    {
        std::array<TextureVertex, 36> Vertices;
    };

    
    BasicCube CreateBasicCube();
    TextureCube CreateTextureCube();
}