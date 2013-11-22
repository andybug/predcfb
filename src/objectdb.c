
#include <stddef.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <assert.h>

#include <polarssl/sha1.h>
#include <predcfb/objectdb.h>

#define OBJECTDB_MAX_OBJECTS   4096
#define OBJECTDB_MAP_SIZE     (1<<11)

enum object_type {
	OBJECTDB_CONF,
	OBJECTDB_TEAM,
	OBJECTDB_GAME,
	OBJECTDB_BLOB
};

union object_data {
	struct conference *conf;
};

struct object {
	objectid id;
	enum object_type type;
	union object_data data;
	struct object *next;
};

static struct object object_table[OBJECTDB_MAX_OBJECTS];
static int num_objects = 0;

static struct object *object_map[OBJECTDB_MAP_SIZE];

enum objectdb_err objectdb_errno = OBJECTDB_ENONE;

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

/* objectid compare functions */

static bool objectid_compare(const objectid *a, const objectid *b)
{
	int i;

	for (i = 0; i < OBJECTDB_MD_SIZE; i++) {
		if (a->md[i] != b->md[i])
			return false;
	}

	return true;
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

/* object table functions */

static struct object *table_new_object(void)
{
	struct object *o;

	if (num_objects >= OBJECTDB_MAX_OBJECTS) {
		objectdb_errno = OBJECTDB_EMAXOBJECTS;
		return NULL;
	}

	o = &object_table[num_objects];
	num_objects++;

	return o;
}

/* object map functions */

static struct object **map_get_bin(const objectid *id)
{
	static const int32_t mask = OBJECTDB_MAP_SIZE - 1;
	int32_t *bits = (int32_t*) &id->md[16];
	int32_t bin;

	/* assert that map size is a power of two */
	assert(OBJECTDB_MAP_SIZE == (OBJECTDB_MAP_SIZE & (~OBJECTDB_MAP_SIZE) + 1));

	bin = *bits & mask;

	return (object_map + bin);
}

static struct object *map_lookup(const objectid *id)
{
	struct object **bin;
	struct object *obj;

	bin = map_get_bin(id);

	if (*bin == NULL) {
		objectdb_errno = OBJECTDB_ENOTFOUND;
		return NULL;
	}

	obj = *bin;

	while (obj) {
		if (objectid_compare(id, &obj->id) == true)
			return obj;

		obj = obj->next;
	}

	objectdb_errno = OBJECTDB_ENOTFOUND;

	return NULL;
}

static int map_insert(struct object *obj)
{
	struct object **bin;
	struct object *current;

	bin = map_get_bin(&obj->id);

	if (*bin == NULL) {
		/* insert this object at the top of the bin */
		*bin = obj;
	} else {
		/* otherwise, insert at the end of the list */
		current = *bin;
		while (current->next) {
			current = current->next;
		}

		current->next = obj;
	}

	obj->next = NULL;

	return OBJECTDB_OK;
}

/* objectdb add and get functions */

int objectdb_add_conference(struct conference *c, objectid *id)
{
	struct object *obj;

	if ((obj = table_new_object()) == NULL)
		return OBJECTDB_ERROR;

	objectid_from_conference(c, id);

	obj->id = *id;
	obj->type = OBJECTDB_CONF;
	obj->data.conf = c;

	if (map_insert(obj) != OBJECTDB_OK)
		return OBJECTDB_ERROR;

	return OBJECTDB_OK;
}

struct conference *objectdb_get_conference(const objectid *id)
{
	struct object *obj;

	if ((obj = map_lookup(id)) == NULL)
		return NULL;

	if (obj->type != OBJECTDB_CONF) {
		objectdb_errno = OBJECTDB_EWRONGTYPE;
		return NULL;
	}

	return obj->data.conf;
}

/* objectdb misc functions */

void objectdb_clear(void)
{
	memset(object_table, 0, sizeof(object_table));
	num_objects = 0;

	memset(object_map, 0, sizeof(object_map));
}
