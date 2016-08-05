#ifndef __BLOB_H__
#define __BLOB_H__

#include <stdint.h>

typedef struct blob {
	uint32_t memsize;
	uint32_t blobsize;
	char     blob[0];
} blob_t;

int  blob_malloc(uint32_t size, blob_t **bpp);
int  blob_calloc(uint32_t size, blob_t **bpp);
int  blob_init(blob_t *bp, const void *data, uint32_t size);
int  blob_new(void *blobdata, uint32_t size, blob_t **bpp);
int  blob_dup(blob_t *bsp, blob_t **bdpp);
int  blob_copy_to_memory(blob_t *bp, void *memory, uint32_t memsize);
void blob_free(blob_t *bp);

uint32_t blob_size(blob_t *bp);
uint32_t blob_memory_size(blob_t *bp);
int32_t  blob_compare(blob_t *bp1, blob_t *bp2);
#endif
