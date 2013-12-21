
#include <stdio.h>
#include <stdint.h>
#include <time.h>
#include <string.h>
#include <assert.h>

#include <predcfb/cfbstats.h>
#include <predcfb/predcfb.h>
#include <predcfb/fieldlist.h>
#include <predcfb/objectid.h>

#include "cfbstats_internal.h"

extern const char *progname;

static int get_ownid(struct linehandler *lh, int *id)
{
	int err;

	err = fieldlist_int_at(lh->flist, lh->current->index, id);
	if (err != FIELDLIST_OK) {
		/* FIXME */
		return CFBSTATS_ERROR;
	}

	return CFBSTATS_OK;
}

static int get_owngameid(struct linehandler *lh, int *id)
{
	int err;
	const char *str;

	err = fieldlist_str_at(lh->flist, lh->current->index, &str);
	if (err != FIELDLIST_OK) {
		/* FIXME */
		return CFBSTATS_ERROR;
	}

	*id = pack_game_code(str);

	return CFBSTATS_OK;
}

static int get_str(struct linehandler *lh)
{
	const char *str;
	const struct fielddesc *cur = lh->current;
	char *outbuf = (char*) (((intptr_t) lh->obj) + cur->offset);

	if (fieldlist_str_at(lh->flist, cur->index, &str) != FIELDLIST_OK) {
		/* FIXME */
		return CFBSTATS_ERROR;
	}

	strncpy(outbuf, str, cur->len);

	return CFBSTATS_OK;
}

static int get_short(struct linehandler *lh)
{
	const struct fielddesc *cur = lh->current;
	short *sout = (short*) (((intptr_t) lh->obj) + cur->offset);

	if (fieldlist_short_at(lh->flist, cur->index, sout) != FIELDLIST_OK) {
		const char *err = fieldlist_strerror(lh->flist);
		fprintf(stderr, "%s: error parsing field index %d (line %d): %s\n",
				progname,
				cur->index,
				lh->flist->line,
				err);
		return CFBSTATS_ERROR;
	}

	return CFBSTATS_OK;
}

static int get_conf_enum(struct linehandler *lh)
{
	const char *str;
	const struct fielddesc *cur = lh->current;
	intptr_t pval = ((intptr_t) lh->obj) + cur->offset;
	enum conference_division *outdiv = (enum conference_division*) pval;

	if (fieldlist_str_at(lh->flist, cur->index, &str) != FIELDLIST_OK) {
		/* FIXME */
		return CFBSTATS_ERROR;
	}

	if (strcmp("FBS", str) == 0) {
		*outdiv = CONFERENCE_FBS;
	} else if (strcmp("FCS", str) == 0) {
		*outdiv = CONFERENCE_FCS;
	} else {
		fprintf(stderr, "%s: invalid value for conference division on line %d\n", progname, lh->flist->line);
		return CFBSTATS_ERROR;
	}

	return CFBSTATS_OK;
}

static int get_site_bool(struct linehandler *lh)
{
	const char *str;
	const struct fielddesc *cur = lh->current;
	intptr_t pbool = ((intptr_t) lh->obj) + cur->offset;
	bool *outbool = (bool*) pbool;

	if (fieldlist_str_at(lh->flist, cur->index, &str) != FIELDLIST_OK) {
		/* FIXME */
		return CFBSTATS_ERROR;
	}

	if (strcmp(str, "TEAM") == 0) {
		*outbool = false;
	} else if (strcmp(str, "NEUTRAL") == 0) {
		*outbool = true;
	} else {
		cfbstats_errno = CFBSTATS_EINVALIDFILE;
		fprintf(stderr, "%s: invalid value for game site (line %d)\n", progname, lh->flist->line);
		return CFBSTATS_ERROR;
	}

	return CFBSTATS_OK;
}

static int get_confid(struct linehandler *lh)
{
	const struct fielddesc *cur = lh->current;
	int id;
	const struct objectid *oid;
	intptr_t poid = ((intptr_t) lh->obj) + cur->offset;
	struct objectid *outoid = (struct objectid*) poid;

	if (fieldlist_int_at(lh->flist, cur->index, &id) != FIELDLIST_OK) {
		/* FIXME */
		return CFBSTATS_ERROR;
	}

	if ((oid = id_map_lookup(id)) == NULL) {
		fprintf(stderr, "%s: conference id does not exist (line %d)\n", progname, lh->flist->line);
		return CFBSTATS_ERROR;
	}

	*outoid = *oid;

	return CFBSTATS_OK;
}

static int get_teamid(struct linehandler *lh)
{
	const struct fielddesc *cur = lh->current;
	int id;
	const struct objectid *oid;
	intptr_t poid = ((intptr_t) lh->obj) + cur->offset;
	struct objectid *outoid = (struct objectid*) poid;

	if (fieldlist_int_at(lh->flist, cur->index, &id) != FIELDLIST_OK) {
		/* FIXME */
		return CFBSTATS_ERROR;
	}

	if ((oid = id_map_lookup(id)) == NULL) {
		fprintf(stderr, "%s: team id does not exist (line %d)\n", progname, lh->flist->line);
		return CFBSTATS_ERROR;
	}

	*outoid = *oid;

	return CFBSTATS_OK;
}

static int get_gameid(struct linehandler *lh)
{
	const struct fielddesc *cur = lh->current;
	const char *str;
	int id;
	const struct objectid *oid;
	intptr_t poid = ((intptr_t) lh->obj) + cur->offset;
	struct objectid *outoid = (struct objectid*) poid;

	if (fieldlist_str_at(lh->flist, cur->index, &str) != FIELDLIST_OK) {
		const char *err = fieldlist_strerror(lh->flist);
		fprintf(stderr, "%s: error parsing field index %d (line %d): %s\n",
				progname,
				cur->index,
				lh->flist->line,
				err);
		return CFBSTATS_ERROR;
	}

	id = pack_game_code(str);

	if ((oid = id_map_lookup(id)) == NULL) {
		fprintf(stderr, "%s: game id does not exist (line %d)\n", progname, lh->flist->line);
		return CFBSTATS_ERROR;
	}

	*outoid = *oid;

	return CFBSTATS_OK;
}

static int get_date(struct linehandler *lh)
{
	/* dates are MM/DD/YYYY HH:MM:SS -ZZZZ */
	static const int DATE_BUF_SIZE = 26;
	const char *str;
	const struct fielddesc *cur = lh->current;
	intptr_t ptime = ((intptr_t) lh->obj) + cur->offset;
	time_t *outtime = (time_t*) ptime;
	char date_buf[DATE_BUF_SIZE];
	struct tm tm;
	const char *lastchar;

	if (fieldlist_str_at(lh->flist, cur->index, &str) != FIELDLIST_OK) {
		/* FIXME */
		return CFBSTATS_ERROR;
	}

	snprintf(date_buf, DATE_BUF_SIZE, "%s 00:00:00 +0000", str);
	lastchar = strptime(date_buf, "%m/%d/%Y %H:%M:%S %z", &tm);
	if (!lastchar || *lastchar != '\0') {
		cfbstats_errno = CFBSTATS_EINVALIDFILE;
		return CFBSTATS_ERROR;
	}

	*outtime = mktime(&tm);

	return CFBSTATS_OK;
}

int linehandler_parse(struct linehandler *lh, int *id)
{
	assert(lh->descriptions != NULL);

	lh->current = lh->descriptions;

	while (lh->current->type != FIELD_TYPE_END) {
		switch (lh->current->type) {
		case FIELD_TYPE_OWNID:
			if (get_ownid(lh, id) != CFBSTATS_OK)
				return CFBSTATS_ERROR;
			break;

		case FIELD_TYPE_OWNGAMEID:
			if (get_owngameid(lh, id) != CFBSTATS_OK)
				return CFBSTATS_ERROR;
			break;
		
		case FIELD_TYPE_STR:
			if (get_str(lh) != CFBSTATS_OK)
				return CFBSTATS_ERROR;
			break;

		case FIELD_TYPE_SHORT:
			if (get_short(lh) != CFBSTATS_OK)
				return CFBSTATS_ERROR;
			break;

		case FIELD_TYPE_DATE:
			if (get_date(lh) != CFBSTATS_OK)
				return CFBSTATS_ERROR;
			break;

		case FIELD_TYPE_CONFERENCE_ENUM:
			if (get_conf_enum(lh) != CFBSTATS_OK)
				return CFBSTATS_ERROR;
			break;

		case FIELD_TYPE_CONFID:
			if (get_confid(lh) != CFBSTATS_OK)
				return CFBSTATS_ERROR;
			break;

		case FIELD_TYPE_TEAMID:
			if (get_teamid(lh) != CFBSTATS_OK)
				return CFBSTATS_ERROR;
			break;

		case FIELD_TYPE_GAMEID:
			if (get_gameid(lh) != CFBSTATS_OK)
				return CFBSTATS_ERROR;
			break;

		case FIELD_TYPE_SITE_BOOL:
			if (get_site_bool(lh) != CFBSTATS_OK)
				return CFBSTATS_ERROR;
			break;

		case FIELD_TYPE_END:
			/* this should never happen... */
			break;
		}

		lh->current++;
	}

	return CFBSTATS_OK;
}

