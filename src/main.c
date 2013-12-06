
#include <stdio.h>
#include <stdlib.h>

#include <config.h>
#include <predcfb/options.h>
#include <predcfb/cfbstats.h>

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
	progname = argv[0];

	if (options_parse(argc, argv) != 0)
		exit(EXIT_FAILURE);

	if (opt_help)
		print_help();

	if (opt_version)
		print_version();

	if (cfbstats_read_zipfile(opt_archive) != CFBSTATS_OK)
		exit(EXIT_FAILURE);

	exit(EXIT_SUCCESS);
}
