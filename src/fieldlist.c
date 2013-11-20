
#include <string.h>

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

/* field_list functions */

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
	f->iter = 0;
	f->error = FIELDLIST_ENONE;
}

const char *fieldlist_iter_begin(struct fieldlist *f)
{
	f->iter = 0;

	return fieldlist_iter_next(f);
}

const char *fieldlist_iter_next(struct fieldlist *f)
{
	const char *field;

	if (f->iter >= f->num_fields)
		return NULL;

	field = f->fields[f->iter];
	f->iter++;

	return field;
}
