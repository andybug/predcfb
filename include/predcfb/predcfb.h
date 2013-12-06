#ifndef PREDCFB_H
#define PREDCFB_H

#include <stdbool.h>
#include <time.h>

#include <predcfb/objectid.h>

#define CONFERENCE_NAME_MAX 64
#define CONFERENCE_NUM_MAX  32

enum conference_division {
	CONFERENCE_FBS,
	CONFERENCE_FCS
};

struct conference {
	char name[CONFERENCE_NAME_MAX];
	enum conference_division subdivision;
};

struct stats {
	short rush_att;
	short rush_yds;
	short rush_tds;

	short pass_att;
	short pass_comp;
	short pass_yds;
	short pass_tds;
	short pass_int;

	short fg_att;
	short fg_made;

	short fumbles;
	short fumbles_lost;

	short penalties;
	short penalty_yds;

	short points;
};

#define TEAM_NAME_MAX   64
#define TEAM_NUM_MAX   256

struct team {
	char name[TEAM_NAME_MAX];
	struct objectid conf_oid;
	struct conference *conf;
	struct stats stats;
};

#define GAME_NUM_MAX 2048

struct game {
	struct objectid home_oid;
	struct team *home;
	struct objectid away_oid;
	struct team *away;
	bool neutral;
	time_t date;
	struct stats home_stats;
	struct stats away_stats;
};

#endif
