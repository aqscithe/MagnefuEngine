#pragma once

#include "PrimitiveCommon.h"
#include <array>


namespace Magnefu
{
    struct Cube
    {
        Maths::vec3 Size;
        Maths::vec3 Color;
        Maths::vec3 Rotation;
        float Angle;
    };

    struct BasicCube
    {
        std::array<BasicVertex, 36> Vertices;
    };

    struct TextureCube
    {
        std::array<TextureVertex, 36> Vertices;
    };

    
    //BasicCube CreateBasicCube();
    //TextureCube CreateTextureCube();
}