
#include <stdlib.h>

#include <libcsv/csv.h>
#include <predcfb/csvparse.h>
#include <predcfb/fieldlist.h>

struct csvparse_context {
	struct csv_parser parser;
	enum csvp_error error;
	struct fieldlist fieldlist;
	int lines;
	int (*handler)(struct fieldlist*);
};

static const char *csvparse_errors[] = {
	"No error",
	"Memory allocation failed",
	"Too many fields",
	"Not enough buffer space for field strings",
	"Parser error"
};

static void add_to_fieldlist(void *str, size_t len, void *mydata)
{
	struct csvparse_context *c = mydata;

	if (fieldlist_add(&c->fieldlist, str, len) != FIELDLIST_OK) {
		switch (c->fieldlist.error) {

		case FIELDLIST_EMAXFIELDS:
			c->error = CSVP_ETOOMANY;
			break;

		case FIELDLIST_ESTRBUFSPACE:
			c->error = CSVP_ENOBUFS;
			break;

		default:
		case FIELDLIST_ENONE:
			/* what? */
			break;
		}
	}
}

static void send_fieldlist_to_parse(int ch, void *mydata)
{
	struct csvparse_context *c = mydata;
	(void) ch;

	if (c->error == CSVP_ENONE) {
		c->lines++;
		c->fieldlist.line = c->lines;

		if (c->handler(&c->fieldlist) != 0)
			c->error = CSVP_EPARSE;
	}

	fieldlist_clear(&c->fieldlist);
}

csvp_ctx *csvp_create(int (*handler)(struct fieldlist*))
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

const char *csvp_strerror(const csvp_ctx *c)
{
	return csvparse_errors[c->error];
}

enum csvp_error csvp_error(const csvp_ctx *c)
{
	return c->error;
}
