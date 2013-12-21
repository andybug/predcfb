
#include <assert.h>

#include <predcfb/cfbstats.h>

#include "cfbstats_internal.h"

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

const char *cfbstats_strerror(void)
{
	return cfbstats_errors[cfbstats_errno];
}

void cfbstats_init(void)
{
	/* make sure that the id map size is a power of 2 */
	assert(!(CFBSTATS_ID_MAP_SIZE & (CFBSTATS_ID_MAP_SIZE - 1)));

	assert(num_fdesc_conference <= total_fields_conference);
	assert(num_fdesc_team <= total_fields_team);
	assert(num_fdesc_game <= total_fields_game);

	id_map_clear();
}
