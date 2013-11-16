
#include <stdio.h>

#include "parse.h"

const struct parse_handler parse_handlers[] = {
	{ "conference.csv", PARSE_FILE_CSV, parse_conference_csv },
	{ NULL, PARSE_FILE_NONE, NULL }
};

const int num_parse_handlers = sizeof(parse_handlers) / sizeof(struct parse_handler);

/* parsing functions */

int parse_conference_csv(struct fieldlist *f)
{
	const char *str;
	int count = 0;

	str = fieldlist_iter_begin(f);

	while (str) {
		if (count == 0)
			fputc('[', stdout);

		fprintf(stdout, "'%s'", str);

		if (count + 1 == f->num_fields)
			fputs("]\n", stdout);
		else
			fputs(", ", stdout);

		str = fieldlist_iter_next(f);
		count++;
	}

	return PARSE_OK;
}
