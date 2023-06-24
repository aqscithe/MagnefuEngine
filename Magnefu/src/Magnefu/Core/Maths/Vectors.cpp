#include "mfpch.h"

namespace Maths
{
    vec3 StrtoVec3(const std::string& str)
    {
        // could do with some error checking...
        // im being very trustworthy right now

        std::stringstream ss;
        ss.str(str);

        int index = 0;
        float vec[3] = { 0.f, 0.f, 0.f };
        std::string del;
        while (std::getline(ss, del, ' '))
        {
            vec[index] = std::stof(del);
            index++;
        }

        return { vec[0], vec[1], vec[2] };
    }

    vec2 normalize(const vec2& v)
    {
        float mag = magnitude(v);
        if (mag == 0.f)
            return { 0.f, 0.f };

        return { v.x / mag, v.y / mag };
    }

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

    // not sure if this angle is in rads already
    vec3 RotateVector(const vec3& v, const vec3& rotAxis, float angle)
    {
        vec3 unitRotAxis = normalize(rotAxis);
        
        // check for special case where v and rotAxis are orthogonal
        if (dotProduct(v, rotAxis) == 0.f)
            return cos(angle) * v + sin(angle) * crossProduct(unitRotAxis, v);

        // Rodriquez Rotation Formula
        return (1.f - cos(angle)) * dotProduct(v, unitRotAxis) * unitRotAxis + cos(angle) * crossProduct(unitRotAxis, v);
    }


    // TODO: do a linear interpolation to ensure theta doesn't become 0
    vec3 slerp(const vec3& v_i, const vec3& v_f, float k)
    {
        // theta is in radians
        float theta = acos(dotProduct(normalize(v_i), normalize(v_f)));
        return ((sin((1 - k) * theta) / sin(theta)) * v_i) + ((sin(k * theta) / sin(theta)) * v_f);
    }

    vec4 slerp(const vec4& v_i, const vec4& v_f, float k)
    {
        float theta = acos(dotProduct(normalize(v_i), normalize(v_f)));
        return ((sin((1 - k) * theta) / sin(theta)) * v_i) + ((sin(k * theta) / sin(theta)) * v_f);
    }
}