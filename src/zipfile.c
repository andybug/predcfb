
#include <string.h>
#include <assert.h>
#include <errno.h>
#include <stdbool.h>

#include <unistd.h>

#include <unzip.h>
#include <csv.h>

#include "zipfile.h"

/*
 * This structure will keep track of the library handles and all of
 * the other variables used during the reading of a zipfile
 */
typedef struct zipfile_read_context {
	unzFile unzip_handle;
	struct csv_parser csv_handle;
	const char *open_archive;
	const char *open_file;
	bool archive_open;
	bool file_open;
	enum zipfile_err error;
} zf_readctx;

/* archive functions */

static int zipfile_check_access(zf_readctx *z, const char *path)
{
	int err;

	err = access(path, R_OK);

	if (err) {
		if (errno == EACCES)
			z->error = ZIPFILE_EACCESS;
		else
			z->error = ZIPFILE_EPATH;

		return ZIPFILE_ERROR;
	}

	return ZIPFILE_OK;
}

static int zipfile_open_archive(zf_readctx *z, const char *path)
{
	int err;

	z->unzip_handle = unzOpen(path);

	if (!z->unzip_handle) {
		err = zipfile_check_access(z, path);

		/* if there is no access error, then the unzip library
		 * is refusing to read the file. this probably means that
		 * the file is malformed: set z->error to indicate a bad
		 * file. if there was an access error, then z->error
		 * was already set
		 */

		if (!err)
			z->error = ZIPFILE_EFILEBAD;

		return ZIPFILE_ERROR;
	}

	z->open_archive = path;
	z->archive_open = true;

	return ZIPFILE_OK;
}

static int zipfile_close_archive(zf_readctx *z)
{
	assert(z->archive_open == true);
	assert(z->file_open == false);

	if (unzClose(z->unzip_handle) != UNZ_OK) {
		z->error = ZIPFILE_EINTERNAL;
		return ZIPFILE_ERROR;
	}

	return ZIPFILE_OK;
}

/* functions for extracting files inside the archive */

static int zipfile_open_file(zf_readctx *z, const char *file)
{
	assert(z->archive_open == true);
	assert(z->file_open == false);

	if (unzLocateFile(z->unzip_handle, file, 1) != UNZ_OK) {
		z->error = ZIPFILE_ENOENT;
		return ZIPFILE_ERROR;
	}

	if (unzOpenCurrentFile(z->unzip_handle) != UNZ_OK) {
		z->error = ZIPFILE_EINTERNAL;
		return ZIPFILE_ERROR;
	}

	return ZIPFILE_OK;
}

static int zipfile_close_file(zf_readctx *z)
{
	assert(z->archive_open == true);
	assert(z->file_open == true);

	if (unzCloseCurrentFile(z->unzip_handle) != UNZ_OK)
		return ZIPFILE_ERROR;

	return ZIPFILE_OK;
}

int zipfile_read(const char *path)
{
	zf_readctx z, *zp = &z;

	memset(zp, 0, sizeof(zf_readctx));

	if (zipfile_open_archive(zp, path) != ZIPFILE_OK)
		return ZIPFILE_ERROR;

	if (zipfile_close_archive(zp) != ZIPFILE_OK)
		return ZIPFILE_ERROR;

	return ZIPFILE_OK;
}
