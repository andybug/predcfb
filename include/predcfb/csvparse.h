#ifndef CSVPARSE_H
#define CSVPARSE_H

#include <stddef.h>
#include <libcsv/csv.h>

#define CSVP_OK		  0
#define CSVP_ERROR	(-1)

/* csvline interface */

#define STRBUF_SIZE 1024
#define CSVLINE_MAX_FIELDS 128

struct strbuf {
	char buf[STRBUF_SIZE];
	size_t used;
};

enum csvline_error {
	CSVLINE_ENONE,
	CSVLINE_EMAXFIELDS,
	CSVLINE_ESTRBUFSPACE,
	CSVLINE_ENULLSTR,
	CSVLINE_ERANGE,
	CSVLINE_EWRONGTYPE,
	CSVLINE_EITEREND,
	CSVLINE_EINDEX
};

struct csvline {
	const char *fields[CSVLINE_MAX_FIELDS];
	int num_fields;
	int line;
	int iter;
	struct strbuf strbuf;
	enum csvline_error error;
};

extern const char *strbuf_add(struct strbuf *s, const char *str, size_t len);
extern void strbuf_clear(struct strbuf *s);

extern int csvline_add(struct csvline *c, const char *str, size_t len);
extern void csvline_clear(struct csvline *c);

extern int csvline_str_at(struct csvline *c, int at, const char **out);
extern int csvline_int_at(struct csvline *c, int at, int *out);
extern int csvline_short_at(struct csvline *c, int at, short *out);

extern const char *csvline_strerror(const struct csvline *c);

/* csvparse interface */

enum csvparse_error {
	CSVP_ENONE,
	CSVP_ETOOMANY,
	CSVP_ENOBUFS,
	CSVP_EPARSE,
	CSVP_EINTERNAL
};

struct csvparse {
	struct csv_parser parser;
	int lines;
	struct csvline csvline;
	int (*handler)(struct csvline*);
	enum csvparse_error error;
};

extern int csvp_init(
		struct csvparse *c,
		int (*handler)(struct csvline*));
extern int csvp_destroy(struct csvparse *c);
extern int csvp_parse(struct csvparse *c, char *buf, size_t len);

extern enum csvparse_error csvp_error(const struct csvparse *c);
extern const char *csvp_strerror(const struct csvparse *c);

#endif
