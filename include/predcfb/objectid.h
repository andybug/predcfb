#ifndef OBJECTID_H
#define OBJECTID_H

#include <predcfb/conference.h>

#define OBJECTID_OK    0
#define OBJECTID_ERROR (-1)

typedef struct objectid objectid;

extern int objectid_conference(const struct conference *c, objectid *o);
extern int objectid_compare(const objectid *a, const objectid *b);

#endif
