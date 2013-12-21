
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

static int check_csv_header(
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
		return check_csv_header(f, fdesc_conference);
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
		return check_csv_header(f, fdesc_team);
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
		return check_csv_header(f, fdesc_game);
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

static void update_team_stats(struct team *team, struct stats *stats)
{
	team->stats.rush_att += stats->rush_att;
	team->stats.rush_yds += stats->rush_yds;
	team->stats.rush_tds += stats->rush_tds;

	team->stats.pass_att += stats->pass_att;
	team->stats.pass_comp += stats->pass_comp;
	team->stats.pass_yds += stats->pass_yds;
	team->stats.pass_tds += stats->pass_tds;
	team->stats.pass_int += stats->pass_int;

	team->stats.fumbles += stats->fumbles;
	team->stats.fumbles_lost += stats->fumbles_lost;

	team->stats.points += stats->points;
}

int parse_stats_csv(struct fieldlist *f)
{
	struct stats_wrapper sw;
	struct linehandler handler;
	int id; // ignore this
	struct team *team;
	struct game *game;

	if (f->num_fields != total_fields_stats) {
		cfbstats_errno = CFBSTATS_EINVALIDFILE;
		return CFBSTATS_ERROR;
	}

	if (f->line == 1) {
		return check_csv_header(f, fdesc_stats);
	}

	handler.descriptions = fdesc_stats;
	handler.flist = f;
	handler.obj = &sw;

	/* parse the fields */
	if (linehandler_parse(&handler, &id) != CFBSTATS_OK)
		return CFBSTATS_ERROR;

	if ((game = objectdb_get_game(&sw.game_oid)) == NULL) {
		cfbstats_errno = CFBSTATS_EOIDLOOKUP;
		return CFBSTATS_ERROR;
	}

	if (objectid_compare(&game->home_oid, &sw.team_oid)) {
		game->home_stats = sw.stats;
	} else {
		game->away_stats = sw.stats;
	}

	if ((team = objectdb_get_team(&sw.team_oid)) == NULL) {
		cfbstats_errno = CFBSTATS_EOIDLOOKUP;
		return CFBSTATS_ERROR;
	}

	update_team_stats(team, &sw.stats);

	return CFBSTATS_OK;
}

const char *cfbstats_strerror(void)
{
	return cfbstats_errors[cfbstats_errno];
}
