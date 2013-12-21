#ifndef CFBSTATS_INTERNAL_H
#define CFBSTATS_INTERNAL_H

#include <predcfb/predcfb.h>
#include <predcfb/objectid.h>
#include <predcfb/fieldlist.h>

#define CFBSTATS_ID_MAP_SIZE 4096

extern void cfbstats_init(void);

/* id_map functions */
extern void id_map_clear(void);
extern int id_map_insert(int id, const struct objectid *oid);
extern const struct objectid *id_map_lookup(int id);
extern int pack_game_code(const char *str);

/* prototypes for handling a line from each file */
extern int parse_conference_csv(struct fieldlist *);
extern int parse_team_csv(struct fieldlist *);
extern int parse_game_csv(struct fieldlist *);
extern int parse_stats_csv(struct fieldlist *);

/* field description structure */
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

/* per-file field description lists */
extern const struct fielddesc fdesc_conference[];
extern const int num_fdesc_conference;
extern const int total_fields_conference;

extern const struct fielddesc fdesc_team[];
extern const int num_fdesc_team;
extern const int total_fields_team;

extern const struct fielddesc fdesc_game[];
extern const int num_fdesc_game;
extern const int total_fields_game;

extern const struct fielddesc fdesc_stats[];
extern const int num_fdesc_stats;
extern const int total_fields_stats;

/* linehandler */
struct linehandler {
	const struct fielddesc *descriptions;
	const struct fielddesc *current;
	struct fieldlist *flist;
	void *obj;
};

extern int linehandler_parse(struct linehandler *lh, int *id);

/*
 * struct that will contain the stats, team, and game ids
 * for use with the linehandling implementation
 */
struct stats_wrapper {
	struct objectid team_oid;
	struct objectid game_oid;
	struct stats stats;
};

#endif
