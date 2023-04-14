#pragma once

#include "PrimitiveCommon.h"


namespace Magnefu
{
    struct Point
    {
        Maths::vec2 Position;
    };

    struct Plane
    {
        Maths::vec2 Size;
        Maths::vec3 Color;
        Maths::vec3 Rotation;
        float Angle;
    };

    struct Triangle
    {
        Vertex v0;
        Vertex v1;
        Vertex v2;
    };

    struct Quad
    {
        Vertex v0;
        Vertex v1;
        Vertex v2;
        Vertex v3;
    };

    Quad CreateQuad(float x, float y, unsigned int texID = 99);
}