#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <assert.h>

#include "dict.h"

static void test_dict_add(void)
{
	int      rc;
	dict_t   d;
	char     k[32];
	char     v[4096];
	uint32_t i;
	char     c;

	rc = dict_init(&d, 10);
	assert(rc == 0);

	for (i = 0; i < 1000ul; i++) {
		snprintf(k, sizeof(k), "%d", i);
		k[sizeof(k) - 1] = 0;

		c = 'a' + (i % 'z');
		memset(v, c, sizeof(v));

		rc = dict_add(&d, k, strlen(k), v, sizeof(v));
		assert(rc == 0);
	}

	dict_deinit(&d);
}

int main()
{
	test_dict_add();
	return 0;
}
