#ifndef CSVPARSE_H
#define CSVPARSE_H

#include <stddef.h>
#include <predcfb/parse.h>

#define CSVP_OK		0
#define CSVP_ERROR	(-1)

typedef struct csvparse_context csvp_ctx;

enum csvp_error {
	CSVP_ENONE,
	CSVP_ENOMEM,
	CSVP_ETOOMANY,
	CSVP_ENOBUFS,
	CSVP_EPARSE
};

extern csvp_ctx *csvp_create(const struct parse_handler *handler);
extern int csvp_destroy(csvp_ctx *c);
extern int csvp_parse(csvp_ctx *c, char *buf, size_t len);

#endif
