
#include <stdio.h>
#include <stdbool.h>

#include <unistd.h>

#include "input.h"
#include "archive.h"
#include "parse.h"

static int check_access(const char *path)
{
	int err;

	err = access(path, R_OK);
	if (err)
		return INPUT_ACCESS_ERROR;

	return INPUT_OK;
}

static int open_archive(archive_ctx *ac, const char *path)
{
	int err;

	err = check_access(path);
	if (err)
		return err;

	err = archive_open(ac, path);

	switch (err) {
	case ARCHIVE_MEMORY_ERROR:
		fputs("out of memory error\n", stderr);
		return INPUT_MEMORY_ERROR;
	
	case ARCHIVE_OPEN_ERROR:
		fputs("error opening archive\n", stderr);
		return INPUT_ARCHIVE_ERROR;
	}

	return INPUT_OK;
}

static int open_file(archive_ctx ac, const char *file)
{
	int err;

	err = archive_open_file(ac, file);
	switch (err) {
	case ARCHIVE_FILE_NOT_FOUND_ERROR:
		fputs("archive missing file\n", stderr);
		return INPUT_INVALID_ARCHIVE_ERROR;

	case ARCHIVE_INTERNAL_ERROR:
		fputs("error opening file in archive\n", stderr);
		return INPUT_ARCHIVE_ERROR;
	}

	return INPUT_OK;
}

static int read_file(archive_ctx ac, const char *file)
{
	static const size_t BUF_SIZE = 16;
	char buf[BUF_SIZE];
	size_t read;
	bool eof = false;
	int err;

	err = open_file(ac, file);
	if (err)
		return err;

	while (!eof) {
		err = archive_read_file(ac, buf, BUF_SIZE, &read);

		if (err == ARCHIVE_OK || (err == ARCHIVE_EOF && read > 0)) {
			/* call parsing functions */
			printf("parse '%.*s'\n", (int)read, buf);
		}

		if (err == ARCHIVE_EOF)
			eof = true;

		else if (err == ARCHIVE_INTERNAL_ERROR)
			return INPUT_ARCHIVE_ERROR;
	}

	err = archive_close_file(ac);
	if (err)
		return INPUT_ARCHIVE_ERROR;

	return INPUT_OK;
}

int input_read_archive(const char *path)
{
	int err;
	archive_ctx ac;

	err = open_archive(&ac, path);
	if (err)
		return err;

	err = read_file(ac, "conference.csv");
	if (err)
		return err;

	err = archive_close(ac);
	if (err)
		return INPUT_ARCHIVE_ERROR;

	return INPUT_OK;
}
