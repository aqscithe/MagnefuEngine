#include "Primitive3D.h"

namespace Primitive
{
    Cube CreateCube(float x, float y, float z, unsigned int texID)
    {
        float sideLength = 1.f;

        // FRONT FACE
        
        // Bottom left
        Vertex v0;
        v0.Position = { x, y, z };
        v0.Color = { 0.f, 0.3f, 0.9f, 1.f };
        v0.TextureCoords = { 0.f, 0.f };
        v0.TextureID = texID;

        // Bottom right
        Vertex v1;
        v1.Position = { x + sideLength, y, z };
        v1.Color = { 0.f, 0.9f, 0.9f, 1.f };
        v1.TextureCoords = { 1.f,  0.f };
        v1.TextureID = texID;

        // Top right
        Vertex v2;
        v2.Position = { x + sideLength, y + sideLength, z };
        v2.Color = { 0.f, 0.3f, 0.9f, 1.f };
        v2.TextureCoords = { 1.f,  1.f };
        v2.TextureID = texID;

        // Top left
        Vertex v3;
        v3.Position = { x, y + sideLength, z };
        v3.Color = { 0.f, 0.9f, 0.9f, 1.f };
        v3.TextureCoords = { 0.f, 1.f };
        v3.TextureID = texID;

        // BACK FACE

        // Bottom left
        Vertex v4;
        v4.Position = { x, y, z - sideLength };
        v4.Color = { 0.f, 0.3f, 0.9f, 1.f };
        v4.TextureCoords = { 0.f, 0.f };
        v4.TextureID = texID;

        // Bottom right
        Vertex v5;
        v5.Position = { x + sideLength, y, z - sideLength };
        v5.Color = { 0.f, 0.9f, 0.9f, 1.f };
        v5.TextureCoords = { 1.f,  0.f };
        v5.TextureID = texID;

        // Top right
        Vertex v6;
        v6.Position = { x + sideLength, y + sideLength, z - sideLength };
        v6.Color = { 0.f, 0.3f, 0.9f, 1.f };
        v6.TextureCoords = { 1.f,  1.f };
        v6.TextureID = texID;

        // Top left
        Vertex v7;
        v7.Position = { x, y + sideLength, z - sideLength };
        v7.Color = { 0.f, 0.9f, 0.9f, 1.f };
        v7.TextureCoords = { 0.f, 1.f };
        v7.TextureID = texID;

        return { v0, v1, v2, v3, v4, v5, v6, v7 };
    }

}