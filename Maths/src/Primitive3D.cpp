#include "Primitive3D.h"

namespace Primitive
{
    Cube CreateCube()
    {
        return {
            BasicVertex{{-0.5f, -0.5f, -0.5f},    {0.0f, 0.0f, -1.0f},     {1.f, 1.f, 1.f, 1.f}},
            BasicVertex{{ 0.5f, -0.5f, -0.5f},    {0.0f, 0.0f, -1.0f},     {1.f, 1.f, 1.f, 1.f}},
            BasicVertex{{ 0.5f, 0.5f, -0.5f },    {0.0f, 0.0f, -1.0f},     {1.f, 1.f, 1.f, 1.f}},
            BasicVertex{{ 0.5f, 0.5f, -0.5f },    {0.0f, 0.0f, -1.0f},     {1.f, 1.f, 1.f, 1.f}},
            BasicVertex{{-0.5f, 0.5f, -0.5f },    {0.0f, 0.0f, -1.0f},     {1.f, 1.f, 1.f, 1.f}},
            BasicVertex{{-0.5f, -0.5f, -0.5f},    {0.0f, 0.0f, -1.0f},     {1.f, 1.f, 1.f, 1.f}},

            BasicVertex{{-0.5f, -0.5f, 0.5f },    { 0.0f, 0.0f, 1.0f},     {1.f, 1.f, 1.f, 1.f}},
            BasicVertex{{ 0.5f, -0.5f, 0.5f },    { 0.0f, 0.0f, 1.0f},     {1.f, 1.f, 1.f, 1.f}},
            BasicVertex{{ 0.5f, 0.5f, 0.5f  },    { 0.0f, 0.0f, 1.0f},     {1.f, 1.f, 1.f, 1.f}},
            BasicVertex{{ 0.5f, 0.5f, 0.5f  },    { 0.0f, 0.0f, 1.0f},     {1.f, 1.f, 1.f, 1.f}},
            BasicVertex{{-0.5f, 0.5f, 0.5f  },    { 0.0f, 0.0f, 1.0f},     {1.f, 1.f, 1.f, 1.f}},
            BasicVertex{{-0.5f, -0.5f, 0.5f },    { 0.0f, 0.0f, 1.0f},     {1.f, 1.f, 1.f, 1.f}},

            BasicVertex{{-0.5f, 0.5f, 0.5f   },    {-1.0f, 0.0f, 0.0f},    {1.f, 1.f, 1.f, 1.f}},
            BasicVertex{{-0.5f, 0.5f, -0.5f  },    {-1.0f, 0.0f, 0.0f},    {1.f, 1.f, 1.f, 1.f}},
            BasicVertex{{-0.5f, -0.5f, -0.5f },    {-1.0f, 0.0f, 0.0f},    {1.f, 1.f, 1.f, 1.f}},
            BasicVertex{{-0.5f, -0.5f, -0.5f },    {-1.0f, 0.0f, 0.0f},    {1.f, 1.f, 1.f, 1.f}},
            BasicVertex{{-0.5f, -0.5f, 0.5f  },    {-1.0f, 0.0f, 0.0f},    {1.f, 1.f, 1.f, 1.f}},
            BasicVertex{{-0.5f, 0.5f, 0.5f   },    {-1.0f, 0.0f, 0.0f},    {1.f, 1.f, 1.f, 1.f}},

            BasicVertex{ {0.5f, 0.5f, 0.5f   },  {1.0f, 0.0f, 0.0f},      {1.f, 1.f, 1.f, 1.f}},
            BasicVertex{ {0.5f, 0.5f, -0.5f  },  {1.0f, 0.0f, 0.0f},      {1.f, 1.f, 1.f, 1.f}},
            BasicVertex{ {0.5f, -0.5f, -0.5f },  {1.0f, 0.0f, 0.0f},      {1.f, 1.f, 1.f, 1.f}},
            BasicVertex{ {0.5f, -0.5f, -0.5f },  {1.0f, 0.0f, 0.0f},      {1.f, 1.f, 1.f, 1.f}},
            BasicVertex{ {0.5f, -0.5f, 0.5f  },  {1.0f, 0.0f, 0.0f},      {1.f, 1.f, 1.f, 1.f}},
            BasicVertex{ {0.5f, 0.5f, 0.5f   },  {1.0f, 0.0f, 0.0f},      {1.f, 1.f, 1.f, 1.f}},

            BasicVertex{{-0.5f, -0.5f, -0.5f},  { 0.0f, -1.0f, 0.0f},   {  1.f, 1.f, 1.f, 1.f}},
            BasicVertex{{ 0.5f, -0.5f, -0.5f},  { 0.0f, -1.0f, 0.0f},   {  1.f, 1.f, 1.f, 1.f}},
            BasicVertex{{ 0.5f, -0.5f, 0.5f },  { 0.0f, -1.0f, 0.0f},   {  1.f, 1.f, 1.f, 1.f}},
            BasicVertex{{ 0.5f, -0.5f, 0.5f },  { 0.0f, -1.0f, 0.0f},   {  1.f, 1.f, 1.f, 1.f}},
            BasicVertex{{-0.5f, -0.5f, 0.5f },  { 0.0f, -1.0f, 0.0f},   {  1.f, 1.f, 1.f, 1.f}},
            BasicVertex{{-0.5f, -0.5f, -0.5f},  { 0.0f, -1.0f, 0.0f},   {  1.f, 1.f, 1.f, 1.f}},

            BasicVertex{{-0.5f, 0.5f, -0.5f},   { 0.0f, 1.0f, 0.0f},   { 1.f, 1.f, 1.f, 1.f} },
            BasicVertex{{ 0.5f, 0.5f, -0.5f},   { 0.0f, 1.0f, 0.0f},   { 1.f, 1.f, 1.f, 1.f} },
            BasicVertex{{ 0.5f, 0.5f, 0.5f },   { 0.0f, 1.0f, 0.0f},   { 1.f, 1.f, 1.f, 1.f} },
            BasicVertex{{ 0.5f, 0.5f, 0.5f },   { 0.0f, 1.0f, 0.0f},   { 1.f, 1.f, 1.f, 1.f} },
            BasicVertex{{-0.5f, 0.5f, 0.5f },   { 0.0f, 1.0f, 0.0f},   { 1.f, 1.f, 1.f, 1.f} },
            BasicVertex{{-0.5f, 0.5f, -0.5f},   { 0.0f, 1.0f, 0.0f},   { 1.f, 1.f, 1.f, 1.f} }
        };
    }
}


        //float sideLength = 1.f;

        //Vertex v0_z_Pos, v0_y_Neg, v0_x_Neg, v1_z_Pos, v1_y_Neg, v5, v2_z_Pos, v7;
        //Vertex v8, v9, v10, v11, v12, v13, v14, v15;
        //Vertex v16, v17, v18, v19, v20, v21, v22, v23;


        //// 0 vertex
        //v0_z_Pos.Position = { x, y, z };
        //v0_z_Pos.Normal = { 0.0f,  0.0f, 1.0f };

        //v0_y_Neg.Position = { x, y, z };
        //v0_y_Neg.Normal = { 0.0f,  -1.0f, 0.0f };

        //v0_x_Neg.Position = { x, y, z };
        //v0_x_Neg.Normal = { -1.0f,  0.0f, 0.0f };


        //// 1 vertex
        //v1_z_Pos.Position = { x + sideLength, y, z };
        //v1_z_Pos.Normal = { 0.0f,  0.0f, 1.0f };
        //v1_y_Neg.Position = { x + sideLength, y, z };
        //v1_y_Neg.Normal = { 0.0f,  -1.0f, 0.0f };
        //v5.Position = { x + sideLength, y, z };
        //v5.Normal = { 1.0f,  0.0f, 0.0f };


        //// 2 vertex
        //v2_z_Pos.Position = { x + sideLength, y + sideLength, z };
        //v2_z_Pos.Normal = { 0.0f,  0.0f, 1.0f };
        //v7.Position = { x + sideLength, y + sideLength, z };
        //v7.Normal = { 0.0f,  1.0f, 0.0f };
        //v8.Position = { x + sideLength, y + sideLength, z };
        //v8.Normal = { 1.0f,  0.0f, 0.0f };

        //// 3 vertex
        //v9.Position = { x, y + sideLength, z };
        //v9.Normal = { 0.0f,  0.0f, 1.0f };
        //v10.Position = { x, y + sideLength, z };
        //v10.Normal = { 0.0f,  1.0f, 0.0f };
        //v11.Position = { x, y + sideLength, z };
        //v11.Normal = { -1.0f,  0.0f, 0.0f };

        //// 4 vertex
        //v12.Position = { x, y, z - sideLength };
        //v12.Normal = { 0.0f,  0.0f, -1.0f };
        //v13.Position = { x, y, z - sideLength };
        //v13.Normal = { 0.0f,  -1.0f, 0.0f };
        //v14.Position = { x, y, z - sideLength };
        //v14.Normal = { -1.0f,  0.0f, 0.0f };

        //// 5 vertex
        //v15.Position = { x + sideLength, y, z - sideLength };
        //v15.Normal = { 0.0f,  0.0f, -1.0f };
        //v16.Position = { x + sideLength, y, z - sideLength };
        //v16.Normal = { 0.0f,  -1.0f, 0.0f };
        //v17.Position = { x + sideLength, y, z - sideLength };
        //v17.Normal = { 1.0f,  0.0f, 0.0f };

        //// 6 vertex
        //v18.Position = { x + sideLength, y + sideLength, z - sideLength };
        //v18.Normal = { 0.0f,  0.0f, -1.0f };
        //v19.Position = { x + sideLength, y + sideLength, z - sideLength };
        //v19.Normal = { 0.0f,  1.0f, 0.0f };
        //v20.Position = { x + sideLength, y + sideLength, z - sideLength };
        //v20.Normal = { 1.0f,  0.0f, 0.0f };

        //// 7 vertex
        //v21.Position = { x, y + sideLength, z - sideLength };
        //v21.Normal = { 0.0f,  0.0f, -1.0f };
        //v22.Position = { x, y + sideLength, z - sideLength };
        //v22.Normal = { 0.0f,  1.0f, 0.0f };
        //v23.Position = { x, y + sideLength, z - sideLength };
        //v23.Normal = { -1.0f,  0.0f, 0.0f };

        //return { v0_z_Pos, v1_z_Pos, v2_z_Pos,   v0_y_Neg, v13, v16,   v0_x_Neg, v11, v23,   v7, v10, v19,   v5, v17, v20,   v12, v18, v21 };



        //v0_z_Pos.Color = { 1.f, 1.f, 1.f, 1.f };
        //v0_z_Pos.TextureCoords = { 0.f, 0.f };
        //v0_z_Pos.TextureID = texID;

        //
        //
        //v0_y_Neg.Color = { 1.f, 1.f, 1.f, 1.f };
        //v0_y_Neg.TextureCoords = { 1.f,  0.f };
        //v0_y_Neg.TextureID = texID;

        //
        //
        //
        //v0_x_Neg.Color = { 1.f, 1.f, 1.f, 1.f };
        //v0_x_Neg.TextureCoords = { 1.f,  1.f };
        //v0_x_Neg.TextureID = texID;

        //
        //
        //v1_z_Pos.Color = { 1.f, 1.f, 1.f, 1.f };
        //v1_z_Pos.TextureCoords = { 0.f, 1.f };
        //v1_z_Pos.TextureID = texID;

        //

        //// Bottom left
        //
        //
        //v1_y_Neg.Color = { 1.f, 1.f, 1.f, 1.f };
        //v1_y_Neg.TextureCoords = { 0.f, 0.f };
        //v1_y_Neg.TextureID = texID;

        //// Bottom right
        //
        //v5.Color = { 1.f, 1.f, 1.f, 1.f };
        //v5.TextureCoords = { 1.f,  0.f };
        //v5.TextureID = texID;

        //// Top right
        //
        //v2_z_Pos.Color = { 1.f, 1.f, 1.f, 1.f };
        //v2_z_Pos.TextureCoords = { 1.f,  1.f };
        //v2_z_Pos.TextureID = texID;

        //// Top left
        //
        //v7.Color = { 1.f, 1.f, 1.f, 1.f };
        //v7.TextureCoords = { 0.f, 1.f };
        //v7.TextureID = texID;