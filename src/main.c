
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include <getopt.h>

static bool opt_help = false;
static bool opt_version = false;

enum long_opts {
	LONG_OPT_HELP,
	LONG_OPT_VERSION
};

static int parse_options(int argc, char **argv)
{
	int c;
	int index;
	static const struct option long_options[] = {
		{ "help", 0, NULL, LONG_OPT_HELP },
		{ "version", 0, NULL, LONG_OPT_VERSION },
		{ NULL, 0, NULL, 0 }
	};

	while ((c = getopt_long(argc, argv, "", long_options, &index)) != -1) {
		switch (c) {
		case LONG_OPT_HELP:
			opt_help = true;
			break;

		case LONG_OPT_VERSION:
			opt_version = true;
			break;

		case '?':
			return -1;
		}
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
	puts("predcfb 0.1.0");
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

	exit(EXIT_SUCCESS);
}
