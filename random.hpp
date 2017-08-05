#ifndef RANDOM_HPP_
#define RANDOM_HPP_

#include "std_types.hpp"

struct Random {
	uint32_t x, y, z, c;
};

void random_seed(Random *random, uint32_t seed);
uint32_t random_get(Random *random);

#endif
