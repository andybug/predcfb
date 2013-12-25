
#include <stdlib.h>
#include <string.h>

#include <libcsv/csv.h>
#include <predcfb/csvparse.h>

static void add_to_csvline(void *str, size_t len, void *mydata)
{
	struct csvparse *c = mydata;

	if (csvline_add(&c->csvline, str, len) != CSVP_OK) {
		switch (c->csvline.error) {

		case CSVLINE_EMAXFIELDS:
			c->error = CSVP_ETOOMANY;
			break;

		case CSVLINE_ESTRBUFSPACE:
			c->error = CSVP_ENOBUFS;
			break;

		default:
		case CSVLINE_ENONE:
			/* what? */
			break;
		}
	}
}

static void send_csvline_to_parse(int ch, void *mydata)
{
	struct csvparse *c = mydata;
	(void) ch;

	if (c->error == CSVP_ENONE) {
		c->lines++;
		c->csvline.line = c->lines;

		if (c->handler(&c->csvline) != 0)
			c->error = CSVP_EPARSE;
	}

	csvline_clear(&c->csvline);
}

int csvp_init(struct csvparse *c, int (*handler)(struct csvline*))
{
	memset(c, 0, sizeof(*c));

	if (csv_init(&c->parser, CSV_STRICT) != CSV_SUCCESS) {
		c->error = CSVP_EINTERNAL;
		return CSVP_ERROR;
	}

	c->handler = handler;

	return CSVP_OK;
}

int csvp_destroy(struct csvparse *c)
{
	int err;

	err = csv_fini(&c->parser,
			add_to_csvline,
			send_csvline_to_parse,
			c);

	if (err != CSV_SUCCESS) {
		c->error = CSVP_EPARSE;
		return CSVP_ERROR;
	}

	/*
	 * this check is necessary since these errors are detected and
	 * set in the callbacks add_to_csvline and send_csvline_to_parse,
	 * which cannot return an error value
	 */
	if (c->error == CSVP_ETOOMANY || c->error == CSVP_EPARSE)
		return CSVP_ERROR;

	csv_free(&c->parser);

	return CSVP_OK;
}

int csvp_parse(struct csvparse *c, char *buf, size_t len)
{
	size_t bytes;

	bytes = csv_parse(&c->parser,
			buf,
			len,
			add_to_csvline,
			send_csvline_to_parse,
			c);

	if (bytes != len) {
		c->error = CSVP_EPARSE;
		return CSVP_ERROR;
	}

	/*
	 * this check is necessary since these errors are detected and
	 * set in the callbacks add_to_csvline and send_csvline_to_parse,
	 * which cannot return an error value
	 */
	if (c->error == CSVP_ETOOMANY || c->error == CSVP_EPARSE)
		return CSVP_ERROR;

	return CSVP_OK;
}

const char *csvp_strerror(const struct csvparse *c)
{
	const char *err;

	switch (c->error) {
	case CSVP_ENONE:
		err = "No error";
		break;

	case CSVP_ETOOMANY:
		err = "Too many fields";
		break;

	case CSVP_ENOBUFS:
		err = "Not enough buffer space";
		break;

	case CSVP_EPARSE:
		err = "Parser error";
		break;

	case CSVP_EINTERNAL:
		err = "Internal libcsv error";
		break;

	default:
		err = "No such error";
		break;
	}

	return err;
}

enum csvparse_error csvp_error(const struct csvparse *c)
{
	return c->error;
}
