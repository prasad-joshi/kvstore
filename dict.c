#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <assert.h>

#include "dict.h"
#include "blob.h"
#include "murmur3.h"

/*
 * BASIC HASH IMPLEMENTATION
 */
uint32_t hash_nelements(hash_t *hashp)
{
	return hashp->nelements;
}

uint32_t hash_nbuckets(hash_t *hashp)
{
	return hashp->nbuckets;
}

int hash_init(hash_t **hashpp, uint32_t nbuckets)
{
	uint32_t s;
	hash_t   *hp;

	assert((nbuckets & (nbuckets - 1)) == 0);

	s  = sizeof(*hp) + sizeof(hash_entry_t *) * nbuckets;
	hp = calloc(1, s);
	if (hp == NULL) {
		return -ENOMEM;
	}

	hp->nbuckets = nbuckets;
	*hashpp      = hp;
	return 0;
}

void hash_bucket_deinit(struct hash_entry *head)
{
	hash_entry_t *e;
	blob_t       *kbp;
	blob_t       *vbp;

	while (head != NULL) {
		e = head->next;
		blob_free(head->keyp);
		blob_free(head->valuep);
		free(head);
		head = e;
	}
}

void hash_deinit(hash_t *hashp)
{
	uint32_t i;

	if (hashp == NULL) {
		return;
	}

	if (hash_nelements(hashp) > 0) {
		for (i = 0; i < hashp->nbuckets; i++) {
			hash_bucket_deinit(hashp->buckets[i]);
		}
	}

	free(hashp);
}

int hash_entry_new(hash_entry_t **entrypp, blob_t *keyp, blob_t *valuep)
{
	hash_entry_t *ep;

	ep = malloc(sizeof(*ep));
	if (ep == NULL) {
		return -ENOMEM;
	}

	ep->keyp   = keyp;
	ep->valuep = valuep;
	*entrypp   = ep;
	return 0;
}

int hash_entry_free(hash_entry_t *entryp)
{
	free(entryp);
}

void _hash_entry_add(hash_t *hashp, hash_entry_t *nep)
{
	void         *k;
	uint32_t     s;
	uint32_t     hash;
	uint32_t     index;
	hash_entry_t *ep;

	assert(hashp && nep);

	blob_get_pointer(nep->keyp, &k, &s);
	assert(k != NULL && s != 0);

	hash  = murmur3_x86_32(k, s);
	index = hash & (hashp->nbuckets - 1);
	assert(index < hashp->nbuckets);

	ep                    = hashp->buckets[index];
	nep->next             = ep;
	hashp->buckets[index] = nep;
	hashp->nelements++;
}

int hash_entry_add(hash_t *hashp, blob_t *keyp, blob_t *valuep)
{
	hash_entry_t *nep;
	int          rc;

	assert(hashp && keyp && valuep);

	nep = NULL;
	rc  = hash_entry_new(&nep, keyp, valuep);
	if (rc < 0) {
		assert(nep == NULL);
		return rc;
	}
	assert(nep != NULL);

	_hash_entry_add(hashp, nep);
	return 0;
}

void hash_entries_move(hash_t *shashp, hash_t *dhashp, uint32_t bucket)
{
	hash_entry_t *ep;
	hash_entry_t *nep;

	assert(shashp && dhashp);
	assert(hash_nbuckets(shashp) > bucket);

	ep = shashp->buckets[bucket];
	while (ep != NULL) {
		nep = ep->next;
		_hash_entry_add(dhashp, ep);
		shashp->nelements--;
		ep = nep;
	}
	shashp->buckets[bucket] = NULL;
}

/*
 * DICTIONARY HASH IMPLEMENTATION
 */
int dict_hash_init(dict_hash_t *dhp)
{
	int    rc;
	hash_t *h;

	assert(dhp);

	h  = NULL;
	rc = hash_init(&h, DEFAULT_BUCKETS);
	if (rc < 0) {
		assert(h == NULL);
		return rc;
	}
	assert(h != NULL);

	memset(dhp, 0, sizeof(*dhp));
	dhp->h[0]     = h;
	dhp->nbuckets = DEFAULT_BUCKETS << 1;
	return 0;
}

int dict_hash_deinit(dict_hash_t *dhp)
{
	int i;

	for (i = 0; i < 2; i++) {
		hash_deinit(dhp->h[i]);
	}
	memset(dhp, 0, sizeof(*dhp));
}

static int dict_hash_rehash(dict_hash_t *dhp, int nrehash)
{
	hash_t   *h0;
	hash_t   *h1;
	uint32_t i;
	int      rc;

	assert(dhp);

	if (dhp->rehashing == 0) {
		return 0;
	}

	/* re-hash elements from dhp>h[0] to dhp->h[1] */
	h0 = dhp->h[0];
	assert(h0 != NULL);

	h1 = dhp->h[1];
	if (h1 == NULL) {
		/* create new hash - h1 */
		rc = hash_init(&h1, dhp->nbuckets << 1);
		if (rc < 0) {
			assert(h1 == NULL);
			return rc;
		}
		assert(h1 != NULL);

		dhp->nbuckets      = dhp->nbuckets << 1;
		dhp->h[1]          = h1;
		/* insert new elements in h1 */
		dhp->index         = 1;
		/* rehash from bucket 0 of h0 */
		dhp->rehash_bucket = 0;
	}

	nrehash = (nrehash < hash_nbuckets(h0)) ?
				nrehash : hash_nbuckets(h0);
	while (nrehash > 0 && hash_nelements(h0) > 0) {
		i = dhp->rehash_bucket;
		for (; i < hash_nbuckets(h0); i++) {
			if (h0->buckets[i] != NULL) {
				break;
			}
		}
		assert(i < hash_nbuckets(h0));

		hash_entries_move(h0, h1, i);
		dhp->rehash_bucket = i;
		nrehash--;
	}

	if (hash_nelements(h0) == 0) {
		/* rehashing completed */
		hash_deinit(h0);
		dhp->h[0] = h1;
		dhp->h[1] = NULL;
		dhp->rehashing     = 0;
		dhp->rehash_bucket = 0;
		dhp->index         = 0; /* insert new elements in h0 */
		h0 = h1 = NULL;
	}

	return 0;
}

int dict_hash_add(dict_hash_t *dhp, blob_t *kbp, blob_t *vbp)
{
	hash_t   *hp;
	int      index;
	uint32_t ne;
	int      rc;

	if (dhp->rehashing != 0) {
		/*
		 * remove entries from one bucket of h[0] and add it to h[1]
		 */
		dict_hash_rehash(dhp, 1);
	}

	assert(dhp->index < 2);

	hp = dhp->h[dhp->index];
	assert(hp != NULL);

	rc = hash_entry_add(hp, kbp, vbp);
	if (rc < 0) {
		return rc;
	}

	if (dhp->rehashing == 0) {
		hp = dhp->h[0];
		ne = hash_nelements(hp) >> 2;
		if (ne > dhp->nbuckets) {
			dhp->rehashing = 1;
		}
	}
	return 0;
}

int dict_init(dict_t *dictp, uint64_t maxmemory)
{
	int rc;

	assert(dictp != NULL);

	memset(dictp, 0, sizeof(*dictp));
	rc = dict_hash_init(&dictp->hash);
	if (rc < 0) {
		return rc;
	}
	dictp->maxmemory = maxmemory;
	return 0;
}

void dict_deinit(dict_t *dictp)
{
	dict_hash_deinit(&dictp->hash);
	memset(dictp, 0, sizeof(*dictp));
}

int dict_add(dict_t *dictp, void *key, uint32_t keysize,
		void *value, uint32_t valuesize)
{
	int    rc;
	blob_t *kbp;
	blob_t *vbp;

	assert(dictp);

	rc = blob_new(key, keysize, &kbp);
	if (rc < 0) {
		return rc;
	}

	rc = blob_new(value, valuesize, &vbp);
	if (rc < 0){
		goto error;
	}

	rc = dict_hash_add(&dictp->hash, kbp, vbp);
	if (rc < 0) {
		goto error;
	}

	return 0;
error:
	if (kbp != NULL) {
		blob_free(kbp);
		kbp = NULL;
	}

	if (vbp != NULL) {
		blob_free(vbp);
		vbp = NULL;
	}
	return rc;
}
