#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <errno.h>
#include <string.h>

#include "blob.h"

uint32_t blob_size(blob_t *bp)
{
	assert(bp != NULL);
	return bp->blobsize;
}

uint32_t blob_memory_size(blob_t *bp)
{
	assert(bp != NULL);
	return bp->memsize;
}

int blob_malloc(uint32_t size, blob_t **bpp)
{
	uint32_t s;
	blob_t   *bp;

	*bpp = NULL;
	s    = sizeof(*bp) + size;
	bp   = malloc(s);
	if (bp == NULL) {
		return -ENOMEM;
	}

	bp->memsize  = size;
	bp->blobsize = 0;
	*bpp         = bp;
	return 0;
}

int blob_calloc(uint32_t size, blob_t **bpp)
{
	uint32_t s;
	blob_t   *bp;

	*bpp = NULL;
	s    = sizeof(*bp) + size;
	bp   = calloc(1, s);
	if (bp == NULL) {
		return -ENOMEM;
	}

	bp->memsize = size;
	*bpp        = bp;
	return 0;

}

void blob_free(blob_t *bp)
{
	free(bp);
}

int blob_init(blob_t *bp, const void *data, uint32_t size)
{
	assert(bp != NULL && data != NULL);
	if (size > bp->memsize) {
		return -1;
	}

	memcpy(bp->blob, data, size);
	bp->blobsize = size;
	return 0;
}

int blob_new(void *blobdata, uint32_t size, blob_t **bpp)
{
	int    rc;
	blob_t *bp;

	assert(bpp != NULL);

	*bpp = NULL;
	bp   = NULL;
	rc   = blob_malloc(size, &bp);
	if (rc < 0) {
		return rc;
	}

	if (blobdata != NULL) {
		rc = blob_init(bp, blobdata, size);
		if (rc < 0) {
			blob_free(bp);
			return rc;
		}
	}

	*bpp = bp;
	return 0;
}

int blob_dup(blob_t *bsp, blob_t **bdpp)
{
	return blob_new(bsp->blob, bsp->blobsize, bdpp);
}

int32_t blob_compare(blob_t *bp1, blob_t *bp2)
{
	uint32_t s;
	int      c;

	assert(bp1 != NULL && bp2 != NULL);

	s = (bp1->blobsize < bp2->blobsize) ? bp1->blobsize : bp2->blobsize;
	c = memcmp(bp1->blob, bp2->blob, s);
	if (c != 0) {
		return c;
	}

	return bp1->blobsize - bp2->blobsize;
}

int blob_copy_to_memory(blob_t *bp, void *memory, uint32_t memsize)
{
	assert(bp != NULL && memory != NULL);
	if (bp->blobsize > memsize) {
		return -1;
	}
	memcpy(memory, &bp->blobsize, sizeof(bp->blobsize) + bp->blobsize);
	return 0;
}

void blob_get_pointer(blob_t *bp, void **dpp, uint32_t *dsp)
{
	assert(bp && dpp && dsp);
	*dpp = bp->blob;
	*dsp = bp->blobsize;
}
