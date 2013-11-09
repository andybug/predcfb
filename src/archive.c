
#include <stdio.h>
#include <stdbool.h>

/* minizip header */
#include <unzip.h>

#include "archive.h"

static unzFile open_archive(const char *path)
{
	unzFile handle;

	handle = unzOpen(path);
	if (!handle)
		fprintf(stderr, "Unable to open archive\n");

	return handle;
}

static void close_archive(unzFile handle)
{
	unzClose(handle);
}

static int open_file(unzFile handle, const char *file)
{
	if (unzLocateFile(handle, file, 1) != UNZ_OK)
		return ARCHIVE_MISSING_FILE_ERROR;

	if (unzOpenCurrentFile(handle) != UNZ_OK)
		return ARCHIVE_INTERNAL_ERROR;

	return ARCHIVE_OK;
}

static int close_file(unzFile handle)
{
	if (unzCloseCurrentFile(handle) != UNZ_OK)
		return ARCHIVE_INTERNAL_ERROR;

	return ARCHIVE_OK;
}

static int read_file(unzFile handle, char *buf, size_t len, size_t *read, bool *eof)
{
	int err;

	/* reset eof flag */
	*eof = false;

	err = unzReadCurrentFile(handle, buf, len);

	if (err > 0) {
		/* successful read */
		*read = (size_t) err;
		if (unzeof(handle))
			*eof = true;
	} else if (err == 0) {
		/* nothing to read, eof */
		*read = 0;
		*eof = true;
	} else if (err < 0) {
		/* error */
		return ARCHIVE_INTERNAL_ERROR;
	}
	
	return ARCHIVE_OK;
}

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

int archive_read(const char *path)
{
	unzFile handle;
	int err;

	handle = open_archive(path);
	if (!handle)
		return ARCHIVE_OPEN_ERROR;

	err = extract_conference_file(handle);
	if (err != ARCHIVE_OK)
		return err;

	close_archive(handle);

	return ARCHIVE_OK;
}
