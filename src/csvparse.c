
#include <stdlib.h>

#include <csv.h>

#include "csvparse.h"

struct csvparse_context {
	struct csv_parser parser;
	enum csvp_error error;
};

csvp_ctx *csvp_create(void)
{
	csvp_ctx *c;

	c = calloc(1, sizeof(*c));
	if (!c)
		return NULL;

	if (csv_init(&c->parser, CSV_STRICT) != CSV_SUCCESS) {
		free(c);
		return NULL;
	}

	return c;
}

int csvp_destroy(csvp_ctx *c)
{
	return 0;
}

int csvp_parse(csvp_ctx *c, char *buf, size_t len)
{
	return 0;
}
