#ifndef PARSE_H
#define PARSE_H

#include <predcfb/fieldlist.h>

#define PARSE_OK	  0
#define PARSE_ERROR	(-1)

enum parse_err {
	PARSE_ENONE,
	PARSE_EZIPFILE
};

extern enum parse_err parse_errno;

extern int parse_conference_csv(struct fieldlist *f);
extern int parse_zipfile(const char *archive);

#endif
