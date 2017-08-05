#include "random.hpp"

// KISS random number generator by George Marsaglia

void random_seed(Random *random, uint32_t seed) {
	random->x = seed;
	random->y = 362436000;
	random->z = 521288629;
	random->c = 7654321;
}

uint32_t random_get(Random *random) {
	// fetch
	uint32_t x = random->x;
	uint32_t y = random->y;
	uint32_t z = random->z;
	uint32_t c = random->c;

	// linear congruential engine
	random->x = 69069 * random->x + 12345;

	// xor-shift
	y ^= y << 13;
	y ^= y >> 17;
	y ^= y << 5;

	// multiply-with-carry
	uint64_t t = 698769069ULL * z + c;
	c = t >> 32;
	z = (uint32_t) t;
   
	// store
	random->x = x;
	random->y = y;
	random->z = z;
	random->c = c;

	uint32_t result = x + y + z;
	return result;
}
