
#include <stdlib.h>
#include <string.h>

#include <csv.h>

#include "parse.h"

#define STRBUF_SIZE 1024
#define FIELD_LIST_SIZE 32

#define FIELD_LIST_OK		0
#define FIELD_LIST_FULL		(-1)
#define FIELD_LIST_STRBUF_FULL	(-2)

struct strbuf {
	char buf[STRBUF_SIZE];
	size_t used;
};

struct field_list {
	const char *fields[FIELD_LIST_SIZE];
	int num_fields;
	struct strbuf strbuf;
};

struct parse_context {
	struct csv_parser parser;
	void (*cb_field)(void*, size_t, void*);
	void (*cb_record)(int, void*);
	struct field_list field_list;
};

/* prototypes for parsing functions */

static void all_cb_field(void*, size_t, void*);
static void conf_cb_record(int, void*);

const struct parse_file_handler parse_file_handlers[] = {
	{ "conference.csv", conf_cb_record },
	{ NULL, NULL }
};

/* strbuf functions */

static const char *strbuf_add(struct strbuf *s, const char *str, size_t len)
{
	size_t total_len;
	size_t new_used;
	const char *alloc_str;

	total_len = len + 1;
	new_used = s->used + total_len;
	if (new_used >= STRBUF_SIZE)
		return NULL;

	alloc_str = memcpy(s->buf + s->used, str, len);
	s->buf[s->used + len] = '\0';
	s->used = new_used;

	return alloc_str;
}

static void strbuf_clear(struct strbuf *s)
{
	s->used = 0;
}

/* field_list functions */

static int field_list_add(struct field_list *f, const char *str, size_t len)
{
	const char *strbuf_str;

	if (f->num_fields >= FIELD_LIST_SIZE)
		return FIELD_LIST_FULL;

	strbuf_str = strbuf_add(&f->strbuf, str, len);
	if (!strbuf_str)
		return FIELD_LIST_STRBUF_FULL;

	f->fields[f->num_fields] = strbuf_str;
	f->num_fields++;

	return FIELD_LIST_OK;
}

static void field_list_clear(struct field_list *f)
{
	strbuf_clear(&f->strbuf);
	f->num_fields = 0;
}

static void all_cb_field(void *data, size_t len, void* v)
{
	struct field_list *fl = v;
	int err;

	err = field_list_add(fl, data, len);
	if (err)
		fputs("error: field_list full\n", stderr);
}

static void conf_cb_record(int i_, void* v)
{
	struct field_list *fl = v;
	int i;

	fputc('[', stdout);

	for (i = 0; i < fl->num_fields; i++) {
		if (i + 1 == fl->num_fields)
			fputs(fl->fields[i], stdout);
		else
			fprintf(stdout, "%s, ", fl->fields[i]);
	}

	fputs("]\n", stdout);

	field_list_clear(fl);
}

int parse_data(parse_ctx p, const char *buf, size_t len)
{
	struct parse_context *pc = p;
	size_t bytes;

	bytes = csv_parse(&pc->parser, buf, len,
	                  all_cb_field, pc->cb_record, &pc->field_list);

	if (bytes != len)
		return PARSE_INTERNAL_ERROR;

	return PARSE_OK;
}

int parse_finish(parse_ctx p)
{
	struct parse_context *pc = p;
	int err;

	err = csv_fini(&pc->parser,
	               all_cb_field, conf_cb_record, &pc->field_list);

	if (err)
		return PARSE_INTERNAL_ERROR;

	csv_free(&pc->parser);
	free(pc);

	return PARSE_OK;
}

int parse_init(parse_ctx *p, const struct parse_file_handler *file_handler)
{
	struct parse_context *pc;
	int err;

	pc = malloc(sizeof *pc);
	if (!pc)
		return PARSE_MEMORY_ERROR;

	err = csv_init(&pc->parser, CSV_STRICT);
	if (err) {
		free(pc);
		return PARSE_INTERNAL_ERROR;
	}

	pc->cb_record = file_handler->handler;
	field_list_clear(&pc->field_list);

	*p = pc;

	return PARSE_OK;
}
