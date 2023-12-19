#pragma once

// -- other includes -- //
#include <random>

namespace Magnefu
{
	class NumGen
	{
	public:
		static int GetRandomNumber(int min, int max);

	private:
		static std::random_device m_RandDevice;
		static std::mt19937 m_Generator;

	};
}