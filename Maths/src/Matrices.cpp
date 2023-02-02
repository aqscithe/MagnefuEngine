#include "Matrices.h"

#include "Common.h"

namespace Maths
{
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

    mat4 perspective(float fovY, float aspect, float near, float far)
    {
        float top = near * tan(fovY / 2.f);
        float right = top * aspect;
        return frustum(-right, right, -top, top, near, far);
    }

    mat4 orthographic(float left, float right, float bottom, float top, float near, float far)
    {
        return
        {
            2 / (right - left),                 0.f,                               0.f,                         0.f,
            0.f,                                2 / (top - bottom),                0.f,                         0.f,
            0.f,                                0.f,                              -2 / (far - near),            0.f,
            -(right + left) / (right - left),   -(top + bottom) / (top - bottom), -(far + near) / (far - near), 1.f
        };
    }

    mat4 frustum(float left, float right, float bottom, float top, float near, float far)
    {
        return
        {
            (near * 2.f) / (right - left),   0.f,                              0.f,                               0.f,
            0.f,                             (near * 2.f) / (top - bottom),    0.f,                               0.f,
            (right + left) / (right - left), (top + bottom) / (top - bottom), -(far + near) / (far - near),      -1.f,
            0.f,                             0.f,                             -(far * near * 2.f) / (far - near), 0.f
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


}