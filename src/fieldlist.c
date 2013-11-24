
#include <stdlib.h>
#include <string.h>
#include <limits.h>

#include <predcfb/fieldlist.h>

/* strbuf functions */

const char *strbuf_add(struct strbuf *s, const char *str, size_t len)
{
	size_t total_len;
	size_t new_used;
	const char *alloc_str;

	total_len = len + 1;
	new_used = s->used + total_len;
	if (new_used > STRBUF_SIZE)
		return NULL;

	alloc_str = memcpy(s->buf + s->used, str, len);
	s->buf[s->used + len] = '\0';
	s->used = new_used;

	return alloc_str;
}

void strbuf_clear(struct strbuf *s)
{
	s->used = 0;
}

/* fieldlist functions */

int fieldlist_add(struct fieldlist *f, const char *str, size_t len)
{
	const char *strbuf_str;

	if (!str) {
		f->error = FIELDLIST_ENULLSTR;
		return FIELDLIST_ERROR;
	}

	if (f->num_fields >= FIELDLIST_MAX_FIELDS) {
		f->error = FIELDLIST_EMAXFIELDS;
		return FIELDLIST_ERROR;
	}

	strbuf_str = strbuf_add(&f->strbuf, str, len);
	if (!strbuf_str) {
		f->error = FIELDLIST_ESTRBUFSPACE;
		return FIELDLIST_ERROR;
	}

	f->fields[f->num_fields] = strbuf_str;
	f->num_fields++;

	return FIELDLIST_OK;
}

void fieldlist_clear(struct fieldlist *f)
{
	strbuf_clear(&f->strbuf);
	f->num_fields = 0;
	f->iter = FIELDLIST_ITER_FIRST;
	f->error = FIELDLIST_ENONE;
}

void fieldlist_iter_begin(struct fieldlist *f)
{
	f->iter = FIELDLIST_ITER_FIRST;
}

const char *fieldlist_iter_next(struct fieldlist *f)
{
	const char *field;

	f->iter++;
	if (f->iter >= f->num_fields) {
		f->error = FIELDLIST_EITEREND;
		return NULL;
	} else if (f->iter < 0) {
		f->error = FIELDLIST_ERANGE;
		return NULL;
	}

	field = f->fields[f->iter];
	return field;
}

int fieldlist_iter_next_int(struct fieldlist *f, int *out)
{
	long int li;
	char *endptr;
	const char *str;

	if ((str = fieldlist_iter_next(f)) == NULL)
		return FIELDLIST_ERROR;

	li = strtol(str, &endptr, 10);

	if (*endptr != '\0') {
		/*
		 * endptr points at the first bad character, so if it
		 * points at anything other than the null byte, there
		 * was an error trying to parse the string
		 */
		f->error = FIELDLIST_EWRONGTYPE;
		return FIELDLIST_ERROR;
	}

	if (li < INT_MIN || li > INT_MAX) {
		f->error = FIELDLIST_ERANGE;
		return FIELDLIST_ERROR;
	}

	*out = (int) li;

	return FIELDLIST_OK;
}
