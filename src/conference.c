
#include <stddef.h>

#include <predcfb/conference.h>

struct conference conferences[CONFERENCE_NUM_MAX];
int num_conferences = 0;

struct conference *conference_create(void)
{
	struct conference *conf;

	if (num_conferences >= CONFERENCE_NUM_MAX)
		return NULL;

	conf = &conferences[num_conferences];
	num_conferences++;

	return conf;
}
