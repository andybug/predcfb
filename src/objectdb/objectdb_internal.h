#ifndef OBJECTDB_INTERNAL_H
#define OBJECTDB_INTERNAL_H

enum object_type {
	OBJECTDB_CONF,
	OBJECTDB_TEAM,
	OBJECTDB_GAME,
	OBJECTDB_BLOB
};

union object_data {
	struct conference *conf;
	struct team *team;
	struct game *game;
};

struct object {
	struct objectid id;
	enum object_type type;
	union object_data data;
	struct object *next;
};

extern int objectdb_write_yaml(const struct object *objects, int num_objects);

#endif
