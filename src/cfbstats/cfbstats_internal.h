#ifndef CFBSTATS_INTERNAL_H
#define CFBSTATS_INTERNAL_H

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

#endif
