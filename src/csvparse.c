
#include <stdlib.h>

#include <csv.h>

#include "csvparse.h"
#include "fieldlist.h"

struct csvparse_context {
	struct csv_parser parser;
	enum csvp_error error;
	struct fieldlist fieldlist;
	struct parse_handler *handler;
};

static void add_to_fieldlist(void *str, size_t len, void *mydata)
{
	struct csvparse_context *c = mydata;

	if (fieldlist_add(&c->fieldlist, str, len) != FIELDLIST_OK)
		c->error = CSVP_ETOOMANY;
}

static void send_fieldlist_to_parse(int ch, void *mydata)
{
	struct csvparse_context *c = mydata;
	(void) ch;

	if (c->fieldlist.error != FIELDLIST_ENONE)
		return;

	if (c->handler->parsing_func(&c->fieldlist) != PARSE_OK)
		c->error = CSVP_EPARSE;
}

csvp_ctx *csvp_create(struct parse_handler *handler)
{
	csvp_ctx *c;

	c = calloc(1, sizeof(*c));
	if (!c)
		return NULL;

	if (csv_init(&c->parser, CSV_STRICT) != CSV_SUCCESS) {
		free(c);
		return NULL;
	}

	c->handler = handler;

	return c;
}

int csvp_destroy(csvp_ctx *c)
{
	int err;

	err = csv_fini(&c->parser,
	               add_to_fieldlist,
	               send_fieldlist_to_parse,
	               c);

	if (err != CSV_SUCCESS) {
		c->error = CSVP_EPARSE;
		return CSVP_ERROR;
	}

	/*
	 * this check is necessary since these errors are detected and
	 * set in the callbacks add_to_fieldlist and send_fieldlist_to_parse,
	 * which cannot return an error value
	 */
	if (c->error == CSVP_ETOOMANY || c->error == CSVP_EPARSE)
		return CSVP_ERROR;

	csv_free(&c->parser);
	free(c);

	return CSVP_OK;
}

int csvp_parse(csvp_ctx *c, char *buf, size_t len)
{
	size_t bytes;

	bytes = csv_parse(&c->parser,
	                  buf,
	                  len,
	                  add_to_fieldlist,
	                  send_fieldlist_to_parse,
	                  c);

	if (bytes != len) {
		c->error = CSVP_EPARSE;
		return CSVP_ERROR;
	}

	/*
	 * this check is necessary since these errors are detected and
	 * set in the callbacks add_to_fieldlist and send_fieldlist_to_parse,
	 * which cannot return an error value
	 */
	if (c->error == CSVP_ETOOMANY || c->error == CSVP_EPARSE)
		return CSVP_ERROR;

	return CSVP_OK;
}
