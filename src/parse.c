
#include <stdlib.h>
#include <string.h>

#include <csv.h>

#include "parse.h"

#define STRBUF_SIZE 1024
#define FIELD_LIST_SIZE 32

struct strbuf {
	char buf[STRBUF_SIZE];
	size_t used;
};

struct field_list {
	char *fields[FIELD_LIST_SIZE];
	int num_fields;
	struct strbuf strbuf;
};

struct parse_context {
	struct csv_parser parser;
	void (*cb_field)(void*, size_t, void*);
	void (*cb_record)(int, void*);
	struct field_list field_list;
};

struct file_handler {
	const char *file;
	void (*cb_record)(int, void*);
};

/* prototypes for parsing functions */
static void all_cb_field(void*, size_t, void*);
static void conf_cb_record(int, void*);

static struct file_handler file_handlers[] = {
	{ "conference.csv", conf_cb_record },
	{ NULL, NULL }
};

static void all_cb_field(void *data, size_t len, void* v)
{
	char *str = data;

	printf("'%.*s'\n", len, str);
}

static void conf_cb_record(int i, void* v)
{
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

int parse_init(parse_ctx *p, const char *file)
{
	struct parse_context *pc;
	int err;

	pc = malloc(sizeof *pc);
	if (!pc)
		return PARSE_MEMORY_ERROR;

	/* FIXME */
	if (strcmp(file, "conference.csv") == 0) {
		pc->cb_record = conf_cb_record;
	}

	err = csv_init(&pc->parser, CSV_STRICT);
	if (err) {
		free(pc);
		return PARSE_INTERNAL_ERROR;
	}

	*p = pc;

	return PARSE_OK;
}
