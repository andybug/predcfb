
#include <stdio.h>
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
		fprintf(stderr, "Unable to open archive\n");
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
		return ARCHIVE_MISSING_FILE_ERROR;

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

#if 0
static int extract_conference_file(unzFile handle)
{
	static const char CONFERENCE_FILE[] = "conference.csv";
	static const int BUF_SIZE = 4096;
	char buf[BUF_SIZE];
	int err;
	size_t bytes;
	bool eof = false;

	err = open_file(handle, CONFERENCE_FILE);
	if (err)
		return err;

	while (!eof) {
		err = read_file(handle, buf, BUF_SIZE - 1, &bytes, &eof);
		if (err != ARCHIVE_OK)
			return err;

		puts("---");
		buf[BUF_SIZE - 1] = '\0';
		puts(buf);
	}

	err = close_file(handle);
	if (err)
		return err;

	return ARCHIVE_OK;
}
#endif
