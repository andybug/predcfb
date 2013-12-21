
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <assert.h>

#include <polarssl/sha1.h>
#include <predcfb/objectid.h>
#include <predcfb/predcfb.h>

/* objectid display functions */

void objectid_print(const struct objectid *id)
{
	char str[OBJECTID_MD_STR_SIZE];

	objectid_string(id, str);
	fputs(str, stdout);
}

void objectid_string(const struct objectid *id, char buf[OBJECTID_MD_STR_SIZE])
{
	int i, j;
	unsigned int high, low;
	static const char digit_table[] = {
		'0', '1', '2', '3',
		'4', '5', '6', '7',
		'8', '9', 'a', 'b',
		'c', 'd', 'e', 'f'
	};

	for (i = 0, j = 0; i < OBJECTID_MD_SIZE; i++, j+=2) {
		high = (id->md[i] & 0xf0) >> 4;
		low = id->md[i] & 0x0f;

		buf[j] = digit_table[high];
		buf[j+1] = digit_table[low];
	}

	buf[OBJECTID_MD_STR_SIZE - 1] = '\0';
}

/* objectid comparison functions */

bool objectid_compare(const struct objectid *a, const struct objectid *b)
{
	int diff;

	diff = memcmp(a->md, b->md, OBJECTID_MD_SIZE);

	return (diff == 0);
}

/* objectid hashing functions */

void objectid_from_conference(const struct conference *c, struct objectid *id)
{
	size_t len;

	/* just hash name */
	len = strlen(c->name);
	sha1((unsigned char*) c->name, len, id->md);
}

void objectid_from_team(const struct team *t, struct objectid *id)
{
	size_t len;

	/* just hash name */
	len = strlen(t->name);
	sha1((unsigned char*) t->name, len, id->md);
}

void objectid_from_game(const struct game *g, struct objectid *id)
{
	/* date format is YYYY-MM-DD\0 */
	static const int DATE_BUF_SIZE = 11;
	char date_buf[DATE_BUF_SIZE];
	struct tm tm;
	size_t num_bytes;
	sha1_context ctx;
	size_t len;

	assert(g->home != NULL);
	assert(g->away != NULL);

	/* create date string */
	gmtime_r(&g->date, &tm);
	num_bytes = strftime(date_buf, DATE_BUF_SIZE, "%Y-%m-%d", &tm);
	assert(num_bytes == DATE_BUF_SIZE - 1);

	sha1_starts(&ctx);

	/* hash home team name */
	len = strlen(g->home->name);
	sha1_update(&ctx, (unsigned char*)g->home->name, len);
	/* hash away team oid */
	len = strlen(g->away->name);
	sha1_update(&ctx, (unsigned char*)g->away->name, len);
	/* hash time_t */
	sha1_update(&ctx, (unsigned char*)date_buf, DATE_BUF_SIZE - 1);

	sha1_finish(&ctx, id->md);
}

