#ifndef CONFERENCE_H
#define CONFERENCE_H

#define CONFERENCE_NAME_MAX 64
#define CONFERENCE_NUM_MAX 32

enum conference_division {
	CONFERENCE_FBS,
	CONFERENCE_FCS
};

struct conference {
	int id;
	char name[CONFERENCE_NAME_MAX];
	enum conference_division div;
};

extern struct conference conferences[CONFERENCE_NUM_MAX];
extern int num_conferences;

extern struct conference *conference_create(void);

#endif
