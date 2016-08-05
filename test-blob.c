#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "blob.h"

#define BUF_SIZE 4096

static void test_blob_new(void)
{
	int      rc;
	char     a[BUF_SIZE];
	blob_t   *bp;
	uint32_t ms;
	blob_t   *dupbp;
	int32_t  c;

	memset(a, 'a', sizeof(a));

	bp = NULL;
	rc = blob_new(a, sizeof(a)-1, &bp);
	assert(rc == 0 && bp != NULL);
	assert(blob_size(bp) == sizeof(a) - 1);
	ms = blob_memory_size(bp);
	assert(blob_size(bp) == ms);

	rc = blob_init(bp, a, sizeof(a));
	assert(rc != 0);
	assert(blob_size(bp) == sizeof(a) - 1);
	assert(blob_size(bp) == ms);
	assert(ms == blob_memory_size(bp));

	rc = blob_init(bp, a, 1);
	assert(rc == 0);
	assert(blob_size(bp) == 1);
	assert(ms == blob_memory_size(bp));

	dupbp = NULL;
	rc = blob_dup(bp, &dupbp);
	assert(rc == 0 && dupbp != NULL);
	assert(blob_size(dupbp) == 1);
	assert(blob_memory_size(dupbp) == 1);

	c = blob_compare(bp, dupbp);
	assert(c == 0);
	c = blob_compare(dupbp, bp);
	assert(c == 0);

	rc = blob_init(bp, a, sizeof(a));
	assert(rc != 0);
	assert(blob_size(bp) == 1);
	assert(ms == blob_memory_size(bp));

	rc = blob_init(bp, a, sizeof(a)-1);
	assert(rc == 0);
	assert(blob_size(bp) == sizeof(a) - 1);
	assert(blob_size(bp) == ms);
	assert(ms == blob_memory_size(bp));

	c = blob_compare(bp, dupbp);
	assert(c > 0);
	assert(c == blob_size(bp) - blob_size(dupbp));

	c = blob_compare(dupbp, bp);
	assert(c < 0);
	assert(c == blob_size(dupbp) - blob_size(bp));

	blob_free(bp);
	bp = NULL;
	blob_free(dupbp);
	dupbp = NULL;
}

int main()
{
	test_blob_new();
	return 0;
}
