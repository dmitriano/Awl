#pragma once

#include <random>

namespace awl
{
	inline std::mt19937 & random()
	{
		static thread_local std::mt19937 random_engine(std::random_device{}());

		return random_engine;
	}
}
