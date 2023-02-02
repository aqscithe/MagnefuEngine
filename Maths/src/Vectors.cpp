#include "Vectors.h"

namespace Maths
{
    vec3 normalize(const vec3& v)
    {
        float mag = magnitude(v);
        if (mag == 0.f)
        {
            return { 0.f, 0.f, 0.f };
        }

        return { v.x / mag, v.y / mag, v.z / mag };
    }

    vec4 normalize(const vec4& v)
    {
        float mag = magnitude(v);
        if (mag == 0.f)
        {
            return { 0.f, 0.f, 0.f, 0.f };
        }

        return { v.x / mag, v.y / mag, v.z / mag, v.w / mag };
    }

    vec3 crossProduct(const vec3& a, const vec3& b)
    {
        return {
            a.y * b.z - b.y * a.z,
            a.z * b.x - b.z * a.x,
            a.x * b.y - b.x * a.y
        };
    }

    vec3 slerp(const vec3& v_i, const vec3& v_f, float k)
    {
        // theta is in radians
        float theta = acos(dotProduct(Maths::normalize(v_i), Maths::normalize(v_f)));
        return ((sin((1 - k) * theta) / sin(theta)) * v_i) + ((sin(k * theta) / sin(theta)) * v_f);
    }

    vec4 slerp(const vec4& v_i, const vec4& v_f, float k)
    {
        float theta = acos(dotProduct(normalize(v_i), normalize(v_f)));
        return ((sin((1 - k) * theta) / sin(theta)) * v_i) + ((sin(k * theta) / sin(theta)) * v_f);
    }
}