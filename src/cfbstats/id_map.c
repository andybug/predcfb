
#include <stdlib.h>
#include <string.h>

#include <predcfb/cfbstats.h>
#include <predcfb/objectid.h>

#include "cfbstats_internal.h"

struct map_entry {
	int id;
	struct objectid oid;
};

static struct map_entry id_map[CFBSTATS_ID_MAP_SIZE];

void id_map_clear(void)
{
	memset(id_map, 0, sizeof(id_map));
}

int id_map_insert(int id, const struct objectid *oid)
{
	static const int mask = CFBSTATS_ID_MAP_SIZE - 1;
	struct map_entry *entry;
	int i;
	int count = 0;

	i = id & mask;

	while (count < CFBSTATS_ID_MAP_SIZE) {
		entry = &id_map[i];
		if (entry->id == 0) {
			entry->id = id;
			entry->oid = *oid;
			break;
		}

		i = (i + 1) & mask;
		count++;
	}

	if (count == CFBSTATS_ID_MAP_SIZE)
		return CFBSTATS_ERROR;

	return CFBSTATS_OK;
}

const struct objectid *id_map_lookup(int id)
{
	const int mask = CFBSTATS_ID_MAP_SIZE - 1;
	struct map_entry *entry;
	int i;
	int count = 0;

	i = id & mask;

	while (count < CFBSTATS_ID_MAP_SIZE) {
		entry = &id_map[i];
		if (entry->id == id)
			return &entry->oid;

		i = (i + 1) & mask;
		count++;
	}

	return NULL;
}

int pack_game_code(const char *str)
{
	int id;
	const char *team1, *team2, *date, *mmdd;
	const size_t team_str_len = 4;
	const size_t date_str_len = 8;
	const size_t mmdd_str_len = 4;
	const size_t buf_size = date_str_len + 1;
	char buf[buf_size];
	int code;

	team1 = str;
	team2 = (str + team_str_len);
	date = (str + (team_str_len * 2));
	mmdd = date + 4;

	/* team1 shifted into bits 16-31 */
	memcpy(buf, team1, team_str_len);
	buf[team_str_len] = '\0';
	code = atoi(buf);
	id = (code << 16) & 0xffff0000;

	/* team2 xor'd into bits 16-31 */
	memcpy(buf, team2, team_str_len);
	buf[team_str_len] = '\0';
	code = atoi(buf);
	id ^= (code << 16) & 0xffff0000;

	/* month and day (mmdd) put into bits 0-15 */
	memcpy(buf, mmdd, mmdd_str_len);
	buf[mmdd_str_len] = '\0';
	code = atoi(buf);
	id |= code & 0x0000ffff;

	return id;
}

