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
        Maths::vec3  Color;
        float        padding1;
        Maths::vec3  Translation;
        float        padding2;
        float        Intensity;
        int          TwoSided;   // 0 or 1
        
    };
}