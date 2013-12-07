#ifndef OPTIONS_H
#define OPTIONS_H

#include <stdbool.h>

extern const char *progname;

extern bool opt_help;
extern bool opt_version;
extern bool opt_save;

extern const char *opt_archive;
extern const char *opt_save_file;

int options_parse(int argc, char **argv);

#endif
