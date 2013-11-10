
#include <stdbool.h>
#include <stdlib.h>

/* minizip header */
#include <unzip.h>

#include "archive.h"

struct archive_context {
	unzFile handle;
};

int archive_open(archive_ctx *a, const char *path)
{
	struct archive_context *ac;

	ac = malloc(sizeof(*ac));
	if (!ac)
		return ARCHIVE_MEMORY_ERROR;

	ac->handle = unzOpen(path);
	if (!ac->handle) {
		free(ac);
		return ARCHIVE_OPEN_ERROR;
	}

	/* set given context to allocated one */
	*a = ac;

	return ARCHIVE_OK;
}

int archive_close(archive_ctx a)
{
	struct archive_context *ac = a;

	unzClose(ac->handle);
	return ARCHIVE_OK;
}

int archive_open_file(archive_ctx a, const char *file)
{
	struct archive_context *ac = a;

	if (unzLocateFile(ac->handle, file, 1) != UNZ_OK)
		return ARCHIVE_FILE_NOT_FOUND_ERROR;

	if (unzOpenCurrentFile(ac->handle) != UNZ_OK)
		return ARCHIVE_INTERNAL_ERROR;

	return ARCHIVE_OK;
}

int archive_close_file(archive_ctx a)
{
	struct archive_context *ac = a;

	if (unzCloseCurrentFile(ac->handle) != UNZ_OK)
		return ARCHIVE_INTERNAL_ERROR;

	return ARCHIVE_OK;
}

int archive_read_file(archive_ctx a, char *buf, size_t len, size_t *read)
{
	struct archive_context *ac = a;
	int err;
	bool eof = false;

	err = unzReadCurrentFile(ac->handle, buf, len);

	if (err > 0) {
		/* successful read */
		*read = (size_t) err;
		if (unzeof(ac->handle))
			eof = true;
	} else if (err == 0) {
		/* nothing to read, eof */
		*read = 0;
		eof = true;
	} else if (err < 0) {
		/* error */
		return ARCHIVE_INTERNAL_ERROR;
	}
	
	return eof ? ARCHIVE_EOF : ARCHIVE_OK;
}
