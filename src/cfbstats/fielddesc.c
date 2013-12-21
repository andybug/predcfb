
#include <limits.h>

#include <predcfb/predcfb.h>

#include "cfbstats_internal.h"

/* conference.csv */
const struct fielddesc fdesc_conference[] = {
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

/* team.csv */
const struct fielddesc fdesc_team[] = {
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

/* game.csv */
const struct fielddesc fdesc_game[] = {
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

#define WRAPPER_OFFSET(a) (offsetof(struct stats_wrapper, a))
#define STATS_OFFSET(a) (WRAPPER_OFFSET(stats) + \
		offsetof(struct stats, a))

const struct fielddesc fdesc_stats[] = {
	{
		.index = 0,
		.name = "Team Code",
		.type = FIELD_TYPE_TEAMID,
		.len = 0,
		.offset = WRAPPER_OFFSET(team_oid),
	},
	{
		.index = 1,
		.name = "Game Code",
		.type = FIELD_TYPE_GAMEID,
		.len = 0,
		.offset = WRAPPER_OFFSET(game_oid),
	},
	{
		.index = 2,
		.name = "Rush Att",
		.type = FIELD_TYPE_SHORT,
		.len = 0,
		.offset = STATS_OFFSET(rush_att),
	},
	{
		.index = 3,
		.name = "Rush Yard",
		.type = FIELD_TYPE_SHORT,
		.len = 0,
		.offset = STATS_OFFSET(rush_yds),
	},
	{
		.index = 4,
		.name = "Rush TD",
		.type = FIELD_TYPE_SHORT,
		.len = 0,
		.offset = STATS_OFFSET(rush_tds),
	},
	{
		.index = 5,
		.name = "Pass Att",
		.type = FIELD_TYPE_SHORT,
		.len = 0,
		.offset = STATS_OFFSET(pass_att),
	},
	{
		.index = 6,
		.name = "Pass Comp",
		.type = FIELD_TYPE_SHORT,
		.len = 0,
		.offset = STATS_OFFSET(pass_comp),
	},
	{
		.index = 7,
		.name = "Pass Yard",
		.type = FIELD_TYPE_SHORT,
		.len = 0,
		.offset = STATS_OFFSET(pass_yds),
	},
	{
		.index = 8,
		.name = "Pass TD",
		.type = FIELD_TYPE_SHORT,
		.len = 0,
		.offset = STATS_OFFSET(pass_tds),
	},
	{
		.index = 9,
		.name = "Pass Int",
		.type = FIELD_TYPE_SHORT,
		.len = 0,
		.offset = STATS_OFFSET(pass_int),
	},
	{
		.index = 35,
		.name = "Points",
		.type = FIELD_TYPE_SHORT,
		.len = 0,
		.offset = STATS_OFFSET(points),
	},
	{
		.index = 43,
		.name = "Fumble",
		.type = FIELD_TYPE_SHORT,
		.len = 0,
		.offset = STATS_OFFSET(fumbles),
	},
	{
		.index = 44,
		.name = "Fumble Lost",
		.type = FIELD_TYPE_SHORT,
		.len = 0,
		.offset = STATS_OFFSET(fumbles_lost),
	},
#if 0
	{
		.index = 49,
		.name = "Sack",
		.type = FIELD_TYPE_SHORT,
		.len = 0,
		.offset = STATS_OFFSET(sacks),
	},
	{
		.index = 51,
		.name = "QB Hurry",
		.type = FIELD_TYPE_SHORT,
		.len = 0,
		.offset = STATS_OFFSET(hurries),
	},
	{
		.index = 52,
		.name = "Fumble Forced",
		.type = FIELD_TYPE_SHORT,
		.len = 0,
		.offset = STATS_OFFSET(fumbles_forced),
	},
	{
		.index = 53,
		.name = "Pass Broken Up",
		.type = FIELD_TYPE_SHORT,
		.len = 0,
		.offset = STATS_OFFSET(pass_breakups),
	},
#endif
	{
		.index = INT_MIN,
		.name = NULL,
		.type = FIELD_TYPE_END,
		.len = 0,
		.offset = 0
	}
};

/* constants */
#define NUM_FDESC(a) ((sizeof(a) / sizeof(*a)) - 1)

const int num_fdesc_conference = NUM_FDESC(fdesc_conference);
const int total_fields_conference = 3;

const int num_fdesc_team = NUM_FDESC(fdesc_team);
const int total_fields_team = 3;

const int num_fdesc_game = NUM_FDESC(fdesc_game);
const int total_fields_game = 6;

const int num_fdesc_stats = NUM_FDESC(fdesc_stats);
const int total_fields_stats = 68;

