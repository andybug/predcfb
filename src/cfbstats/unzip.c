
#include <stdio.h>

#include <predcfb/zipfile.h>
#include <predcfb/csvparse.h>

#include "cfbstats_internal.h"


static void handle_zipfile_error(zf_readctx *zf, const char *func)
{
	const char *err;

	err = zipfile_strerr(zf);
	fprintf(stderr, "%s: %s\n", func, err);

	cfbstats_errno = CFBSTATS_EZIPFILE;
}

static int read_csv_file_from_zipfile(zf_readctx *zf,
                                      const struct file_handler *handler)
{
	static const int CSV_BUF_SIZE = 4096;
	char buf[CSV_BUF_SIZE];
	ssize_t bytes;
	csvp_ctx *csvp;

	if (zipfile_open_file(zf, handler->file) != ZIPFILE_OK) {
		handle_zipfile_error(zf, __func__);
		return CFBSTATS_ERROR;
	}

	csvp = csvp_create(handler->parsing_func);
	if (!csvp) {
		fprintf(stderr, "%s: csvp initialization error\n", progname);
		return CFBSTATS_ERROR;
	}

	while ((bytes = zipfile_read_file(zf, buf, CSV_BUF_SIZE))) {
		if (bytes == ZIPFILE_ERROR)
			return CFBSTATS_ERROR;

		if (bytes == 0) /* eof */
			break;

		if (csvp_parse(csvp, buf, bytes) != CSVP_OK) {
			const char *err;

			if (csvp_error(csvp) == CSVP_EPARSE) {
				/*
				 * if it is a parsing error, then check our
				 * own error code since the error originated
				 * here
				 */
				err = cfbstats_strerror();
				fprintf(stderr, "%s: %s\n", progname, err);
			}

			err = csvp_strerror(csvp);
			fprintf(stderr, "%s: %s in %s\n",
			        progname, err, handler->file);

			return CFBSTATS_ERROR;
		}
	}

	if (csvp_destroy(csvp) != CSVP_OK) {
		const char *err = csvp_strerror(csvp);
		fprintf(stderr, "%s: %s in %s\n",
		        progname, err, handler->file);
		return CFBSTATS_ERROR;
	}

	if (zipfile_close_file(zf) != ZIPFILE_OK) {
		handle_zipfile_error(zf, __func__);
		return CFBSTATS_ERROR;
	}

	return CFBSTATS_OK;
}

static int read_files_from_zipfile(zf_readctx *zf)
{
	const struct file_handler *handler = file_handlers;

	while (handler->type != CFBSTATS_FILE_NONE) {
		switch (handler->type) {
		case CFBSTATS_FILE_CSV:
			if (read_csv_file_from_zipfile(zf, handler) != CFBSTATS_OK)
				return CFBSTATS_ERROR;
			break;

		case CFBSTATS_FILE_NONE:
		default:
			/* this can't happen... */
			break;
		}

		handler++;
	}

	return CFBSTATS_OK;
}

/* global functions */

int cfbstats_read_zipfile(const char *path)
{
	zf_readctx *zf;

	cfbstats_init();

	zf = zipfile_open_archive(path);
	if (!zf) {
		cfbstats_errno = CFBSTATS_ENOMEM;
		return CFBSTATS_ERROR;
	} else if (zipfile_get_error(zf) != ZIPFILE_ENONE) {
		handle_zipfile_error(zf, __func__);
		return CFBSTATS_ERROR;
	}

	if (read_files_from_zipfile(zf) != ZIPFILE_OK)
		return CFBSTATS_ERROR;

	if (zipfile_close_archive(zf) != ZIPFILE_OK) {
		handle_zipfile_error(zf, __func__);
		return CFBSTATS_ERROR;
	}

	/*
	 * finish up by setting all of the pointers in the data
	 * from the object ids
	 */
	if (objectdb_link() != OBJECTDB_OK)
		return CFBSTATS_ERROR;

	return CFBSTATS_OK;
}
