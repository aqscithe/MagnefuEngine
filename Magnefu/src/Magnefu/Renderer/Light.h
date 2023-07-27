#pragma once


namespace Magnefu
{
	struct alignas(16) Light
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
}