
#include <stdlib.h>
#include <errno.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <time.h>

#include <predcfb/cfbstats.h>
#include <predcfb/predcfb.h>
#include <predcfb/objectdb.h>
#include <predcfb/zipfile.h>
#include <predcfb/csvparse.h>

#define CFBSTATS_ID_MAP_SIZE 4096

extern const char *progname;

struct map_entry {
	int id;
	struct objectid oid;
};

enum file_type {
	CFBSTATS_FILE_NONE,
	CFBSTATS_FILE_CSV
};

struct file_handler {
	const char *file;
	enum file_type type;
	int (*parsing_func)(struct fieldlist *);
};

/* prototypes for the handlers */
static int parse_conference_csv(struct fieldlist *);
static int parse_team_csv(struct fieldlist *);
static int parse_game_csv(struct fieldlist *);

static const struct file_handler file_handlers[] = {
	{ "conference.csv", CFBSTATS_FILE_CSV, parse_conference_csv },
	{ "team.csv",       CFBSTATS_FILE_CSV, parse_team_csv },
	{ "game.csv",       CFBSTATS_FILE_CSV, parse_game_csv },
	{ NULL, CFBSTATS_FILE_NONE, NULL }
};

static struct map_entry id_map[CFBSTATS_ID_MAP_SIZE];

/* subtract 1 from total to account for ending null struct */
static const int num_file_handlers = (sizeof(file_handlers) /
                                      sizeof(struct file_handler)) - 1;

enum cfbstats_err cfbstats_errno = CFBSTATS_ENONE;
static const char *cfbstats_errors[] = {
	"No error",
	"Error parsing zip file",
	"Memory allocation failed",
	"File does not match expected format",
	"Too many records",
	"Failed cfbstats id lookup",
	"Failed objectid lookup"
};

/* initialization functions */

static void cfbstats_init(void)
{
	/* make sure that the id map size is a power of 2 */
	assert(!(CFBSTATS_ID_MAP_SIZE & (CFBSTATS_ID_MAP_SIZE - 1)));

	memset(id_map, 0, sizeof(id_map));
}

/* cfbstats id to objectid map */

static int id_map_insert(int id, const struct objectid *oid)
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

static const struct objectid *id_map_lookup(int id)
{
	static const int mask = CFBSTATS_ID_MAP_SIZE - 1;
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

/* csv header verification */

static int check_csv_header(struct fieldlist *f, const char **fields, int num)
{
	int i;
	const char *field;

	assert(f->num_fields == num);

	fieldlist_iter_begin(f);

	for (i = 0; i < num; i++) {
		field = fieldlist_iter_next(f);
		if (strcmp(field, fields[i]) != 0) {
			cfbstats_errno = CFBSTATS_EINVALIDFILE;
			return CFBSTATS_ERROR;
		}
	}

	return CFBSTATS_OK;
}

#define NUM_FIELDS(a) (sizeof(a) / sizeof(*a))

/* parse conference.csv */

static int parse_conference_csv(struct fieldlist *f)
{
	static const char *fields[] = {
		"Conference Code",
		"Name",
		"Subdivision"
	};
	static const int NUM_CONFERENCE_FIELDS = NUM_FIELDS(fields);
	static bool processed_header = false;

	struct conference *conf;
	const char *str;
	size_t len;
	int id;
	struct objectid oid;

	if (f->num_fields != NUM_CONFERENCE_FIELDS) {
		cfbstats_errno = CFBSTATS_EINVALIDFILE;
		return CFBSTATS_ERROR;
	}

	if (!processed_header) {
		processed_header = true;
		return check_csv_header(f, fields, NUM_CONFERENCE_FIELDS);
	}

	conf = objectdb_create_conference();
	if (!conf) {
		/* too many conferences!
		 * TODO: print error string
		 */
		return CFBSTATS_ERROR;
	}

	fieldlist_iter_begin(f);

	/* id field */
	if (fieldlist_iter_next_int(f, &id) != FIELDLIST_OK)
		return CFBSTATS_ERROR;

	/* conference name */
	str = fieldlist_iter_next(f);
	len = strlen(str);
	if (len >= CONFERENCE_NAME_MAX)
		return CFBSTATS_ERROR;
	strncpy(conf->name, str, CONFERENCE_NAME_MAX);

	/* subdivision */
	str = fieldlist_iter_next(f);
	if (strcmp("FBS", str) == 0)
		conf->subdivision = CONFERENCE_FBS;
	else if (strcmp("FCS", str) == 0)
		conf->subdivision = CONFERENCE_FCS;
	else
		return CFBSTATS_ERROR;

	/* add the conference to the objectdb */
	if (objectdb_add_conference(conf, &oid) != OBJECTDB_OK)
		return CFBSTATS_ERROR;

	/* add the conference to the id map */
	if (id_map_insert(id, &oid) != CFBSTATS_OK)
		return CFBSTATS_ERROR;

	return CFBSTATS_OK;
}

/* parse team.csv */

static int parse_team_csv(struct fieldlist *f)
{
	static const char *fields[] = {
		"Team Code",
		"Name",
		"Conference Code"
	};
	static const int NUM_TEAM_FIELDS = NUM_FIELDS(fields);
	static bool processed_header = false;

	const char *str;
	size_t len;
	int id;
	int conf_id;
	struct objectid oid;
	const struct objectid *conf_oid;
	struct team *team;

	if (f->num_fields != NUM_TEAM_FIELDS) {
		cfbstats_errno = CFBSTATS_EINVALIDFILE;
		return CFBSTATS_ERROR;
	}

	if (!processed_header) {
		processed_header = true;
		return check_csv_header(f, fields, NUM_TEAM_FIELDS);
	}

	if ((team = objectdb_create_team()) == NULL) {
		cfbstats_errno = CFBSTATS_ETOOMANY;
		return CFBSTATS_ERROR;
	}

	fieldlist_iter_begin(f);

	/* id field */
	if (fieldlist_iter_next_int(f, &id) != FIELDLIST_OK)
		return CFBSTATS_ERROR;

	/* team name */
	str = fieldlist_iter_next(f);
	len = strlen(str);
	if (len >= TEAM_NAME_MAX)
		return CFBSTATS_ERROR;
	strncpy(team->name, str, TEAM_NAME_MAX);

	/* conf id */
	if (fieldlist_iter_next_int(f, &conf_id) != FIELDLIST_OK)
		return CFBSTATS_ERROR;

	/* get the conf oid from the id */
	if ((conf_oid = id_map_lookup(conf_id)) == NULL) {
		cfbstats_errno = CFBSTATS_EIDLOOKUP;
		return CFBSTATS_ERROR;
	}
	team->conf_oid = *conf_oid;

	/* add the team to the object db */
	if (objectdb_add_team(team, &oid) != OBJECTDB_OK)
		return CFBSTATS_ERROR;

	/* add team to id map */
	if (id_map_insert(id, &oid) != OBJECTDB_OK)
		return CFBSTATS_ERROR;

	return CFBSTATS_OK;
}

/* parse game.csv */

static int parse_game_csv(struct fieldlist *f)
{
	static const char *fields[] = {
		"Game Code",
		"Date",
		"Visit Team Code",
		"Home Team Code",
		"Stadium Code",
		"Site"
	};
	static const int NUM_GAME_FIELDS = NUM_FIELDS(fields);
	/* dates are MM/DD/YYYY HH:MM:SS -ZZZZ */
	static const int DATE_BUF_SIZE = 26;
	static bool processed_header = false;

	struct game *game;
	struct tm tm;
	const char *str, *lastchar;
	int id;
	const struct objectid *oid;
	struct objectid game_oid;
	char date_buf[DATE_BUF_SIZE];

	if (f->num_fields != NUM_GAME_FIELDS) {
		cfbstats_errno = CFBSTATS_EINVALIDFILE;
		return CFBSTATS_ERROR;
	}

	if (!processed_header) {
		processed_header = true;
		return check_csv_header(f, fields, NUM_GAME_FIELDS);
	}

	if ((game = objectdb_create_game()) == NULL) {
		cfbstats_errno = CFBSTATS_ETOOMANY;
		return CFBSTATS_ERROR;
	}

	fieldlist_iter_begin(f);

	/* ignore the game code */
	str = fieldlist_iter_next(f);

	/* date in the format 08/29/2013 */
	str = fieldlist_iter_next(f);
	snprintf(date_buf, DATE_BUF_SIZE, "%s 00:00:00 +0000", str);
	lastchar = strptime(date_buf, "%m/%d/%Y %H:%M:%S %z", &tm);
	if (!lastchar || *lastchar != '\0') {
		cfbstats_errno = CFBSTATS_EINVALIDFILE;
		return CFBSTATS_ERROR;
	}
	game->date = mktime(&tm);

	/* visiting team */
	if (fieldlist_iter_next_int(f, &id) != FIELDLIST_OK) {
		cfbstats_errno = CFBSTATS_EINVALIDFILE;
		return CFBSTATS_ERROR;
	}

	/* get the away team's oid from the id */
	if ((oid = id_map_lookup(id)) == NULL) {
		cfbstats_errno = CFBSTATS_EIDLOOKUP;
		return CFBSTATS_ERROR;
	}
	game->away_oid = *oid;

	/* home team */
	if (fieldlist_iter_next_int(f, &id) != FIELDLIST_OK) {
		cfbstats_errno = CFBSTATS_EINVALIDFILE;
		return CFBSTATS_ERROR;
	}

	/* get the home team's oid from the id */
	if ((oid = id_map_lookup(id)) == NULL) {
		cfbstats_errno = CFBSTATS_EIDLOOKUP;
		return CFBSTATS_ERROR;
	}
	game->home_oid = *oid;

	/* ignore stadium code */
	str = fieldlist_iter_next(f);

	/* site */
	str = fieldlist_iter_next(f);
	if (strcmp(str, "TEAM") == 0) {
		game->neutral = false;
	} else if (strcmp(str, "NEUTRAL") == 0) {
		game->neutral = true;
	} else {
		cfbstats_errno = CFBSTATS_EINVALIDFILE;
		return CFBSTATS_ERROR;
	}

	/* finally, add the game to the db */
	if (objectdb_add_game(game, &game_oid) != OBJECTDB_OK)
		return CFBSTATS_ERROR;

	return CFBSTATS_OK;
}

/* zipfile parsing */

static void handle_zipfile_error(zf_readctx *zf, const char *func)
{
	const char *err;

	err = zipfile_strerr(zf);
	fprintf(stderr, "%s: %s\n", func, err);

	cfbstats_errno = CFBSTATS_EZIPFILE;
}

static int read_csv_file_from_zipfile(zf_readctx *zf,
                                      const struct file_handler *handler)
{
	static const int CSV_BUF_SIZE = 4096;
	char buf[CSV_BUF_SIZE];
	ssize_t bytes;
	csvp_ctx *csvp;

	if (zipfile_open_file(zf, handler->file) != ZIPFILE_OK) {
		handle_zipfile_error(zf, __func__);
		return CFBSTATS_ERROR;
	}

	csvp = csvp_create(handler->parsing_func);
	if (!csvp) {
		fprintf(stderr, "%s: csvp initialization error\n", progname);
		return CFBSTATS_ERROR;
	}

	while ((bytes = zipfile_read_file(zf, buf, CSV_BUF_SIZE))) {
		if (bytes == ZIPFILE_ERROR)
			return CFBSTATS_ERROR;

		if (bytes == 0) /* eof */
			break;

		if (csvp_parse(csvp, buf, bytes) != CSVP_OK) {
			const char *err;

			if (csvp_error(csvp) == CSVP_EPARSE) {
				/*
				 * if it is a parsing error, then check our
				 * own error code since the error originated
				 * here
				 */
				err = cfbstats_strerror();
				fprintf(stderr, "%s: %s\n", progname, err);
			}

			err = csvp_strerror(csvp);
			fprintf(stderr, "%s: %s in %s\n",
			        progname, err, handler->file);

			return CFBSTATS_ERROR;
		}
	}

	if (csvp_destroy(csvp) != CSVP_OK) {
		const char *err = csvp_strerror(csvp);
		fprintf(stderr, "%s: %s in %s\n",
		        progname, err, handler->file);
		return CFBSTATS_ERROR;
	}

	if (zipfile_close_file(zf) != ZIPFILE_OK) {
		handle_zipfile_error(zf, __func__);
		return CFBSTATS_ERROR;
	}

	return CFBSTATS_OK;
}

static int read_files_from_zipfile(zf_readctx *zf)
{
	const struct file_handler *handler = file_handlers;

	while (handler->type != CFBSTATS_FILE_NONE) {
		switch (handler->type) {
		case CFBSTATS_FILE_CSV:
			if (read_csv_file_from_zipfile(zf, handler) != CFBSTATS_OK)
				return CFBSTATS_ERROR;
			break;

		case CFBSTATS_FILE_NONE:
		default:
			/* this can't happen... */
			break;
		}

		handler++;
	}

	return CFBSTATS_OK;
}

/* global functions */

int cfbstats_read_zipfile(const char *path)
{
	zf_readctx *zf;

	cfbstats_init();

	zf = zipfile_open_archive(path);
	if (!zf) {
		cfbstats_errno = CFBSTATS_ENOMEM;
		return CFBSTATS_ERROR;
	} else if (zipfile_get_error(zf) != ZIPFILE_ENONE) {
		handle_zipfile_error(zf, __func__);
		return CFBSTATS_ERROR;
	}

	if (read_files_from_zipfile(zf) != ZIPFILE_OK)
		return CFBSTATS_ERROR;

	if (zipfile_close_archive(zf) != ZIPFILE_OK) {
		handle_zipfile_error(zf, __func__);
		return CFBSTATS_ERROR;
	}

	/*
	 * finish up by setting all of the pointers in the data
	 * from the object ids
	 */
	if (objectdb_link() != OBJECTDB_OK)
		return CFBSTATS_ERROR;

	return CFBSTATS_OK;
}

const char *cfbstats_strerror(void)
{
	return cfbstats_errors[cfbstats_errno];
}
