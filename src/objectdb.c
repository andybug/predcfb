
#include <stddef.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <assert.h>

#include <predcfb/predcfb.h>
#include <predcfb/objectid.h>
#include <predcfb/objectdb.h>

#include "objectdb_internal.h"

#define OBJECTDB_MAX_OBJECTS   4096
#define OBJECTDB_MAP_SIZE      2048

static struct object object_table[OBJECTDB_MAX_OBJECTS];
static int num_objects = 0;

static struct conference conferences[CONFERENCE_NUM_MAX];
static int num_conferences = 0;

static struct team teams[TEAM_NUM_MAX];
static int num_teams = 0;

static struct game games[GAME_NUM_MAX];
static int num_games = 0;

static struct object *object_map[OBJECTDB_MAP_SIZE];

enum objectdb_err objectdb_errno = OBJECTDB_ENONE;

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

static struct object **map_get_bin(const struct objectid *id)
{
	static const int32_t mask = OBJECTDB_MAP_SIZE - 1;
	int32_t *bits = (int32_t*) &id->md[16];
	int32_t bin;

	/* assert that map size is a power of two */
	assert(!(OBJECTDB_MAP_SIZE & (OBJECTDB_MAP_SIZE - 1)));

	bin = *bits & mask;

	return (object_map + bin);
}

static struct object *map_lookup(const struct objectid *id)
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
	struct object *cur;
	struct object *prev;

	bin = map_get_bin(&obj->id);

	if (*bin == NULL) {
		/* insert this object at the top of the bin */
		*bin = obj;
	} else {
		/* otherwise, insert at the end of the list */
		cur = *bin;
		prev = NULL;
		while (cur) {
			if (objectid_compare(&obj->id, &cur->id) == true) {
				objectdb_errno = OBJECTDB_EDUPLICATE;
				return OBJECTDB_ERROR;
			}

			prev = cur;
			cur = cur->next;
		}

		assert(prev != NULL);
		prev->next = obj;
	}

	obj->next = NULL;

	return OBJECTDB_OK;
}

/* objectdb create functions */

struct conference *objectdb_create_conference(void)
{
	struct conference *conf;

	if (num_conferences >= CONFERENCE_NUM_MAX) {
		objectdb_errno = OBJECTDB_EMAXCONFS;
		return NULL;
	}

	conf = &conferences[num_conferences];
	num_conferences++;

	return conf;
}

struct team *objectdb_create_team(void)
{
	struct team *team;

	if (num_teams >= TEAM_NUM_MAX) {
		objectdb_errno = OBJECTDB_EMAXTEAMS;
		return NULL;
	}

	team = &teams[num_teams];
	num_teams++;

	return team;
}

struct game *objectdb_create_game(void)
{
	struct game *game;

	if (num_games >= GAME_NUM_MAX) {
		objectdb_errno = OBJECTDB_EMAXGAMES;
		return NULL;
	}

	game = &games[num_games];
	num_games++;

	return game;
}

/* objectdb add functions */

int objectdb_add_conference(struct conference *c, struct objectid *id)
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

int objectdb_add_team(struct team *t, struct objectid *id)
{
	struct object *obj;

	if ((obj = table_new_object()) == NULL)
		return OBJECTDB_ERROR;

	objectid_from_team(t, id);

	obj->id = *id;
	obj->type = OBJECTDB_TEAM;
	obj->data.team = t;

	if (map_insert(obj) != OBJECTDB_OK)
		return OBJECTDB_ERROR;

	return OBJECTDB_OK;
}

int objectdb_add_game(struct game *g, struct objectid *id)
{
	struct object *obj;

	if ((obj = table_new_object()) == NULL)
		return OBJECTDB_ERROR;

	objectid_from_game(g, id);

	obj->id = *id;
	obj->type = OBJECTDB_GAME;
	obj->data.game = g;

	if (map_insert(obj) != OBJECTDB_OK)
		return OBJECTDB_ERROR;

	return OBJECTDB_OK;
}

/* objectdb get functions */

struct conference *objectdb_get_conference(const struct objectid *id)
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

struct team *objectdb_get_team(const struct objectid *id)
{
	struct object *obj;

	if ((obj = map_lookup(id)) == NULL)
		return NULL;

	if (obj->type != OBJECTDB_TEAM) {
		objectdb_errno = OBJECTDB_EWRONGTYPE;
		return NULL;
	}

	return obj->data.team;
}

struct game *objectdb_get_game(const struct objectid *id)
{
	struct object *obj;

	if ((obj = map_lookup(id)) == NULL)
		return NULL;

	if (obj->type != OBJECTDB_GAME) {
		objectdb_errno = OBJECTDB_EWRONGTYPE;
		return NULL;
	}

	return obj->data.game;
}

/* objectdb misc functions */

void objectdb_clear(void)
{
	memset(object_table, 0, sizeof(object_table));
	num_objects = 0;

	memset(conferences, 0, sizeof(conferences));
	num_conferences = 0;

	memset(teams, 0, sizeof(teams));
	num_teams = 0;

	memset(games, 0, sizeof(games));
	num_games = 0;

	memset(object_map, 0, sizeof(object_map));
}

int objectdb_link(void)
{
	int i;
	struct team *team;
	struct conference *conf;
	struct game *game;

	/* set the conference pointer for each team */
	for (i = 0; i < num_teams; i++) {
		team = &teams[i];

		if ((conf = objectdb_get_conference(&team->conf_oid)) == NULL)
			return OBJECTDB_ERROR;

		team->conf = conf;
	}

	/* set the home and away pointers for each game */
	for (i = 0; i < num_games; i++) {
		game = &games[i];

		if ((team = objectdb_get_team(&game->home_oid)) == NULL)
			return OBJECTDB_ERROR;
		game->home = team;

		if ((team = objectdb_get_team(&game->away_oid)) == NULL)
			return OBJECTDB_ERROR;
		game->away = team;
	}

	return OBJECTDB_OK;
}

int objectdb_save(void)
{
	return objectdb_save_yaml(object_table, num_objects);
}
