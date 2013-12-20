
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

#include "cfbstats/cfbstats_internal.h"

extern const char *progname;

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

	assert(num_fdesc_conference <= total_fields_conference);
	assert(num_fdesc_team <= total_fields_team);
	assert(num_fdesc_game <= total_fields_game);

	id_map_clear();
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

static int check_csv_header_2(
		struct fieldlist *f,
		const struct fielddesc *desc_list)
{
	const char *field;
	const struct fielddesc *desc = desc_list;

	while (desc->type != FIELD_TYPE_END) {
		if (fieldlist_str_at(f, desc->index, &field) != FIELDLIST_OK)
			return CFBSTATS_ERROR;

		if (strcmp(field, desc->name) != 0) {
			cfbstats_errno = CFBSTATS_EINVALIDFILE;
			return CFBSTATS_ERROR;
		}

		desc++;
	}

	return CFBSTATS_OK;
}

/* parse conference.csv */

int parse_conference_csv(struct fieldlist *f)
{
	struct linehandler handler;
	struct conference *conf;
	struct objectid oid;
	int id;

	if (f->num_fields != total_fields_conference) {
		cfbstats_errno = CFBSTATS_EINVALIDFILE;
		return CFBSTATS_ERROR;
	}

	if (f->line == 1) {
		return check_csv_header_2(f, fdesc_conference);
	}

	conf = objectdb_create_conference();
	if (!conf) {
		/* too many conferences!
		 * TODO: print error string
		 */
		return CFBSTATS_ERROR;
	}

	handler.descriptions = fdesc_conference;
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

int parse_team_csv(struct fieldlist *f)
{
	struct linehandler handler;
	struct objectid oid;
	int id;
	struct team *team;

	if (f->num_fields != total_fields_team) {
		cfbstats_errno = CFBSTATS_EINVALIDFILE;
		return CFBSTATS_ERROR;
	}

	if (f->line == 1) {
		return check_csv_header_2(f, fdesc_team);
	}

	if ((team = objectdb_create_team()) == NULL) {
		cfbstats_errno = CFBSTATS_ETOOMANY;
		return CFBSTATS_ERROR;
	}

	handler.descriptions = fdesc_team;
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

int parse_game_csv(struct fieldlist *f)
{
	struct linehandler handler;
	struct objectid oid;
	int id;
	struct game *game;

	if (f->num_fields != total_fields_game) {
		cfbstats_errno = CFBSTATS_EINVALIDFILE;
		return CFBSTATS_ERROR;
	}

	if (f->line == 1) {
		return check_csv_header_2(f, fdesc_game);
	}

	if ((game = objectdb_create_game()) == NULL) {
		cfbstats_errno = CFBSTATS_ETOOMANY;
		return CFBSTATS_ERROR;
	}

	handler.descriptions = fdesc_game;
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
	struct stats_wrapper sw;
	struct linehandler handler;
	int id; // ignore this

	if (f->num_fields != total_fields_stats) {
		cfbstats_errno = CFBSTATS_EINVALIDFILE;
		return CFBSTATS_ERROR;
	}

	if (f->line == 1) {
		return check_csv_header_2(f, fdesc_stats);
	}

	handler.descriptions = fdesc_stats;
	handler.flist = f;
	handler.obj = &sw;

	/* parse the fields */
	if (linehandler_parse(&handler, &id) != CFBSTATS_OK)
		return CFBSTATS_ERROR;

#if 0
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
#endif

	return CFBSTATS_OK;
}

const char *cfbstats_strerror(void)
{
	return cfbstats_errors[cfbstats_errno];
}
