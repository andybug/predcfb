#ifndef PARSE_H
#define PARSE_H

#define PARSE_OK		0
#define PARSE_MEMORY_ERROR	(-1)
#define PARSE_INTERNAL_ERROR	(-2)

typedef void* parse_ctx;

extern int parse_init(parse_ctx *p, const char *file);
extern int parse_finish(parse_ctx p);
extern int parse_data(parse_ctx p, const char *buf, size_t len);

#endif
