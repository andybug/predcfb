#ifndef PARSE_H
#define PARSE_H

#include "fieldlist.h"

#define PARSE_OK	0
#define PARSE_ERROR	(-1)

enum parse_file_type {
	PARSE_FILE_NONE,
	PARSE_FILE_CSV
};

struct parse_handler {
	const char *file;
	enum parse_file_type type;
	int (*parsing_func)(struct fieldlist *);
};

extern const struct parse_handler parse_handlers[];
extern const int num_parse_handlers;

extern int parse_conference_csv(struct fieldlist *f);

#endif
