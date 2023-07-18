#pragma once


namespace Magnefu
{
	struct Light
	{
        Maths::vec3 LightPos;
        Maths::vec3 LightColor;
        float       MaxLightDist;
        float       RadiantFlux;
        int         LightEnabled;
	};
}