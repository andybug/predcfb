
#include <stdlib.h>
#include <errno.h>
#include <assert.h>
#include <limits.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>

#include <predcfb/parse.h>
#include <predcfb/conference.h>
#include <predcfb/objectdb.h>
#include <predcfb/zipfile.h>
#include <predcfb/csvparse.h>

enum parse_file_type {
	PARSE_FILE_NONE,
	PARSE_FILE_CSV
};

struct parse_handler {
	const char *file;
	enum parse_file_type type;
	int (*parsing_func)(struct fieldlist *);
};

const struct parse_handler cfbstats_handlers[] = {
	{ "conference.csv", PARSE_FILE_CSV, parse_conference_csv },
	{ NULL, PARSE_FILE_NONE, NULL }
};

/* subtract 1 from total to account for ending null struct */
const int num_cfbstats_handlers = (sizeof(cfbstats_handlers) /
                                   sizeof(struct parse_handler)) - 1;

enum parse_err parse_errno = PARSE_ENONE;

/* conversion functions */

#if 0
static int parse_short(const char *str, short *out)
{
	long int li;
	char *endptr;

	li = strtol(str, &endptr, 10);

	if (*endptr != '\0') {
		/*
		 * a non-number character was found in the string,
		 * so this is invalid
		 */
		return PARSE_ERROR;
	}

	if (li == LONG_MIN || li == LONG_MAX) {
		if (errno == ERANGE)
			return PARSE_ERROR;
	}

	if (li < SHRT_MIN || li > SHRT_MAX)
		return PARSE_ERROR;

	*out = (short) li;

	return PARSE_OK;
}
#endif

static int parse_int(const char *str, int *out)
{
	long int li;
	char *endptr;

	li = strtol(str, &endptr, 10);

	if (*endptr != '\0') {
		/*
		 * a non-number character was found in the string,
		 * so this is invalid
		 */
		return PARSE_ERROR;
	}

	if (li == LONG_MIN || li == LONG_MAX) {
		if (errno == ERANGE)
			return PARSE_ERROR;
	}

	if (li < INT_MIN || li > INT_MAX)
		return PARSE_ERROR;

	*out = (int) li;

	return PARSE_OK;
}

/* parsing functions */

static int check_conference_csv_header(struct fieldlist *f)
{
	static const char *field_names[] = {
		"Conference Code",
		"Name",
		"Subdivision"
	};

	int count = 0;
	const char *field;

	assert(f->num_fields == 3);

	field = fieldlist_iter_begin(f);

	while (field) {
		if (count >= 3)
			return PARSE_ERROR;

		if (strcmp(field_names[count], field) != 0)
			return PARSE_ERROR;

		field = fieldlist_iter_next(f);
		count++;
	}

	return PARSE_OK;
}

int parse_conference_csv(struct fieldlist *f)
{
	static bool processed_header = false;
	struct conference *conf;
	const char *str;
	size_t len;
	int id;
	objectid oid;

	assert(f->num_fields == 3);

	if (!processed_header) {
		processed_header = true;
		return check_conference_csv_header(f);
	}

	conf = conference_create();
	if (!conf) {
		/* too many conferences! */
		return PARSE_ERROR;
	}

	/* id field */
	str = fieldlist_iter_begin(f);
	if (parse_int(str, &id) != PARSE_OK)
		return PARSE_ERROR;

	/* conference name */
	str = fieldlist_iter_next(f);
	len = strlen(str);
	if (len >= CONFERENCE_NAME_MAX)
		return PARSE_ERROR;
	strncpy(conf->name, str, CONFERENCE_NAME_MAX);

	/* division */
	str = fieldlist_iter_next(f);
	if (strcmp("FBS", str) == 0)
		conf->div = CONFERENCE_FBS;
	else if (strcmp("FCS", str) == 0)
		conf->div = CONFERENCE_FCS;
	else
		return PARSE_ERROR;

	/* add the conference to the objectdb */
	if (objectdb_add_conference(conf, &oid) != OBJECTDB_OK)
		return PARSE_ERROR;

	char oid_str[OBJECTDB_MD_STR_SIZE];
	objectid_string(&oid, oid_str);

	printf("%s  %s (%d)\n", oid_str, conf->name, (int)conf->div);

	return PARSE_OK;
}

/* zipfile parsing */

static void handle_zipfile_error(zf_readctx *zf, const char *func)
{
	const char *err;

	err = zipfile_strerr(zf);
	fprintf(stderr, "%s: %s\n", func, err);

	parse_errno = PARSE_EZIPFILE;
}

static int read_csv_file_from_zipfile(zf_readctx *zf,
                                      const struct parse_handler *handler)
{
	static const int CSV_BUF_SIZE = 4096;
	char buf[CSV_BUF_SIZE];
	ssize_t bytes;
	csvp_ctx *csvp;

	if (zipfile_open_file(zf, handler->file) != ZIPFILE_OK) {
		handle_zipfile_error(zf, __func__);
		return PARSE_ERROR;
	}

	csvp = csvp_create(handler->parsing_func);
	if (!csvp) {
		/* TODO: handle csvp error correctly */
		return PARSE_ERROR;
	}

	while ((bytes = zipfile_read_file(zf, buf, CSV_BUF_SIZE))) {
		if (bytes == ZIPFILE_ERROR)
			return PARSE_ERROR;

		if (bytes == 0) /* eof */
			break;

		if (csvp_parse(csvp, buf, bytes) != CSVP_OK) {
			/* TODO: handle csvp error correctly */
			return PARSE_ERROR;
		}
	}

	if (csvp_destroy(csvp) != CSVP_OK) {
		/* TODO: handle csvp error correctly */
		return PARSE_ERROR;
	}

	if (zipfile_close_file(zf) != ZIPFILE_OK) {
		handle_zipfile_error(zf, __func__);
		return PARSE_ERROR;
	}

	return PARSE_OK;
}

static int read_files_from_zipfile(zf_readctx *zf)
{
	const struct parse_handler *handler = cfbstats_handlers;

	while (handler->type != PARSE_FILE_NONE) {
		switch (handler->type) {
		case PARSE_FILE_CSV:
			if (read_csv_file_from_zipfile(zf, handler) != PARSE_OK)
				return PARSE_ERROR;
			break;

		case PARSE_FILE_NONE:
		default:
			/* this can't happen... */
			break;
		}

		handler++;
	}

	return PARSE_OK;
}

int parse_zipfile(const char *path)
{
	zf_readctx *zf;

	zf = zipfile_open_archive(path);
	if (!zf || (zipfile_get_error(zf) != ZIPFILE_ENONE)) {
		if (zf)
			handle_zipfile_error(zf, __func__);
		else
			parse_errno = PARSE_ENOMEM;

		return PARSE_ERROR;
	}

	if (read_files_from_zipfile(zf) != ZIPFILE_OK)
		return PARSE_ERROR;

	if (zipfile_close_archive(zf) != ZIPFILE_OK) {
		handle_zipfile_error(zf, __func__);
		return PARSE_ERROR;
	}

	return PARSE_OK;
}
