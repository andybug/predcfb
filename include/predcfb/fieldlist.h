#ifndef FIELDLIST_H
#define FIELDLIST_H

#include <stddef.h>

#define STRBUF_SIZE 1024

#define FIELDLIST_MAX_FIELDS 32
#define FIELDLIST_ITER_FIRST (-1)

#define FIELDLIST_OK      0
#define FIELDLIST_ERROR (-1)

struct strbuf {
	char buf[STRBUF_SIZE];
	size_t used;
};

enum fieldlist_error {
	FIELDLIST_ENONE,
	FIELDLIST_EMAXFIELDS,
	FIELDLIST_ESTRBUFSPACE,
	FIELDLIST_ENULLSTR,
	FIELDLIST_ERANGE,
	FIELDLIST_EWRONGTYPE,
	FIELDLIST_EITEREND
};

struct fieldlist {
	const char *fields[FIELDLIST_MAX_FIELDS];
	int num_fields;
	int iter;
	struct strbuf strbuf;
	enum fieldlist_error error;
};

extern const char *strbuf_add(struct strbuf *s, const char *str, size_t len);
extern void strbuf_clear(struct strbuf *s);

extern int fieldlist_add(struct fieldlist *f, const char *str, size_t len);
extern void fieldlist_clear(struct fieldlist *f);

extern const char *fieldlist_strerror(struct fieldlist *f);

extern void fieldlist_iter_begin(struct fieldlist *f);
extern const char *fieldlist_iter_next(struct fieldlist *f);
/*
 * the following functions get the next object from the fieldlist
 * and attempts to convert them to the specified type
 */
extern int fieldlist_iter_next_int(struct fieldlist *f, int *out);
extern int fieldlist_iter_next_short(struct fieldlist *f, short *out);

#endif
