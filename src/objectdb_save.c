
#include <stdio.h>

#include <yaml.h>

#include <predcfb/predcfb.h>
#include <predcfb/options.h>
#include <predcfb/objectdb.h>

#include "objectdb_internal.h"

int objectdb_save_yaml(const struct object *objects, int num_objects)
{
	FILE *outf;
	int err = OBJECTDB_OK;
	yaml_emitter_t emitter;
	yaml_event_t event;

	outf = fopen(opt_save_file, "w");
	if (!outf)
		return OBJECTDB_ERROR;

	yaml_emitter_initialize(&emitter);
	yaml_emitter_set_output_file(&emitter, outf);

	yaml_stream_start_event_initialize(&event, YAML_UTF8_ENCODING);
	if (!yaml_emitter_emit(&emitter, &event))
		goto cleanup;

	yaml_stream_end_event_initialize(&event);
	if (!yaml_emitter_emit(&emitter, &event))
		goto cleanup;

	printf("%s: wrote %d objects to %s\n", progname, num_objects,
	                                       opt_save_file);
cleanup:
	yaml_emitter_delete(&emitter);
	fclose(outf);

	return err;
}
