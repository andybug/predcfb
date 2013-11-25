#ifndef CFBSTATS_H
#define CFBSTATS_H

#define CFBSTATS_OK       0
#define CFBSTATS_ERROR  (-1)

enum cfbstats_err {
	CFBSTATS_ENONE,
	CFBSTATS_EZIPFILE,
	CFBSTATS_ENOMEM,
	CFBSTATS_EINVALIDFILE,
	CFBSTATS_ETOOMANY,
	CFBSTATS_EIDLOOKUP,
	CFBSTATS_EOIDLOOKUP
};

extern enum cfbstats_err cfbstats_errno;
extern int cfbstats_read_zipfile(const char *archive);

#endif
