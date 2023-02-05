#include "Primitive2D.h"



namespace Primitive
{

    Quad CreateQuad(float x, float y, unsigned int texID)
    {
        float sideLength = 1.f;

        // Bottom left
        Vertex v0;
        v0.Position = { x, y, 0.f };
        v0.Color = { 0.f, 0.3f, 0.9f, 1.f };
        v0.TextureCoords = { 0.f, 0.f };
        v0.TextureID = texID;

        // Bottom right
        Vertex v1;
        v1.Position = { x + sideLength, y, 0.f };
        v1.Color = { 0.f, 0.9f, 0.9f, 1.f };
        v1.TextureCoords = { 1.f,  0.f };
        v1.TextureID = texID;

        // Top right
        Vertex v2;
        v2.Position = { x + sideLength, y + sideLength, 0.f };
        v2.Color = { 0.f, 0.3f, 0.9f, 1.f };
        v2.TextureCoords = { 1.f,  1.f };
        v2.TextureID = texID;

        // Top left
        Vertex v3;
        v3.Position = { x, y + sideLength, 0.f };
        v3.Color = { 0.f, 0.9f, 0.9f, 1.f };
        v3.TextureCoords = { 0.f, 1.f };
        v3.TextureID = texID;

        return { v0, v1, v2, v3 };
    }
}