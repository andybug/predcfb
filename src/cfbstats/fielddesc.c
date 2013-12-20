
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

/* constants */
#define NUM_FDESC(a) ((sizeof(a) / sizeof(*a)) - 1)

const int num_fdesc_conference = NUM_FDESC(fdesc_conference);
const int total_fields_conference = 3;

const int num_fdesc_team = NUM_FDESC(fdesc_team);
const int total_fields_team = 3;

const int num_fdesc_game = NUM_FDESC(fdesc_game);
const int total_fields_game = 6;

