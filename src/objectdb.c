
#include <stddef.h>
#include <string.h>
#include <stdio.h>

#include <polarssl/sha1.h>
#include <predcfb/objectdb.h>

/* objectid display functions */

void objectid_print(const objectid *id)
{
	char str[OBJECTDB_MD_STR_SIZE];

	objectid_string(id, str);
	fputs(str, stdout);
}

void objectid_string(const objectid *id, char buf[OBJECTDB_MD_STR_SIZE])
{
	int i, j;
	unsigned int high, low;
	static const char digit_table[] = {
		'0', '1', '2', '3',
		'4', '5', '6', '7',
		'8', '9', 'a', 'b',
		'c', 'd', 'e', 'f'
	};

	for (i = 0, j = 0; i < OBJECTDB_MD_SIZE; i++, j+=2) {
		high = (id->md[i] & 0xf0) >> 4;
		low = id->md[i] & 0x0f;

		buf[j] = digit_table[high];
		buf[j+1] = digit_table[low];
	}

	buf[OBJECTDB_MD_STR_SIZE - 1] = '\0';
}

/* objectid hashing functions */

static void objectid_from_conference(const struct conference *c, objectid *id)
{
	size_t len;
	sha1_context ctx;

	sha1_starts(&ctx);

	/* hash name */
	len = strlen(c->name);
	sha1_update(&ctx, (unsigned char*) c->name, len);

	/* hash div */
	len = sizeof(c->div);
	sha1_update(&ctx, (unsigned char*) &c->div, len);

	sha1_finish(&ctx, id->md);
}

/* objectdb add and get functions */

int objectdb_add_conference(struct conference *c, objectid *id)
{
	objectid_from_conference(c, id);

	return OBJECTDB_OK;
}

struct conference *objectdb_get_conference(const objectid *id)
{
	return NULL;
}

/* objectdb misc functions */

void objectdb_clear(void)
{
}
