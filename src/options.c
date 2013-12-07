
#include <stdio.h>
#include <getopt.h>
#include <predcfb/options.h>

const char *progname;

bool opt_help = false;
bool opt_version = false;
bool opt_save = false;

const char *opt_archive = NULL;
const char *opt_save_file = "predcfb.yml";

enum long_opts {
	LONG_OPT_HELP,
	LONG_OPT_VERSION,
	LONG_OPT_SAVE
};

int options_parse(int argc, char **argv)
{
	int c;
	int index;
	bool require_file = true;
	static const struct option long_options[] = {
		{ "help", 0, NULL, LONG_OPT_HELP },
		{ "version", 0, NULL, LONG_OPT_VERSION },
		{ "save", 2, NULL, LONG_OPT_SAVE },
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

		case LONG_OPT_SAVE:
			opt_save = true;
			if (optarg)
				opt_save_file = optarg;
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
