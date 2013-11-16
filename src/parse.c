
#include <stdlib.h>
#include <errno.h>
#include <assert.h>
#include <limits.h>
#include <string.h>
#include <stdio.h>

#include "parse.h"
#include "conference.h"

const struct parse_handler parse_handlers[] = {
	{ "conference.csv", PARSE_FILE_CSV, parse_conference_csv },
	{ NULL, PARSE_FILE_NONE, NULL }
};

/* subtract 1 from total to account for ending null struct */
const int num_parse_handlers = (sizeof(parse_handlers) / sizeof(struct parse_handler)) - 1;

/* conversion functions */

static int parse_short(const char *str, short *out)
{
	long int li;
	char *endptr;

	li = strtol(str, &endptr, 10);

	if (*endptr != '\0') {
		/* there was a bad char, str is not a number */
		return PARSE_ERROR;
	}

	if (li == LONG_MIN || li == LONG_MAX) {
		if (errno == ERANGE)
			return PARSE_ERROR;
	}

	if (li < SHRT_MIN || li > SHRT_MAX)
		return PARSE_ERROR;

	*out = (short) li;

	return PARSE_OK;
}

/* parsing functions */

int parse_conference_csv(struct fieldlist *f)
{
	struct conference *conf;
	const char *str;
	size_t len;

	assert(f->num_fields == 3);

	conf = conference_create();
	if (!conf) {
		/* too many conferences! */
		return PARSE_ERROR;
	}

	/* id field */
	str = fieldlist_iter_begin(f);
	if (parse_short(str, &conf->id) != PARSE_OK)
		return PARSE_ERROR;

	/* conference name */
	str = fieldlist_iter_next(f);
	len = strlen(str);
	if (len >= CONFERENCE_NAME_MAX)
		return PARSE_ERROR;
	strncpy(conf->name, str, CONFERENCE_NAME_MAX);

	/* division */
	str = fieldlist_iter_next(f);
	if (strcmp("FBS", str) == 0)
		conf->div = CONFERENCE_FBS;
	else if (strcmp("FCS", str) == 0)
		conf->div = CONFERENCE_FCS;
	else
		return PARSE_ERROR;

	printf("%d %s (%d)\n", conf->id, conf->name, (int)conf->div);

	return PARSE_OK;
}
