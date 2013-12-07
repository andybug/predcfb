
#include <stdio.h>
#include <errno.h>
#include <string.h>

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

static int begin_yaml(struct save_context *ctx)
{
	int err;

	yaml_emitter_initialize(&ctx->emitter);
	yaml_emitter_set_output_file(&ctx->emitter, ctx->outf);

	yaml_stream_start_event_initialize(&ctx->event, YAML_UTF8_ENCODING);
	if (!yaml_emitter_emit(&ctx->emitter, &ctx->event))
		return OBJECTDB_ERROR;

	err = yaml_document_start_event_initialize(&ctx->event,
			NULL, NULL, NULL, 0);
	if (err == 0)
		return OBJECTDB_ERROR;

	if (!yaml_emitter_emit(&ctx->emitter, &ctx->event))
		return OBJECTDB_ERROR;

	return OBJECTDB_OK;
}

static int end_yaml(struct save_context *ctx)
{
	if (!yaml_document_end_event_initialize(&ctx->event, 0))
		return OBJECTDB_ERROR;

	if (!yaml_emitter_emit(&ctx->emitter, &ctx->event))
		return OBJECTDB_ERROR;

	yaml_stream_end_event_initialize(&ctx->event);
	if (!yaml_emitter_emit(&ctx->emitter, &ctx->event))
		return OBJECTDB_ERROR;

	return OBJECTDB_OK;
}

static int emit_objects(struct save_context *ctx)
{
	int i;
	int err;

	err = yaml_mapping_start_event_initialize(&ctx->event, NULL, (unsigned char*)"tag:objects:map", 0, YAML_BLOCK_MAPPING_STYLE);
	if (err == 0)
		return OBJECTDB_ERROR;

	if (!yaml_emitter_emit(&ctx->emitter, &ctx->event))
		return OBJECTDB_ERROR;

	for (i = 0; i < ctx->num_objects; i++) {
		printf("%04d %d\n", i, (int)ctx->objects[i].type);
	}

	if (!yaml_mapping_end_event_initialize(&ctx->event))
		return OBJECTDB_ERROR;

	if (!yaml_emitter_emit(&ctx->emitter, &ctx->event))
		return OBJECTDB_ERROR;

	return OBJECTDB_OK;
}

int objectdb_save_yaml(const struct object *objects, int num_objects)
{
	struct save_context ctx;
	int err = OBJECTDB_ERROR;

	memset(&ctx, 0, sizeof(ctx));

	ctx.objects = objects;
	ctx.num_objects = num_objects;

	if (open_file(&ctx) != OBJECTDB_OK)
		return OBJECTDB_ERROR;

	if (begin_yaml(&ctx) != OBJECTDB_OK)
		goto cleanup;

	if (emit_objects(&ctx) != OBJECTDB_OK)
		goto cleanup;

	if (end_yaml(&ctx) != OBJECTDB_OK)
		goto cleanup;

	err = OBJECTDB_OK;
	printf("%s: wrote %d objects to %s\n", progname, num_objects,
	                                       opt_save_file);
cleanup:
	yaml_emitter_delete(&ctx.emitter);
	fclose(ctx.outf);

	return err;
}
