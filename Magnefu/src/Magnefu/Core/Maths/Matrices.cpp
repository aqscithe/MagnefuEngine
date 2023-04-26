#include "mfpch.h"

namespace Maths
{

   mat3::mat3(const mat4& m)
    {
       c[0] = m.c[0].xyz;
       c[1] = m.c[1].xyz;
       c[2] = m.c[2].xyz;
    }

   
    /*mat4::mat4(vec4 v0, vec4 v1, vec4 v2, vec4 v3)
    {
        c[0] = v0;
        c[1] = v1;
        c[2] = v2;
        c[3] = v3;
    }

    mat4::mat4(
        float x0, float x1, float x2, float x3,
        float y0, float y1, float y2, float y3,
        float z0, float z1, float z2, float z3,
        float w0, float w1, float w2, float w3
    )
    {
        c[0].e[0] = x0;
        c[0].e[1] = x1;
        c[0].e[2] = x2;
        c[0].e[3] = x3;

        c[1].e[0] = y0;
        c[1].e[1] = y1;
        c[1].e[2] = y2;
        c[1].e[3] = y3;

        c[2].e[0] = z0;
        c[2].e[1] = z1;
        c[2].e[2] = z2;
        c[2].e[3] = z3;

        c[3].e[0] = w0;
        c[3].e[1] = w1;
        c[3].e[2] = w2;
        c[3].e[3] = w3;
    }

    mat4::mat4(mat3 m)
    {
        c[0].e[0] = m.c[0].e[0];
        c[0].e[1] = m.c[0].e[1];
        c[0].e[2] = m.c[0].e[2];
        c[0].e[3] = 0.f;

        c[1].e[0] = m.c[1].e[0];
        c[1].e[1] = m.c[1].e[1];
        c[1].e[2] = m.c[1].e[2];
        c[1].e[3] = 0.f;

        c[2].e[0] = m.c[2].e[0];
        c[2].e[1] = m.c[2].e[1];
        c[2].e[2] = m.c[2].e[2];
        c[2].e[3] = 0.f;

        c[3].e[0] = 0.f;
        c[3].e[1] = 0.f;
        c[3].e[2] = 0.f;
        c[3].e[3] = 1.f;*/
    //}

    mat4 identity()
    {
        return {
            1.f, 0.f, 0.f, 0.f,
            0.f, 1.f, 0.f, 0.f,
            0.f, 0.f, 1.f, 0.f,
            0.f, 0.f, 0.f, 1.f,
        };
    }

    mat4 axis(const vec3& r, const vec3& u, const vec3& f)
    {
        return {
            r.x, u.x, f.x, 0.f,
            r.y, u.y, f.y, 0.f,
            r.z, u.z, f.z, 0.f,
            0.f, 0.f, 0.f, 1.f
        };
    }

    mat4 translate(const vec3& t)
    {
        return {
            1.f, 0.f, 0.f, 0.f,
            0.f, 1.f, 0.f, 0.f,
            0.f, 0.f, 1.f, 0.f,
            t.x, t.y, t.z, 1.f,
        };
    }

    mat4 scale(const vec3& s)
    {
        return {
            s.x, 0.f, 0.f, 0.f,
            0.f, s.y, 0.f, 0.f,
            0.f, 0.f, s.z, 0.f,
            0.f, 0.f, 0.f, 1.f,
        };
    }

    mat4 rotate(const float angleInDegrees, const vec3& axis)
    {
        float angleInRadians = toRadians(angleInDegrees);
        float c = cos(angleInRadians);
        float s = sin(angleInRadians);


        return
        {
            c + pow(axis.x, 2) * (1.f - c),             axis.y * axis.x * (1.f - c) + axis.z * s,   axis.z * axis.x * (1.f - c) - axis.y * s,   0.f,
            axis.x * axis.y * (1.f - c) - axis.z * s,   c + pow(axis.y, 2) * (1.f - c),             axis.z * axis.y * (1.f - c) + axis.x * s,   0.f,
            axis.x * axis.z * (1.f - c) + axis.y * s,   axis.y * axis.z * (1.f - c) - axis.x * s,   c + pow(axis.z, 2) * (1.f - c),             0.f,
            0,                                          0,                                          0,                                          1.f
        };
    }

    mat4 rotateX(float& angleRadians)
    {
        float c = cos(angleRadians);
        float s = sin(angleRadians);
        return
        {
            1.f, 0.f, 0.f, 0.f,
            0.f,   c,   s, 0.f,
            0.f,  -s,   c, 0.f,
            0.f, 0.f, 0.f, 1.f,
        };
    }

    mat4 rotateY(float& angleRadians)
    {
        float c = cos(angleRadians);
        float s = sin(angleRadians);
        return
        {
              c, 0.f,  -s, 0.f,
            0.f, 1.f, 0.f, 0.f,
              s, 0.f,   c, 0.f,
            0.f, 0.f, 0.f, 1.f,
        };
    }

    mat4 rotateZ(float& angleRadians)
    {
        float c = cos(angleRadians);
        float s = sin(angleRadians);
        return
        {
              c,   s,  0.f, 0.f,
             -s,   c,  0.f, 0.f,
            0.f, 0.f,  1.f, 0.f,
            0.f, 0.f,  0.f, 1.f,
        };
    }

    mat4 perspective(float fovY, float aspect, float _near, float _far)
    {
        float top = _near * tan(fovY / 2.f);
        float right = top * aspect;
        return frustum(-right, right, -top, top, _near, _far);
    }

    mat4 orthographic(float left, float right, float bottom, float top, float _near, float _far)
    {
        return
        {
            2 / (right - left),                 0.f,                               0.f,                         0.f,
            0.f,                                2 / (top - bottom),                0.f,                         0.f,
            0.f,                                0.f,                              -2 / (_far - _near),            0.f,
            -(right + left) / (right - left),   -(top + bottom) / (top - bottom), -(_far + _near) / (_far - _near), 1.f
        };
    }

    mat4 frustum(float left, float right, float bottom, float top, float _near, float _far)
    {
        return
        {
            (_near * 2.f) / (right - left),   0.f,                              0.f,                               0.f,
            0.f,                             (_near * 2.f) / (top - bottom),    0.f,                               0.f,
            (right + left) / (right - left), (top + bottom) / (top - bottom), -(_far + _near) / (_far - _near),      -1.f,
            0.f,                             0.f,                             -(_far * _near * 2.f) / (_far - _near), 0.f
        };
    }

    bool invert(const float m[16], float out[16])
    {
        float inv[16], det;
        int i;

        inv[0] = m[5] * m[10] * m[15] -
            m[5] * m[11] * m[14] -
            m[9] * m[6] * m[15] +
            m[9] * m[7] * m[14] +
            m[13] * m[6] * m[11] -
            m[13] * m[7] * m[10];

        inv[4] = -m[4] * m[10] * m[15] +
            m[4] * m[11] * m[14] +
            m[8] * m[6] * m[15] -
            m[8] * m[7] * m[14] -
            m[12] * m[6] * m[11] +
            m[12] * m[7] * m[10];

        inv[8] = m[4] * m[9] * m[15] -
            m[4] * m[11] * m[13] -
            m[8] * m[5] * m[15] +
            m[8] * m[7] * m[13] +
            m[12] * m[5] * m[11] -
            m[12] * m[7] * m[9];

        inv[12] = -m[4] * m[9] * m[14] +
            m[4] * m[10] * m[13] +
            m[8] * m[5] * m[14] -
            m[8] * m[6] * m[13] -
            m[12] * m[5] * m[10] +
            m[12] * m[6] * m[9];

        inv[1] = -m[1] * m[10] * m[15] +
            m[1] * m[11] * m[14] +
            m[9] * m[2] * m[15] -
            m[9] * m[3] * m[14] -
            m[13] * m[2] * m[11] +
            m[13] * m[3] * m[10];

        inv[5] = m[0] * m[10] * m[15] -
            m[0] * m[11] * m[14] -
            m[8] * m[2] * m[15] +
            m[8] * m[3] * m[14] +
            m[12] * m[2] * m[11] -
            m[12] * m[3] * m[10];

        inv[9] = -m[0] * m[9] * m[15] +
            m[0] * m[11] * m[13] +
            m[8] * m[1] * m[15] -
            m[8] * m[3] * m[13] -
            m[12] * m[1] * m[11] +
            m[12] * m[3] * m[9];

        inv[13] = m[0] * m[9] * m[14] -
            m[0] * m[10] * m[13] -
            m[8] * m[1] * m[14] +
            m[8] * m[2] * m[13] +
            m[12] * m[1] * m[10] -
            m[12] * m[2] * m[9];

        inv[2] = m[1] * m[6] * m[15] -
            m[1] * m[7] * m[14] -
            m[5] * m[2] * m[15] +
            m[5] * m[3] * m[14] +
            m[13] * m[2] * m[7] -
            m[13] * m[3] * m[6];

        inv[6] = -m[0] * m[6] * m[15] +
            m[0] * m[7] * m[14] +
            m[4] * m[2] * m[15] -
            m[4] * m[3] * m[14] -
            m[12] * m[2] * m[7] +
            m[12] * m[3] * m[6];

        inv[10] = m[0] * m[5] * m[15] -
            m[0] * m[7] * m[13] -
            m[4] * m[1] * m[15] +
            m[4] * m[3] * m[13] +
            m[12] * m[1] * m[7] -
            m[12] * m[3] * m[5];

        inv[14] = -m[0] * m[5] * m[14] +
            m[0] * m[6] * m[13] +
            m[4] * m[1] * m[14] -
            m[4] * m[2] * m[13] -
            m[12] * m[1] * m[6] +
            m[12] * m[2] * m[5];

        inv[3] = -m[1] * m[6] * m[11] +
            m[1] * m[7] * m[10] +
            m[5] * m[2] * m[11] -
            m[5] * m[3] * m[10] -
            m[9] * m[2] * m[7] +
            m[9] * m[3] * m[6];

        inv[7] = m[0] * m[6] * m[11] -
            m[0] * m[7] * m[10] -
            m[4] * m[2] * m[11] +
            m[4] * m[3] * m[10] +
            m[8] * m[2] * m[7] -
            m[8] * m[3] * m[6];

        inv[11] = -m[0] * m[5] * m[11] +
            m[0] * m[7] * m[9] +
            m[4] * m[1] * m[11] -
            m[4] * m[3] * m[9] -
            m[8] * m[1] * m[7] +
            m[8] * m[3] * m[5];

        inv[15] = m[0] * m[5] * m[10] -
            m[0] * m[6] * m[9] -
            m[4] * m[1] * m[10] +
            m[4] * m[2] * m[9] +
            m[8] * m[1] * m[6] -
            m[8] * m[2] * m[5];

        det = m[0] * inv[0] + m[1] * inv[4] + m[2] * inv[8] + m[3] * inv[12];

        if (det == 0)
            return false;

        det = 1.0f / det;

        for (i = 0; i < 16; i++)
            out[i] = inv[i] * det;

        return true;
    }

    mat4 transpose(const Maths::mat4& m)
    {
        return {
            m.e[0], m.e[4], m.e[8],  m.e[12],
            m.e[1], m.e[5], m.e[9],  m.e[13],
            m.e[2], m.e[6], m.e[10], m.e[14],
            m.e[3], m.e[7], m.e[11], m.e[15]
        };
    }


}