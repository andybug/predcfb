#ifndef PREDCFB_H
#define PREDCFB_H

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

#endif
