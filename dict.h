#ifndef __DICT_H__
#define __DICT_H__

#include <stdint.h>
#include "blob.h"

#define DEFAULT_BUCKETS 4

typedef struct hash_entry {
	struct hash_entry *next;
	blob_t            *keyp;
	blob_t            *valuep;
} hash_entry_t;

typedef struct hash {
	uint32_t     nbuckets;
	uint32_t     nelements;
	hash_entry_t *buckets[0];
} hash_t;

typedef struct dict_hash {
	hash_t   *h[2];
	uint32_t nbuckets;
	uint32_t rehash_bucket;
	int8_t   rehashing;
	int8_t   index;
} dict_hash_t;

typedef struct dict {
	uint64_t    maxmemory;
	dict_hash_t hash;
} dict_t;

int dict_init(dict_t *dictp, uint64_t maxmemory);
void dict_deinit(dict_t *dictp);

int dict_add(dict_t *dictp, void *key, uint32_t keysize,
		void *value, uint32_t valuesize);
#if 0
int dict_find(dict_t *dictp, void *key, uint32_t keysize,
		void *value, uint32_t valuesize);
int dict_delete(dict_t *dictp, void *key, uint32_t keysize);
#endif
#endif
