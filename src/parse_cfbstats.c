
#include <stdlib.h>
#include <errno.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <time.h>
#include <stddef.h>
#include <stdint.h>
#include <limits.h>

#include <predcfb/cfbstats.h>
#include <predcfb/predcfb.h>
#include <predcfb/objectdb.h>
#include <predcfb/zipfile.h>
#include <predcfb/csvparse.h>

#define CFBSTATS_ID_MAP_SIZE 4096

extern const char *progname;

struct map_entry {
	int id;
	struct objectid oid;
};

enum field_type {
	FIELD_TYPE_END,
	FIELD_TYPE_OWNID,
	FIELD_TYPE_OWNGAMEID,
	FIELD_TYPE_CONFID,
	FIELD_TYPE_TEAMID,
	FIELD_TYPE_GAMEID,
	FIELD_TYPE_STR,
	FIELD_TYPE_SHORT,
	FIELD_TYPE_DATE,
	/* special types */
	FIELD_TYPE_CONFERENCE_ENUM,
	FIELD_TYPE_SITE_BOOL
};

struct fielddesc {
	int index;
	const char *name;
	enum field_type type;
	size_t len;
	size_t offset;
};

struct linehandler {
	const struct fielddesc *descriptions;
	const struct fielddesc *current;
	struct fieldlist *flist;
	void *obj;
};

static struct map_entry id_map[CFBSTATS_ID_MAP_SIZE];

enum cfbstats_err cfbstats_errno = CFBSTATS_ENONE;
static const char *cfbstats_errors[] = {
	"No error",
	"Error parsing zip file",
	"Memory allocation failed",
	"File does not match expected format",
	"Too many records",
	"Failed cfbstats id lookup",
	"Failed objectid lookup"
};

/* initialization functions */

void cfbstats_init(void)
{
	/* make sure that the id map size is a power of 2 */
	assert(!(CFBSTATS_ID_MAP_SIZE & (CFBSTATS_ID_MAP_SIZE - 1)));

	memset(id_map, 0, sizeof(id_map));
}

/* cfbstats id to objectid map */

static int id_map_insert(int id, const struct objectid *oid)
{
	static const int mask = CFBSTATS_ID_MAP_SIZE - 1;
	struct map_entry *entry;
	int i;
	int count = 0;

	i = id & mask;

	while (count < CFBSTATS_ID_MAP_SIZE) {
		entry = &id_map[i];
		if (entry->id == 0) {
			entry->id = id;
			entry->oid = *oid;
			break;
		}

		i = (i + 1) & mask;
		count++;
	}

	if (count == CFBSTATS_ID_MAP_SIZE)
		return CFBSTATS_ERROR;

	return CFBSTATS_OK;
}

static const struct objectid *id_map_lookup(int id)
{
	static const int mask = CFBSTATS_ID_MAP_SIZE - 1;
	struct map_entry *entry;
	int i;
	int count = 0;

	i = id & mask;

	while (count < CFBSTATS_ID_MAP_SIZE) {
		entry = &id_map[i];
		if (entry->id == id)
			return &entry->oid;

		i = (i + 1) & mask;
		count++;
	}

	return NULL;
}

static int pack_game_code(const char *str)
{
	int id;
	const char *team1, *team2, *date, *mmdd;
	static const size_t team_str_len = 4;
	static const size_t date_str_len = 8;
	static const size_t mmdd_str_len = 4;
	static const size_t buf_size = date_str_len + 1;
	char buf[buf_size];
	int code;

	team1 = str;
	team2 = (str + team_str_len);
	date = (str + (team_str_len * 2));
	mmdd = date + 4;

	/* team1 shifted into bits 16-31 */
	memcpy(buf, team1, team_str_len);
	buf[team_str_len] = '\0';
	code = atoi(buf);
	id = (code << 16) & 0xffff0000;

	/* team2 xor'd into bits 16-31 */
	memcpy(buf, team2, team_str_len);
	buf[team_str_len] = '\0';
	code = atoi(buf);
	id ^= (code << 16) & 0xffff0000;

	/* month and day (mmdd) put into bits 0-15 */
	memcpy(buf, mmdd, mmdd_str_len);
	buf[mmdd_str_len] = '\0';
	code = atoi(buf);
	id |= code & 0x0000ffff;

	return id;
}

/* line handler functions */

static int linehandler_get_ownid(struct linehandler *lh, int *id)
{
	int err;

	err = fieldlist_int_at(lh->flist, lh->current->index, id);
	if (err != FIELDLIST_OK) {
		/* FIXME */
		return CFBSTATS_ERROR;
	}

	return CFBSTATS_OK;
}

static int linehandler_get_owngameid(struct linehandler *lh, int *id)
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

static int linehandler_get_str(struct linehandler *lh)
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

static int linehandler_get_conf_enum(struct linehandler *lh)
{
	const char *str;
	const struct fielddesc *cur = lh->current;
	size_t pval = ((intptr_t) lh->obj) + cur->offset;
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

static int linehandler_get_site_bool(struct linehandler *lh)
{
	const char *str;
	const struct fielddesc *cur = lh->current;
	size_t pbool = ((intptr_t) lh->obj) + cur->offset;
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

static int linehandler_get_confid(struct linehandler *lh)
{
	const struct fielddesc *cur = lh->current;
	int id;
	const struct objectid *oid;
	size_t poid = ((intptr_t) lh->obj) + cur->offset;
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

static int linehandler_get_teamid(struct linehandler *lh)
{
	const struct fielddesc *cur = lh->current;
	int id;
	const struct objectid *oid;
	size_t poid = ((intptr_t) lh->obj) + cur->offset;
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

static int linehandler_get_gameid(struct linehandler *lh)
{
	const struct fielddesc *cur = lh->current;
	int id;
	const struct objectid *oid;
	size_t poid = ((intptr_t) lh->obj) + cur->offset;
	struct objectid *outoid = (struct objectid*) poid;

	if (fieldlist_int_at(lh->flist, cur->index, &id) != FIELDLIST_OK) {
		/* FIXME */
		return CFBSTATS_ERROR;
	}

	if ((oid = id_map_lookup(id)) == NULL) {
		fprintf(stderr, "%s: game id does not exist (line %d)\n", progname, lh->flist->line);
		return CFBSTATS_ERROR;
	}

	*outoid = *oid;

	return CFBSTATS_OK;
}

static int linehandler_get_date(struct linehandler *lh)
{
	/* dates are MM/DD/YYYY HH:MM:SS -ZZZZ */
	static const int DATE_BUF_SIZE = 26;
	const char *str;
	const struct fielddesc *cur = lh->current;
	size_t ptime = ((intptr_t) lh->obj) + cur->offset;
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

static int linehandler_parse(struct linehandler *lh, int *id)
{
	assert(lh->descriptions != NULL);

	lh->current = lh->descriptions;

	while (lh->current->type != FIELD_TYPE_END) {
		switch (lh->current->type) {
		case FIELD_TYPE_OWNID:
			if (linehandler_get_ownid(lh, id) != CFBSTATS_OK)
				return CFBSTATS_ERROR;
			break;

		case FIELD_TYPE_OWNGAMEID:
			if (linehandler_get_owngameid(lh, id) != CFBSTATS_OK)
				return CFBSTATS_ERROR;
			break;
		
		case FIELD_TYPE_STR:
			if (linehandler_get_str(lh) != CFBSTATS_OK)
				return CFBSTATS_ERROR;
			break;

		case FIELD_TYPE_DATE:
			if (linehandler_get_date(lh) != CFBSTATS_OK)
				return CFBSTATS_ERROR;
			break;

		case FIELD_TYPE_CONFERENCE_ENUM:
			if (linehandler_get_conf_enum(lh) != CFBSTATS_OK)
				return CFBSTATS_ERROR;
			break;

		case FIELD_TYPE_CONFID:
			if (linehandler_get_confid(lh) != CFBSTATS_OK)
				return CFBSTATS_ERROR;
			break;

		case FIELD_TYPE_TEAMID:
			if (linehandler_get_teamid(lh) != CFBSTATS_OK)
				return CFBSTATS_ERROR;
			break;

		case FIELD_TYPE_GAMEID:
			if (linehandler_get_gameid(lh) != CFBSTATS_OK)
				return CFBSTATS_ERROR;
			break;

		case FIELD_TYPE_SITE_BOOL:
			if (linehandler_get_site_bool(lh) != CFBSTATS_OK)
				return CFBSTATS_ERROR;
			break;
		}

		lh->current++;
	}

	return CFBSTATS_OK;
}

/* csv header verification */

#define NUM_FIELDS(a) (sizeof(a) / sizeof(*a))

static int check_csv_header(struct fieldlist *f, const char **fields, int num)
{
	int i;
	const char *field;

	assert(f->num_fields == num);

	fieldlist_iter_begin(f);

	for (i = 0; i < num; i++) {
		field = fieldlist_iter_next(f);
		if (strcmp(field, fields[i]) != 0) {
			cfbstats_errno = CFBSTATS_EINVALIDFILE;
			return CFBSTATS_ERROR;
		}
	}

	return CFBSTATS_OK;
}

static int check_csv_header_2(struct fieldlist *f,
                              const struct fielddesc *desc_list,
			      int num)
{
	int i;
	const char *field;
	const struct fielddesc *desc;

	for (i = 0; i < num; i++) {
		desc = &desc_list[i];

		if (desc->type == FIELD_TYPE_END)
			break;

		if (fieldlist_str_at(f, desc->index, &field) != FIELDLIST_OK)
			return CFBSTATS_ERROR;

		if (strcmp(field, desc->name) != 0) {
			cfbstats_errno = CFBSTATS_EINVALIDFILE;
			return CFBSTATS_ERROR;
		}
	}

	return CFBSTATS_OK;
}

/* parse conference.csv */

static const struct fielddesc desc_conference[] = {
	{
		.index = 0,
		.name = "Conference Code",
		.type = FIELD_TYPE_OWNID,
		.len = 0,
		.offset = 0
	},
	{
		.index = 1,
		.name = "Name",
		.type = FIELD_TYPE_STR,
		.len = CONFERENCE_NAME_MAX,
		.offset = offsetof(struct conference, name)

	},
	{
		.index = 2,
		.name = "Subdivision",
		.type = FIELD_TYPE_CONFERENCE_ENUM,
		.len = 0,
		.offset = offsetof(struct conference, subdivision)
	},
	{
		.index = INT_MIN,
		.name = NULL,
		.type = FIELD_TYPE_END,
		.len = 0,
		.offset = 0
	}
};

int parse_conference_csv(struct fieldlist *f)
{
	static const int num_fields = NUM_FIELDS(desc_conference) - 1;

	struct linehandler handler;
	struct conference *conf;
	struct objectid oid;
	int id;

	if (f->num_fields != num_fields) {
		cfbstats_errno = CFBSTATS_EINVALIDFILE;
		return CFBSTATS_ERROR;
	}

	if (f->line == 1) {
		return check_csv_header_2(f, desc_conference, num_fields);
	}

	conf = objectdb_create_conference();
	if (!conf) {
		/* too many conferences!
		 * TODO: print error string
		 */
		return CFBSTATS_ERROR;
	}

	handler.descriptions = desc_conference;
	handler.flist = f;
	handler.obj = conf;

	/* parse the fields */
	if (linehandler_parse(&handler, &id) != CFBSTATS_OK)
		return CFBSTATS_ERROR;

	/* add the conference to the objectdb */
	if (objectdb_add_conference(conf, &oid) != OBJECTDB_OK)
		return CFBSTATS_ERROR;

	/* add the conference to the id map */
	if (id_map_insert(id, &oid) != CFBSTATS_OK)
		return CFBSTATS_ERROR;

	return CFBSTATS_OK;
}

/* parse team.csv */

static const struct fielddesc desc_team[] = {
	{
		.index = 0,
		.name = "Team Code",
		.type = FIELD_TYPE_OWNID,
		.len = 0,
		.offset = 0
	},
	{
		.index = 1,
		.name = "Name",
		.type = FIELD_TYPE_STR,
		.len = TEAM_NAME_MAX,
		.offset = offsetof(struct team, name)
	},
	{
		.index = 2,
		.name = "Conference Code",
		.type = FIELD_TYPE_CONFID,
		.len = 0,
		.offset = offsetof(struct team, conf_oid)
	},
	{
		.index = INT_MIN,
		.name = NULL,
		.type = FIELD_TYPE_END,
		.len = 0,
		.offset = 0
	}
};

int parse_team_csv(struct fieldlist *f)
{
	static const int num_fields = NUM_FIELDS(desc_team) - 1;

	struct linehandler handler;
	struct objectid oid;
	int id;
	struct team *team;

	if (f->num_fields != num_fields) {
		cfbstats_errno = CFBSTATS_EINVALIDFILE;
		return CFBSTATS_ERROR;
	}

	if (f->line == 1) {
		return check_csv_header_2(f, desc_team, num_fields);
	}

	if ((team = objectdb_create_team()) == NULL) {
		cfbstats_errno = CFBSTATS_ETOOMANY;
		return CFBSTATS_ERROR;
	}

	handler.descriptions = desc_team;
	handler.flist = f;
	handler.obj = team;

	/* parse the fields */
	if (linehandler_parse(&handler, &id) != CFBSTATS_OK)
		return CFBSTATS_ERROR;

	/* add the team to the object db */
	if (objectdb_add_team(team, &oid) != OBJECTDB_OK)
		return CFBSTATS_ERROR;

	/* add the team to the id map */
	if (id_map_insert(id, &oid) != CFBSTATS_OK)
		return CFBSTATS_ERROR;

	return CFBSTATS_OK;
}

/* parse game.csv */

static const struct fielddesc desc_game[] = {
	{
		.index = 0,
		.name = "Game Code",
		.type = FIELD_TYPE_OWNGAMEID,
		.len = 0,
		.offset = 0
	},
	{
		.index = 1,
		.name = "Date",
		.type = FIELD_TYPE_DATE,
		.len = 0,
		.offset = offsetof(struct game, date)
	},
	{
		.index = 2,
		.name = "Visit Team Code",
		.type = FIELD_TYPE_TEAMID,
		.len = 0,
		.offset = offsetof(struct game, away_oid)
	},
	{
		.index = 3,
		.name = "Home Team Code",
		.type = FIELD_TYPE_TEAMID,
		.len = 0,
		.offset = offsetof(struct game, home_oid)
	},
	{
		.index = 5,
		.name = "Site",
		.type = FIELD_TYPE_SITE_BOOL,
		.len = 0,
		.offset = offsetof(struct game, neutral)
	},
	{
		.index = INT_MIN,
		.name = NULL,
		.type = FIELD_TYPE_END,
		.len = 0,
		.offset = 0
	}
};

int parse_game_csv(struct fieldlist *f)
{
	static const int parse_fields = NUM_FIELDS(desc_game) - 1;
	static const int num_fields = 6;

	struct linehandler handler;
	struct objectid oid;
	int id;
	struct game *game;

	if (f->num_fields != num_fields) {
		cfbstats_errno = CFBSTATS_EINVALIDFILE;
		return CFBSTATS_ERROR;
	}

	if (f->line == 1) {
		return check_csv_header_2(f, desc_game, parse_fields);
	}

	if ((game = objectdb_create_game()) == NULL) {
		cfbstats_errno = CFBSTATS_ETOOMANY;
		return CFBSTATS_ERROR;
	}

	handler.descriptions = desc_game;
	handler.flist = f;
	handler.obj = game;

	/* parse the fields */
	if (linehandler_parse(&handler, &id) != CFBSTATS_OK)
		return CFBSTATS_ERROR;

	/* add the game to the db */
	if (objectdb_add_game(game, &oid) != OBJECTDB_OK)
		return CFBSTATS_ERROR;

	/* finally, add the id to the id map */
	if (id_map_insert(id, &oid) != CFBSTATS_OK)
		return CFBSTATS_ERROR;

	return CFBSTATS_OK;
}

/* parse team-game-statistics.csv */

static void set_stat_value(struct game *g, size_t field_off,
                           bool home, short val)
{
	intptr_t field_addr;
	short *field;

	if (home) {
		field_addr = ((intptr_t)&g->home_stats) + field_off;
		field = (short*)field_addr;
		*field = val;
	} else {
		field_addr = ((intptr_t)&g->away_stats) + field_off;
		field = (short*)field_addr;
		*field = val;
	}
}

int parse_stats_csv(struct fieldlist *f)
{
	static const char *fields[] = {
		"Team Code",
		"Game Code",
		"Rush Att",
		"Rush Yard",
		"Rush TD",
		"Pass Att",
		"Pass Comp",
		"Pass Yard",
		"Pass TD",
		"Pass Int",
		"Pass Conv",
		"Kickoff Ret",
		"Kickoff Ret Yard",
		"Kickoff Ret TD",
		"Punt Ret",
		"Punt Ret Yard",
		"Punt Ret TD",
		"Fum Ret",
		"Fum Ret Yard",
		"Fum Ret TD",
		"Int Ret",
		"Int Ret Yard",
		"Int Ret TD",
		"Misc Ret",
		"Misc Ret Yard",
		"Misc Ret TD",
		"Field Goal Att",
		"Field Goal Made",
		"Off XP Kick Att",
		"Off XP Kick Made",
		"Off 2XP Att",
		"Off 2XP Made",
		"Def 2XP Att",
		"Def 2XP Made",
		"Safety",
		"Points",
		"Punt",
		"Punt Yard",
		"Kickoff",
		"Kickoff Yard",
		"Kickoff Touchback",
		"Kickoff Out-Of-Bounds",
		"Kickoff Onside",
		"Fumble",
		"Fumble Lost",
		"Tackle Solo",
		"Tackle Assist",
		"Tackle For Loss",
		"Tackle For Loss Yard",
		"Sack",
		"Sack Yard",
		"QB Hurry",
		"Fumble Forced",
		"Pass Broken Up",
		"Kick/Punt Blocked",
		"1st Down Rush",
		"1st Down Pass",
		"1st Down Penalty",
		"Time Of Possession",
		"Penalty",
		"Penalty Yard",
		"Third Down Att",
		"Third Down Conv",
		"Fourth Down Att",
		"Fourth Down Conv",
		"Red Zone Att",
		"Red Zone TD",
		"Red Zone Field Goal"
	};
	static const int NUM_STAT_FIELDS = NUM_FIELDS(fields);
	static bool processed_header = false;

	const char *str;
	int id;
	struct team *team;
	struct game *game;
	const struct objectid *oid;
	short sval;
	bool home;

	if (f->num_fields != NUM_STAT_FIELDS) {
		cfbstats_errno = CFBSTATS_EINVALIDFILE;
		return CFBSTATS_ERROR;
	}

	if (!processed_header) {
		processed_header = true;
		return check_csv_header(f, fields, NUM_STAT_FIELDS);
	}

	fieldlist_iter_begin(f);

	/* get team pointer from team code */
	if (fieldlist_iter_next_int(f, &id) != FIELDLIST_OK) {
		cfbstats_errno = CFBSTATS_EINVALIDFILE;
		return CFBSTATS_ERROR;
	}

	if ((oid = id_map_lookup(id)) == NULL) {
		cfbstats_errno = CFBSTATS_EIDLOOKUP;
		return CFBSTATS_ERROR;
	}

	if ((team = objectdb_get_team(oid)) == NULL) {
		cfbstats_errno = CFBSTATS_EOIDLOOKUP;
		return CFBSTATS_ERROR;
	}

	/* get game pointer from game code */
	str = fieldlist_iter_next(f);
	id = pack_game_code(str);

	if ((oid = id_map_lookup(id)) == NULL) {
		cfbstats_errno = CFBSTATS_EIDLOOKUP;
		return CFBSTATS_ERROR;
	}

	if ((game = objectdb_get_game(oid)) == NULL) {
		cfbstats_errno = CFBSTATS_EOIDLOOKUP;
		return CFBSTATS_ERROR;
	}

	/* check whether we are parsing the home team or not */
	home = game->home == team ? true : false;

	/* rush attempts */
	if (fieldlist_iter_next_short(f, &sval) != FIELDLIST_OK) {
		cfbstats_errno = CFBSTATS_EINVALIDFILE;
		return CFBSTATS_ERROR;
	}
	set_stat_value(game, offsetof(struct stats, rush_att), home, sval);

	/* rush yards */
	if (fieldlist_iter_next_short(f, &sval) != FIELDLIST_OK) {
		cfbstats_errno = CFBSTATS_EINVALIDFILE;
		return CFBSTATS_ERROR;
	}
	set_stat_value(game, offsetof(struct stats, rush_yds), home, sval);

	/* rush tds */
	if (fieldlist_iter_next_short(f, &sval) != FIELDLIST_OK) {
		cfbstats_errno = CFBSTATS_EINVALIDFILE;
		return CFBSTATS_ERROR;
	}
	set_stat_value(game, offsetof(struct stats, rush_tds), home, sval);

	/* pass attempts */
	if (fieldlist_iter_next_short(f, &sval) != FIELDLIST_OK) {
		cfbstats_errno = CFBSTATS_EINVALIDFILE;
		return CFBSTATS_ERROR;
	}
	set_stat_value(game, offsetof(struct stats, pass_att), home, sval);

	/* pass completions */
	if (fieldlist_iter_next_short(f, &sval) != FIELDLIST_OK) {
		cfbstats_errno = CFBSTATS_EINVALIDFILE;
		return CFBSTATS_ERROR;
	}
	set_stat_value(game, offsetof(struct stats, pass_comp), home, sval);

	/* pass yards */
	if (fieldlist_iter_next_short(f, &sval) != FIELDLIST_OK) {
		cfbstats_errno = CFBSTATS_EINVALIDFILE;
		return CFBSTATS_ERROR;
	}
	set_stat_value(game, offsetof(struct stats, pass_yds), home, sval);

	/* pass tds */
	if (fieldlist_iter_next_short(f, &sval) != FIELDLIST_OK) {
		cfbstats_errno = CFBSTATS_EINVALIDFILE;
		return CFBSTATS_ERROR;
	}
	set_stat_value(game, offsetof(struct stats, pass_tds), home, sval);

	/* pass interceptions */
	if (fieldlist_iter_next_short(f, &sval) != FIELDLIST_OK) {
		cfbstats_errno = CFBSTATS_EINVALIDFILE;
		return CFBSTATS_ERROR;
	}
	set_stat_value(game, offsetof(struct stats, pass_int), home, sval);

	/* skip these fields */
	fieldlist_iter_next(f);
	fieldlist_iter_next(f);
	fieldlist_iter_next(f);
	fieldlist_iter_next(f);
	fieldlist_iter_next(f);
	fieldlist_iter_next(f);
	fieldlist_iter_next(f);
	fieldlist_iter_next(f);
	fieldlist_iter_next(f);
	fieldlist_iter_next(f);
	fieldlist_iter_next(f);
	fieldlist_iter_next(f);
	fieldlist_iter_next(f);
	fieldlist_iter_next(f);
	fieldlist_iter_next(f);
	fieldlist_iter_next(f);

	/* field goals attempted */
	if (fieldlist_iter_next_short(f, &sval) != FIELDLIST_OK) {
		cfbstats_errno = CFBSTATS_EINVALIDFILE;
		return CFBSTATS_ERROR;
	}
	set_stat_value(game, offsetof(struct stats, fg_att), home, sval);

	/* field goals made */
	if (fieldlist_iter_next_short(f, &sval) != FIELDLIST_OK) {
		cfbstats_errno = CFBSTATS_EINVALIDFILE;
		return CFBSTATS_ERROR;
	}
	set_stat_value(game, offsetof(struct stats, fg_made), home, sval);

	/* skip these fields */
	fieldlist_iter_next(f);
	fieldlist_iter_next(f);
	fieldlist_iter_next(f);
	fieldlist_iter_next(f);
	fieldlist_iter_next(f);
	fieldlist_iter_next(f);
	fieldlist_iter_next(f);

	/* points */
	if (fieldlist_iter_next_short(f, &sval) != FIELDLIST_OK) {
		cfbstats_errno = CFBSTATS_EINVALIDFILE;
		return CFBSTATS_ERROR;
	}
	set_stat_value(game, offsetof(struct stats, points), home, sval);

	/* skip these fields */
	fieldlist_iter_next(f);
	fieldlist_iter_next(f);
	fieldlist_iter_next(f);
	fieldlist_iter_next(f);
	fieldlist_iter_next(f);
	fieldlist_iter_next(f);
	fieldlist_iter_next(f);

	/* fumbles */
	if (fieldlist_iter_next_short(f, &sval) != FIELDLIST_OK) {
		cfbstats_errno = CFBSTATS_EINVALIDFILE;
		return CFBSTATS_ERROR;
	}
	set_stat_value(game, offsetof(struct stats, fumbles), home, sval);

	/* fumbles lost */
	if (fieldlist_iter_next_short(f, &sval) != FIELDLIST_OK) {
		cfbstats_errno = CFBSTATS_EINVALIDFILE;
		return CFBSTATS_ERROR;
	}
	set_stat_value(game, offsetof(struct stats, fumbles_lost), home, sval);

	/* skip these fields */
	fieldlist_iter_next(f);
	fieldlist_iter_next(f);
	fieldlist_iter_next(f);
	fieldlist_iter_next(f);
	fieldlist_iter_next(f);
	fieldlist_iter_next(f);
	fieldlist_iter_next(f);
	fieldlist_iter_next(f);
	fieldlist_iter_next(f);
	fieldlist_iter_next(f);
	fieldlist_iter_next(f);
	fieldlist_iter_next(f);
	fieldlist_iter_next(f);
	fieldlist_iter_next(f);

	/* penalties */
	if (fieldlist_iter_next_short(f, &sval) != FIELDLIST_OK) {
		cfbstats_errno = CFBSTATS_EINVALIDFILE;
		return CFBSTATS_ERROR;
	}
	set_stat_value(game, offsetof(struct stats, penalties), home, sval);

	/* penalty yards */
	if (fieldlist_iter_next_short(f, &sval) != FIELDLIST_OK) {
		cfbstats_errno = CFBSTATS_EINVALIDFILE;
		return CFBSTATS_ERROR;
	}
	set_stat_value(game, offsetof(struct stats, penalty_yds), home, sval);

	/* ignore the rest */

	return CFBSTATS_OK;
}

const char *cfbstats_strerror(void)
{
	return cfbstats_errors[cfbstats_errno];
}
