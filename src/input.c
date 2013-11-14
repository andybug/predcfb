
#include <stdio.h>

#include <unistd.h>

#include "input.h"
#include "archive.h"
#include "parse.h"

struct read_context {
	archive_ctx ac;
	parse_ctx pc;
};

static int check_access(const char *path)
{
	int err;

	err = access(path, R_OK);
	if (err)
		return INPUT_ACCESS_ERROR;

	return INPUT_OK;
}

static int input_open_archive(struct read_context *rc, const char *path)
{
	int err;

	err = check_access(path);
	if (err)
		return err;

	err = archive_open(&rc->ac, path);

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

static int input_close_archive(struct read_context *rc)
{
	int err;

	err = archive_close(rc->ac);
	if (err)
		return INPUT_ARCHIVE_ERROR;

	rc->ac = NULL;

	return INPUT_OK;
}

static int input_open_file(struct read_context *rc, const char *file)
{
	int err;

	err = archive_open_file(rc->ac, file);
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

static int input_init_parser(struct read_context *rc, const char *file)
{
	int err;

	err = parse_init(&rc->pc, file);
	switch (err) {
	case PARSE_MEMORY_ERROR:
		fputs("out of memory error\n", stderr);
		return INPUT_MEMORY_ERROR;

	case PARSE_INTERNAL_ERROR:
		fputs("error initializing parser\n", stderr);
		return INPUT_PARSE_ERROR;
	}

	return INPUT_OK;
}

static int input_read_file(struct read_context *rc, const char *file)
{
	static const size_t BUF_SIZE = 4096;
	char buf[BUF_SIZE];
	size_t read;
	int err;

	err = input_open_file(rc, file);
	if (err)
		return err;

	err = input_init_parser(rc, file);
	if (err)
		return err;

	for (;;) {
		err = archive_read_file(rc->ac, buf, BUF_SIZE, &read);

		if (err == ARCHIVE_OK || (err == ARCHIVE_EOF && read > 0)) {
			err = parse_data(rc->pc, buf, read);
			if (err)
				return INPUT_PARSE_ERROR;
		}

		if (err == ARCHIVE_EOF)
			break;

		else if (err == ARCHIVE_INTERNAL_ERROR)
			return INPUT_ARCHIVE_ERROR;
	}

	err = parse_finish(rc->pc);
	if (err)
		return INPUT_PARSE_ERROR;

	err = archive_close_file(rc->ac);
	if (err)
		return INPUT_ARCHIVE_ERROR;

	return INPUT_OK;
}

int input_read_archive(const char *path)
{
	int err;
	struct read_context rc;

	err = input_open_archive(&rc, path);
	if (err)
		return err;

	err = input_read_file(&rc, "conference.csv");
	if (err)
		return err;

	err = input_close_archive(&rc);
	if (err)
		return err;

	return INPUT_OK;
}
