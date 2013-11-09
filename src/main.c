
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include <getopt.h>

#include "config.h"
#include "archive.h"

static bool opt_help = false;
static bool opt_version = false;
static const char *opt_archive = NULL;

enum long_opts {
	LONG_OPT_HELP,
	LONG_OPT_VERSION
};

static int parse_options(int argc, char **argv)
{
	int c;
	int index;
	bool require_file = true;
	static const struct option long_options[] = {
		{ "help", 0, NULL, LONG_OPT_HELP },
		{ "version", 0, NULL, LONG_OPT_VERSION },
		{ NULL, 0, NULL, 0 }
	};

	while ((c = getopt_long(argc, argv, "", long_options, &index)) != -1) {
		switch (c) {
		case LONG_OPT_HELP:
			opt_help = true;
			require_file = false;
			break;

		case LONG_OPT_VERSION:
			opt_version = true;
			require_file = false;
			break;

		case '?':
			return -1;
		}
	}

	/* handle non-options */
	while (optind < argc) {
		if (!opt_archive) {
			opt_archive = argv[optind];
		} else {
			fprintf(stderr, "Only one file can be specified\n");
			return -2;
		}

		optind++;
	}

	/* ensure that a filename was provided */
	if (!opt_archive && require_file) {
		fprintf(stderr, "%s: missing file operand\n", argv[0]);
		return -3;
	}

	return 0;
}

static void print_help(void)
{
	static const char *usage =
		"usage: predcfb [--help] [--version] <zip file>\n"
		"\tthe zip file containing parsable data can be found at www.cfbstats.com";

	puts(usage);
	exit(EXIT_SUCCESS);
}

static void print_version(void)
{
	printf("predcfb version %d.%d.%d\n",
	       PREDCFB_VERSION_MAJOR,
	       PREDCFB_VERSION_MINOR,
	       PREDCFB_VERSION_PATCH);

	exit(EXIT_SUCCESS);
}

int main(int argc, char **argv)
{
	int err;

	err = parse_options(argc, argv);
	if (err)
		exit(EXIT_FAILURE);

	if (opt_help)
		print_help();

	if (opt_version)
		print_version();

	err = archive_read(opt_archive);
	if (err)
		exit(EXIT_FAILURE);

	exit(EXIT_SUCCESS);
}
