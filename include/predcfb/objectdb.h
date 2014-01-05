#ifndef OBJECTDB_H
#define OBJECTDB_H

#include <predcfb/objectid.h>
#include <predcfb/predcfb.h>

#define OBJECTDB_OK       0
#define OBJECTDB_ERROR  (-1)

enum objectdb_err {
	OBJECTDB_ENONE,
	OBJECTDB_EMAXOBJECTS,
	OBJECTDB_EMAXCONFS,
	OBJECTDB_EMAXTEAMS,
	OBJECTDB_EMAXGAMES,
	OBJECTDB_ENOTFOUND,
	OBJECTDB_EWRONGTYPE,
	OBJECTDB_EDUPLICATE
};

extern enum objectdb_err objectdb_errno;

extern struct conference *objectdb_create_conference(void);
extern int objectdb_add_conference(struct conference *c, struct objectid *id);
extern struct conference *objectdb_get_conference(const struct objectid *id);

extern struct team *objectdb_create_team(void);
extern int objectdb_add_team(struct team *c, struct objectid *id);
extern struct team *objectdb_get_team(const struct objectid *id);

extern struct game *objectdb_create_game(void);
extern int objectdb_add_game(struct game *g, struct objectid *id);
extern struct game *objectdb_get_game(const struct objectid *id);

/* return the list of games and set num_games */
extern struct game *objectdb_get_games(int *num_games);

extern void objectdb_clear(void);

extern int objectdb_write(void);

#endif
