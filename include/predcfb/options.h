#ifndef OPTIONS_H
#define OPTIONS_H

#include <stdbool.h>

extern const char *progname;

extern bool opt_help;
extern bool opt_version;
extern const char *opt_archive;

int options_parse(int argc, char **argv);

#endif
