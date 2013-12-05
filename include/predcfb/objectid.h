#ifndef OBJECTID_H
#define OBJECTID_H

#include <stddef.h>
#include <stdbool.h>

#define OBJECTID_MD_SIZE      20
#define OBJECTID_MD_STR_SIZE  41

struct objectid {
	unsigned char md[OBJECTID_MD_SIZE];
};

typedef struct oid_context oid_ctx;

/* return true if objectid a and b are equal */
extern bool objectid_compare(const struct objectid *a,
                             const struct objectid *b);

/* print objectid to stdout in hex format, no newline */
extern void objectid_print(const struct objectid *id);

/* write hex form of objectid to buf */
extern void objectid_string(const struct objectid *id,
                            char buf[OBJECTID_MD_STR_SIZE]);

/* need this to avoid circular dependencies */
struct conference;
struct team;
struct game;

/* create an objectid from a conference */
extern void objectid_from_conference(const struct conference *c,
                                     struct objectid *id);

/* create an objectid from a team */
extern void objectid_from_team(const struct team *t, struct objectid *id);

/* create an objectid from a game */
extern void objectid_from_game(const struct game *g, struct objectid *id);

#endif
