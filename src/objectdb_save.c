
#include <stdio.h>
#include <errno.h>

#include <yaml.h>

#include <predcfb/predcfb.h>
#include <predcfb/options.h>
#include <predcfb/objectdb.h>

#include "objectdb_internal.h"

struct save_context {
	FILE *outf;
	yaml_emitter_t emitter;
	yaml_event_t event;

	const struct object *objects;
	int num_objects;
};

static int open_file(struct save_context *ctx)
{
	ctx->outf = fopen(opt_save_file, "w");
	if (!ctx->outf) {
		fprintf(stderr, "%s: could not open '%s' for writing\n",
		        progname, opt_save_file);
		return OBJECTDB_ERROR;
	}

	return OBJECTDB_OK;
}

int objectdb_save_yaml(const struct object *objects, int num_objects)
{
	struct save_context ctx = { 0 };
	int err = OBJECTDB_OK;

	if (!open_file(&ctx))
		return OBJECTDB_ERROR;

	yaml_emitter_initialize(&emitter);
	yaml_emitter_set_output_file(&emitter, outf);

	yaml_stream_start_event_initialize(&event, YAML_UTF8_ENCODING);
	if (!yaml_emitter_emit(&emitter, &event))
		goto cleanup;

	if (!yaml_document_start_event_initialize(&event, NULL, NULL, NULL, 0))
		goto cleanup;

	if (!yaml_emitter_emit(&emitter, &event))
		goto cleanup;



	if (!yaml_sequence_start_event_initialize(&event, NULL, "objects", 0, YAML_ANY_SEQUENCE_STYLE))
		goto cleanup;

	if (!yaml_emitter_emit(&emitter, &event))
		goto cleanup;

	if (!yaml_sequence_end_event_initialize(&event))
		goto cleanup;

	if (!yaml_emitter_emit(&emitter, &event))
		goto cleanup;



	if (!yaml_document_end_event_initialize(&event, 0))
		goto cleanup;

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
