#include <stdint.h>
#include "murmur3.h"

/*
 * Implementation copied from MurmurHash Wiki page at 
 * https://en.wikipedia.org/wiki/MurmurHash
 */

#define ROT32(x, y) ((x << y) | (x >> (32 - y)))
uint32_t murmur3_x86_32(const void *key, uint32_t len)
{
	static const uint32_t c1   = 0xcc9e2d51;
	static const uint32_t c2   = 0x1b873593;
	static const uint32_t r1   = 15;
	static const uint32_t r2   = 13;
	static const uint32_t m    = 5;
	static const uint32_t n    = 0xe6546b64;
	static const uint32_t seed = 5381; /* default seed */

	uint32_t       hash    = seed;
	const uint32_t *blocks = (const uint32_t *) key;
	const int      nblocks = len / 4;

	int      i;
	uint32_t k;

	/* body */
	for (i = 0; i <	nblocks; i++) {
		k  = blocks[i];
		k *= c1;
		k  = ROT32(k, r1);
		k *= c2;

		hash ^= k;
		hash  = ROT32(hash, r2) * m + n;
	}

	/*
	 * No swapping for tail part as we assume machine is x86 or
	 * little endian.
	 */
	const uint8_t *tail = (const uint8_t *) (key + nblocks * 4);
	k = 0;
	switch (len & 3) {
	case 3:
		k ^= tail[2] << 16;
	case 2:
		k ^= tail[1] << 8;
	case 1:
		k ^= tail[0];

		k    *= c1;
		k     = ROT32(k, r1);
		k    *= c2;
		hash ^= k;
	}

	/* finalization */
	hash ^= len;
	hash ^= (hash >> 16);
	hash *= 0x85ebca6b;
	hash ^= (hash >> 13);
	hash *= 0xc2b2ae35;
	hash ^= (hash >> 16);

	return hash;
}
