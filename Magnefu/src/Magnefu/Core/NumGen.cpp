// - PCH -- //
#include "mfpch.h"

// -- heade r-- //
#include "NumGen.h"

namespace Magnefu
{
	std::random_device NumGen::m_RandDevice;
	std::mt19937 NumGen::m_Generator(NumGen::m_RandDevice());

	int NumGen::GetRandomNumber(int min, int max)
	{
		std::uniform_int_distribution<> dis(min, max);
		return dis(m_Generator);
	}
}