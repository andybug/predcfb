#ifndef CSVPARSE_H
#define CSVPARSE_H

#include <stddef.h>
#include <libcsv/csv.h>
#include <predcfb/fieldlist.h>

#define CSVP_OK		  0
#define CSVP_ERROR	(-1)

enum csvp_error {
	CSVP_ENONE,
	CSVP_ENOMEM,
	CSVP_ETOOMANY,
	CSVP_ENOBUFS,
	CSVP_EPARSE,
	CSVP_EINTERNAL
};

struct csvparse {
	struct csv_parser parser;
	int lines;
	struct fieldlist fieldlist;
	int (*handler)(struct fieldlist*);
	enum csvp_error error;
};

extern int csvp_init(
		struct csvparse *c,
		int (*handler)(struct fieldlist*));
extern int csvp_destroy(struct csvparse *c);
extern int csvp_parse(struct csvparse *c, char *buf, size_t len);

extern enum csvp_error csvp_error(const struct csvparse *c);
extern const char *csvp_strerror(const struct csvparse *c);

#endif
