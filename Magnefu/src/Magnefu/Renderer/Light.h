#pragma once


namespace Magnefu
{
	struct alignas(16) PointLight
	{
        Maths::vec3 LightPos;
        float       padding1;
        Maths::vec3 LightColor;
        float       padding2;
        float       MaxLightDist;
        float       RadiantFlux;
        int         LightEnabled;
        int         padding3;
	};

    struct alignas(16) AreaLight
    {
        float        Intensity;
        Maths::vec3  Color;
        float        padding1;
        Maths::vec3  Translation;
        float        padding2;
        Maths::vec4  Points[4];  // vec3 => vec4 -- last component(4) in each array position is padding
        bool         TwoSided;
        
    };
}