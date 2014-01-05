
#include <stdio.h>
#include <time.h>

#include <predcfb/objectdb.h>

/*
 * scan for the next Tuesday, so that games that aren't on
 * Saturday will get picked up ok
 */
void sched_find_week_end(const time_t *start, time_t *end)
{
	static const int TUESDAY = 2;	/* Sun = 0, Mon = 1, ..., Sat = 6 */
	static const int WEEK_BEGIN = TUESDAY;
	struct tm tm;

	gmtime_r(start, &tm);

	if (tm.tm_wday < WEEK_BEGIN) {
		tm.tm_mday += WEEK_BEGIN - tm.tm_wday;
	} else {
		tm.tm_mday += (7 - tm.tm_wday) + WEEK_BEGIN;
	}

	*end = mktime(&tm);
}
