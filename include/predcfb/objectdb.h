#ifndef OBJECTDB_H
#define OBJECTDB_H

#include <predcfb/objectid.h>
#include <predcfb/conference.h>

#define OBJECTDB_OK    0
#define OBJECTDB_ERROR (-1)

extern int objectdb_add_conference(struct conference *c);
extern struct conference *objectdb_get_conference(const objectid *id);

extern void objectdb_clear(void);

#endif
