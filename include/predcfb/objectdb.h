#ifndef OBJECTDB_H
#define OBJECTDB_H

#include <predcfb/conference.h>

#define OBJECTDB_OK    0
#define OBJECTDB_ERROR (-1)

#define OBJECTDB_MD_SIZE 20

typedef struct objectid {
	unsigned char md[OBJECTDB_MD_SIZE];
} objectid;

extern int objectdb_add_conference(struct conference *c, objectid *id);
extern struct conference *objectdb_get_conference(const objectid *id);

extern void objectdb_clear(void);

#endif
