#ifndef CFBSTATS_INTERNAL_H
#define CFBSTATS_INTERNAL_H

extern void cfbstats_init(void);

/* prototypes for handling a line from each file */
extern int parse_conference_csv(struct fieldlist *);
extern int parse_team_csv(struct fieldlist *);
extern int parse_game_csv(struct fieldlist *);
extern int parse_stats_csv(struct fieldlist *);

#endif
