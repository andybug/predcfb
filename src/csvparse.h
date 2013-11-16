#ifndef CSVPARSE_H
#define CSVPARSE_H

#include <stddef.h>

typedef struct csvparse_context csvp_ctx;

extern csvp_ctx *csvp_create(void);
extern int csvp_destroy(csvp_ctx *c);
extern int csvp_parse(csvp_ctx *c, char *buf, size_t len);

#endif
