#ifndef OBJECTDB_H
#define OBJECTDB_H

#include <predcfb/conference.h>

#define OBJECTDB_OK    0
#define OBJECTDB_ERROR (-1)

#define OBJECTDB_MD_SIZE     20
#define OBJECTDB_MD_STR_SIZE 41

typedef struct objectid {
	unsigned char md[OBJECTDB_MD_SIZE];
} objectid;

extern void objectid_print(const objectid *id);
extern void objectid_string(const objectid *id, char buf[OBJECTDB_MD_STR_SIZE]);

extern int objectdb_add_conference(struct conference *c, objectid *id);
extern struct conference *objectdb_get_conference(const objectid *id);

extern void objectdb_clear(void);

#endif
