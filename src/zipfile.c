
#include <string.h>
#include <assert.h>
#include <errno.h>
#include <stdbool.h>
#include <stdlib.h>

#include <unistd.h>

#include <minizip/unzip.h>
#include <predcfb/zipfile.h>

/*
 * This structure will keep track of the library handles and all of
 * the other variables used during the reading of a zipfile
 */
struct zipfile_read_context {
	unzFile unzip_handle;
	bool archive_open;
	bool file_open;
	enum zipfile_err error;
};

/* error functions */

const char *zipfile_strerr(zf_readctx *z)
{
	(void) z;
	return "FIXME: generic zipfile error";
}

enum zipfile_err zipfile_get_error(zf_readctx *z)
{
	return z->error;
}

static int check_open_states(zf_readctx *z, bool archive, bool file)
{
	int retval = ZIPFILE_OK;

	/* check open state of the archive */
	if (archive) {
		if (z->archive_open == false) {
			retval = ZIPFILE_ERROR;
			z->error = ZIPFILE_ENOTOPEN;
		}
	} else {
		if (z->archive_open == true) {
			retval = ZIPFILE_ERROR;
			z->error = ZIPFILE_ESTILLOPEN;
		}
	}

	/* check open state of the file */
	if (file) {
		if (z->file_open == false) {
			retval = ZIPFILE_ERROR;
			z->error = ZIPFILE_ENOTOPEN;
		}
	} else {
		if (z->file_open == true) {
			retval = ZIPFILE_ERROR;
			z->error = ZIPFILE_ESTILLOPEN;
		}
	}

	return retval;
}

/* archive functions */

static int zipfile_check_access(zf_readctx *z, const char *path)
{
	if (access(path, R_OK) != 0) {
		switch (errno) {
		case EACCES:
			z->error = ZIPFILE_EACCESS;
			break;

		case ELOOP:
		case ENAMETOOLONG:
		case ENOENT:
		case ENOTDIR:
			z->error = ZIPFILE_EPATH;
			break;
		}

		return ZIPFILE_ERROR;
	}

	return ZIPFILE_OK;
}

zf_readctx *zipfile_open_archive(const char *path)
{
	zf_readctx *z;

	z = malloc(sizeof(*z));
	if (!z)
		return NULL;

	z->unzip_handle = unzOpen(path);
	if (!z->unzip_handle) {

		/* if there is no access error, then the unzip library
		 * is refusing to read the file. this probably means that
		 * the file is malformed: set z->error to indicate a bad
		 * file. if there was an access error, then z->error
		 * was already set
		 */

		if (zipfile_check_access(z, path) != ZIPFILE_OK)
			z->error = ZIPFILE_EFILEBAD;

		return z;
	}

	z->archive_open = true;
	z->file_open = false;
	z->error = ZIPFILE_ENONE;

	return z;
}

int zipfile_close_archive(zf_readctx *z)
{
	/* archive open and file closed */
	if (check_open_states(z, true, false) != ZIPFILE_OK)
		return ZIPFILE_ERROR;

	if (unzClose(z->unzip_handle) != UNZ_OK) {
		z->error = ZIPFILE_EINTERNAL;
		return ZIPFILE_ERROR;
	}

	z->archive_open = false;
	free(z);

	return ZIPFILE_OK;
}

/* functions for extracting files inside the archive */

int zipfile_open_file(zf_readctx *z, const char *file)
{
	/* archive open and file closed */
	if (check_open_states(z, true, false) != ZIPFILE_OK)
		return ZIPFILE_ERROR;

	if (unzLocateFile(z->unzip_handle, file, 1) != UNZ_OK) {
		z->error = ZIPFILE_ENOENT;
		return ZIPFILE_ERROR;
	}

	if (unzOpenCurrentFile(z->unzip_handle) != UNZ_OK) {
		z->error = ZIPFILE_EINTERNAL;
		return ZIPFILE_ERROR;
	}

	z->file_open = true;

	return ZIPFILE_OK;
}

int zipfile_close_file(zf_readctx *z)
{
	/* archive open and file open */
	if (check_open_states(z, true, true) != ZIPFILE_OK)
		return ZIPFILE_ERROR;

	if (unzCloseCurrentFile(z->unzip_handle) != UNZ_OK)
		return ZIPFILE_ERROR;

	z->file_open = false;

	return ZIPFILE_OK;
}

/* functions for reading data from the zipfile */

ssize_t zipfile_read_file(zf_readctx *z, char *buf, size_t count)
{
	int err;
	ssize_t bytes_read = 0;

	/* archive open and file open */
	if (check_open_states(z, true, true) != ZIPFILE_OK)
		return ZIPFILE_ERROR;

	err = unzReadCurrentFile(z->unzip_handle, buf, count);

	if (err > 0) {
		/* successful read */
		bytes_read = (ssize_t) err;
	} else if (err == 0) {
		/* nothing to read, eof */
		bytes_read = 0;
	} else if (err < 0) {
		/* error */
		z->error = ZIPFILE_EINTERNAL;
		return ZIPFILE_ERROR;
	}

	return bytes_read;
}
