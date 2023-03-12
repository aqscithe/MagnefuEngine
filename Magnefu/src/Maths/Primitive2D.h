#pragma once

#include "PrimitiveCommon.h"


namespace Primitive
{
    struct Point
    {
        Maths::vec2 Position;
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