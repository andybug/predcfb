
#include <stdio.h>

#include "parse.h"

const struct parse_handler parse_handlers[] = {
	{ "conference.csv", PARSE_FILE_CSV, parse_conference_csv },
	{ NULL, PARSE_FILE_NONE, NULL }
};

const int num_parse_handlers = sizeof(parse_handlers) / sizeof(struct parse_handler);

/* parsing functions */

int parse_conference_csv(struct fieldlist *f)
{
	const char *str;
	int count = 0;

	str = fieldlist_iter_begin(f);

	while (str) {
		if (count == 0)
			fputc('[', stdout);

		fprintf(stdout, "'%s'", str);

		if (count + 1 == f->num_fields)
			fputs("]\n", stdout);
		else
			fputs(", ", stdout);

		count++;
	}

	return PARSE_OK;
}

#if 0
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
#endif
