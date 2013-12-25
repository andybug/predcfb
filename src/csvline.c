
#include <stdlib.h>
#include <string.h>
#include <limits.h>

#include <predcfb/csvparse.h>

static const char *error_strings[] = {
	"No error",
	"Maximum fields reached",
	"String buffer out of space",
	"Null string received as input",
	"Value not in range",
	"Value is not expected type",
	"Iterator is at the end",
	"Index is out of bounds"
};

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

/* csvline functions */

int csvline_add(struct csvline *c, const char *str, size_t len)
{
	const char *strbuf_str;

	if (!str) {
		c->error = CSVLINE_ENULLSTR;
		return CSVP_ERROR;
	}

	if (c->num_fields >= CSVLINE_MAX_FIELDS) {
		c->error = CSVLINE_EMAXFIELDS;
		return CSVP_ERROR;
	}

	strbuf_str = strbuf_add(&c->strbuf, str, len);
	if (!strbuf_str) {
		c->error = CSVLINE_ESTRBUFSPACE;
		return CSVP_ERROR;
	}

	c->fields[c->num_fields] = strbuf_str;
	c->num_fields++;

	return CSVP_OK;
}

void csvline_clear(struct csvline *c)
{
	strbuf_clear(&c->strbuf);
	c->num_fields = 0;
	c->line = 0;
	c->error = CSVLINE_ENONE;
}

int csvline_str_at(struct csvline *c, int at, const char **out)
{
	if (at < 0 || at >= c->num_fields) {
		c->error = CSVLINE_EINDEX;
		return CSVP_ERROR;
	}
	
	*out = c->fields[at];

	return CSVP_OK;
}

int csvline_int_at(struct csvline *c, int at, int *out)
{
	long int li;
	char *endptr;
	const char *str;

	if (at < 0 || at >= c->num_fields) {
		c->error = CSVLINE_EINDEX;
		return CSVP_ERROR;
	}

	str = c->fields[at];
	li = strtol(str, &endptr, 10);

	if (*endptr != '\0') {
		/*
		 * endptr points at the first bad character, so if it
		 * points at anything other than the null byte, there
		 * was an error trying to parse the string
		 */
		c->error = CSVLINE_EWRONGTYPE;
		return CSVP_ERROR;
	}

	if (li < INT_MIN || li > INT_MAX) {
		c->error = CSVLINE_ERANGE;
		return CSVP_ERROR;
	}

	*out = (int) li;

	return CSVP_OK;
}

int csvline_short_at(struct csvline *c, int at, short *out)
{
	long int li;
	char *endptr;
	const char *str;

	if (at < 0 || at >= c->num_fields) {
		c->error = CSVLINE_EINDEX;
		return CSVP_ERROR;
	}

	str = c->fields[at];
	li = strtol(str, &endptr, 10);

	if (*endptr != '\0') {
		/*
		 * endptr points at the first bad character, so if it
		 * points at anything other than the null byte, there
		 * was an error trying to parse the string
		 */
		c->error = CSVLINE_EWRONGTYPE;
		return CSVP_ERROR;
	}

	if (li < SHRT_MIN || li > SHRT_MAX) {
		c->error = CSVLINE_ERANGE;
		return CSVP_ERROR;
	}

	*out = (short) li;

	return CSVP_OK;
}

const char *csvline_strerror(const struct csvline *c)
{
	return error_strings[(int)c->error];
}
