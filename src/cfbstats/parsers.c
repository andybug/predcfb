
#include <string.h>

#include <predcfb/cfbstats.h>
#include <predcfb/objectdb.h>

#include "cfbstats_internal.h"

/* csv header verification */

static int check_csv_header(
		struct csvline *c,
		const struct fielddesc *desc_list)
{
	const char *field;
	const struct fielddesc *desc = desc_list;

	while (desc->type != FIELD_TYPE_END) {
		if (csvline_str_at(c, desc->index, &field) != CSVP_OK)
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

int parse_conference_csv(struct csvline *c)
{
	struct linehandler handler;
	struct conference *conf;
	struct objectid oid;
	int id;

	if (c->num_fields != total_fields_conference) {
		cfbstats_errno = CFBSTATS_EINVALIDFILE;
		return CFBSTATS_ERROR;
	}

	if (c->line == 1) {
		return check_csv_header(c, fdesc_conference);
	}

	conf = objectdb_create_conference();
	if (!conf) {
		/* too many conferences!
		 * TODO: print error string
		 */
		return CFBSTATS_ERROR;
	}

	handler.descriptions = fdesc_conference;
	handler.csvline = c;
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

int parse_team_csv(struct csvline *c)
{
	struct linehandler handler;
	struct objectid oid;
	int id;
	struct team *team;

	if (c->num_fields != total_fields_team) {
		cfbstats_errno = CFBSTATS_EINVALIDFILE;
		return CFBSTATS_ERROR;
	}

	if (c->line == 1) {
		return check_csv_header(c, fdesc_team);
	}

	if ((team = objectdb_create_team()) == NULL) {
		cfbstats_errno = CFBSTATS_ETOOMANY;
		return CFBSTATS_ERROR;
	}

	handler.descriptions = fdesc_team;
	handler.csvline = c;
	handler.obj = team;

	/* parse the fields */
	if (linehandler_parse(&handler, &id) != CFBSTATS_OK)
		return CFBSTATS_ERROR;

	/* set the conference pointer from the oid */
	if ((team->conf = objectdb_get_conference(&team->conf_oid)) == NULL) {
		cfbstats_errno = CFBSTATS_EOIDLOOKUP;
		return CFBSTATS_ERROR;
	}

	/* add the team to the object db */
	if (objectdb_add_team(team, &oid) != OBJECTDB_OK)
		return CFBSTATS_ERROR;

	/* add the team to the id map */
	if (id_map_insert(id, &oid) != CFBSTATS_OK)
		return CFBSTATS_ERROR;

	return CFBSTATS_OK;
}

/* parse game.csv */

int parse_game_csv(struct csvline *c)
{
	struct linehandler handler;
	struct objectid oid;
	int id;
	struct game *game;

	if (c->num_fields != total_fields_game) {
		cfbstats_errno = CFBSTATS_EINVALIDFILE;
		return CFBSTATS_ERROR;
	}

	if (c->line == 1) {
		return check_csv_header(c, fdesc_game);
	}

	if ((game = objectdb_create_game()) == NULL) {
		cfbstats_errno = CFBSTATS_ETOOMANY;
		return CFBSTATS_ERROR;
	}

	handler.descriptions = fdesc_game;
	handler.csvline = c;
	handler.obj = game;

	/* parse the fields */
	if (linehandler_parse(&handler, &id) != CFBSTATS_OK)
		return CFBSTATS_ERROR;

	/* set home and away team pointers */
	if ((game->home = objectdb_get_team(&game->home_oid)) == NULL) {
		cfbstats_errno = CFBSTATS_EOIDLOOKUP;
		return CFBSTATS_ERROR;
	}

	if ((game->away = objectdb_get_team(&game->away_oid)) == NULL) {
		cfbstats_errno = CFBSTATS_EOIDLOOKUP;
		return CFBSTATS_ERROR;
	}

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

int parse_stats_csv(struct csvline *c)
{
	struct stats_wrapper sw;
	struct linehandler handler;
	int id; // ignore this
	struct team *team;
	struct game *game;

	if (c->num_fields != total_fields_stats) {
		cfbstats_errno = CFBSTATS_EINVALIDFILE;
		return CFBSTATS_ERROR;
	}

	if (c->line == 1) {
		return check_csv_header(c, fdesc_stats);
	}

	handler.descriptions = fdesc_stats;
	handler.csvline = c;
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
