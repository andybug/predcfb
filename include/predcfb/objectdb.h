#ifndef OBJECTDB_H
#define OBJECTDB_H

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

#define OBJECTDB_MD_SIZE     20
#define OBJECTDB_MD_STR_SIZE 41

typedef struct objectid {
	unsigned char md[OBJECTDB_MD_SIZE];
} objectid;

struct conference;
struct team;

extern enum objectdb_err objectdb_errno;

extern void objectid_print(const objectid *id);
extern void objectid_string(const objectid *id,
                            char buf[OBJECTDB_MD_STR_SIZE]);

extern struct conference *objectdb_create_conference(void);
extern int objectdb_add_conference(struct conference *c, objectid *id);
extern struct conference *objectdb_get_conference(const objectid *id);

extern struct team *objectdb_create_team(void);
extern int objectdb_add_team(struct team *c, objectid *id);
extern struct team *objectdb_get_team(const objectid *id);

extern struct game *objectdb_create_game(void);
extern int objectdb_add_game(struct game *g, objectid *id);
extern struct game *objectdb_get_game(const objectid *id);

extern void objectdb_clear(void);

#endif
