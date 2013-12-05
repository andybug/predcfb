#ifndef OBJECTID_H
#define OBJECTID_H

#include <stddef.h>
#include <stdbool.h>

#define OBJECTID_MD_SIZE      20
#define OBJECTID_MD_STR_SIZE  41

struct objectid {
	unsigned char md[OBJECTDB_MD_SIZE];
};

typedef struct oid_context oid_ctx;

extern bool objectid_compare(const struct objectid *a,
                             const struct objectid *b);
extern void objectid_print(const struct objectid *id);
extern void objectid_string(const struct objectid *id,
                            char buf[OBJECTDB_MD_STR_SIZE]);

extern void objectid_create(const void *data, size_t len,
                            struct objectid *out);
extern oid_ctx *objectid_begin(void);
extern void objectid_update(oid_ctx *ctx, const void *data, size_t len);
extern void objectid_finish(oid_ctx *ctx, struct objectid *out);

#endif
