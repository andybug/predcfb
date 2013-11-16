
#include <stdlib.h>
#include <string.h>

#include <csv.h>

#include "parse.h"
#include "fieldlist.h"

struct parse_context {
	struct csv_parser parser;
	void (*cb_field)(void*, size_t, void*);
	void (*cb_record)(int, void*);
	struct fieldlist field_list;
};

/* prototypes for parsing functions */

static void all_cb_field(void*, size_t, void*);
static void conf_cb_record(int, void*);

const struct parse_file_handler parse_file_handlers[] = {
	{ "conference.csv", conf_cb_record },
	{ NULL, NULL }
};

static void all_cb_field(void *data, size_t len, void* v)
{
	struct fieldlist *fl = v;
	int err;

	err = fieldlist_add(fl, data, len);
	if (err)
		fputs("error: field_list full\n", stderr);
}

static void conf_cb_record(int i_, void* v)
{
	struct fieldlist *fl = v;
	int i;

	fputc('[', stdout);

	for (i = 0; i < fl->num_fields; i++) {
		if (i + 1 == fl->num_fields)
			fputs(fl->fields[i], stdout);
		else
			fprintf(stdout, "%s, ", fl->fields[i]);
	}

	fputs("]\n", stdout);

	fieldlist_clear(fl);
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
	fieldlist_clear(&pc->field_list);

	*p = pc;

	return PARSE_OK;
}
