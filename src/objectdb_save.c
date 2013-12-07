
#include <stdio.h>

#include <yaml.h>

#include <predcfb/predcfb.h>
#include <predcfb/options.h>
#include <predcfb/objectdb.h>

#include "objectdb_internal.h"

int objectdb_save_yaml(const struct object *objects, int num_objects)
{
	puts("hi mom!");
	printf("writing %d objects...\n", num_objects);
	return OBJECTDB_OK;
}
