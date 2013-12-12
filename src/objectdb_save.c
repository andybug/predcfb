
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <time.h>

#include <yaml.h>

#include <predcfb/predcfb.h>
#include <predcfb/options.h>
#include <predcfb/objectdb.h>
#include <predcfb/objectid.h>

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

static int emit_scalar(struct save_context *ctx,
                       const char *name, const char *val)
{
	size_t len;
	int err;

	/* emit name portion of scalar */
	len = strlen(name);
	err = yaml_scalar_event_initialize(
			&ctx->event,
			NULL,
			NULL,
			(unsigned char*)name,
			(int)len,
			1,
			0,
			YAML_PLAIN_SCALAR_STYLE);
	if (err == 0)
		return OBJECTDB_ERROR;

	if (!yaml_emitter_emit(&ctx->emitter, &ctx->event))
		return OBJECTDB_ERROR;

	/* emit value portion of scalar */
	len = strlen(val);
	err = yaml_scalar_event_initialize(
			&ctx->event,
			NULL,
			NULL,
			(unsigned char*)val,
			(int)len,
			1, 0,
			YAML_PLAIN_SCALAR_STYLE);
	if (err == 0)
		return OBJECTDB_ERROR;

	if (!yaml_emitter_emit(&ctx->emitter, &ctx->event))
		return OBJECTDB_ERROR;

	return OBJECTDB_OK;
}

static int emit_scalar_map(struct save_context *ctx, const char *name)
{
	size_t len;
	int err;

	len = strlen(name);

	err = yaml_scalar_event_initialize(
			&ctx->event,
			NULL,
			NULL,
			(unsigned char*)name,
			(int)len,
			1,
			0,
			YAML_PLAIN_SCALAR_STYLE);
	if (err == 0)
		return OBJECTDB_ERROR;

	if (!yaml_emitter_emit(&ctx->emitter, &ctx->event))
		return OBJECTDB_ERROR;

	err = yaml_mapping_start_event_initialize(
			&ctx->event,
			NULL,
			NULL,
			0,
			YAML_BLOCK_MAPPING_STYLE);
	if (err == 0)
		return OBJECTDB_ERROR;

	if (!yaml_emitter_emit(&ctx->emitter, &ctx->event))
		return OBJECTDB_ERROR;

	return OBJECTDB_OK;
}

static int emit_map_begin(struct save_context *ctx)
{
	int err;

	err = yaml_mapping_start_event_initialize(
			&ctx->event,
			NULL,
			NULL,
			0,
			YAML_BLOCK_MAPPING_STYLE);
	if (err == 0)
		return OBJECTDB_ERROR;

	if (!yaml_emitter_emit(&ctx->emitter, &ctx->event))
		return OBJECTDB_ERROR;

	return OBJECTDB_OK;
}

static int emit_map_end(struct save_context *ctx)
{
	if (!yaml_mapping_end_event_initialize(&ctx->event))
		return OBJECTDB_ERROR;

	if (!yaml_emitter_emit(&ctx->emitter, &ctx->event))
		return OBJECTDB_ERROR;

	return OBJECTDB_OK;
}

static int emit_sequence_begin(struct save_context *ctx)
{
	int err;

	err = yaml_sequence_start_event_initialize(
			&ctx->event,
			NULL,
			NULL,
			0,
			YAML_BLOCK_SEQUENCE_STYLE);
	if (err == 0)
		return OBJECTDB_ERROR;

	if (!yaml_emitter_emit(&ctx->emitter, &ctx->event))
		return OBJECTDB_ERROR;

	return OBJECTDB_OK;
}

static int emit_sequence_end(struct save_context *ctx)
{
	if (!yaml_sequence_end_event_initialize(&ctx->event))
		return OBJECTDB_ERROR;

	if (!yaml_emitter_emit(&ctx->emitter, &ctx->event))
		return OBJECTDB_ERROR;

	return OBJECTDB_OK;
}

static int emit_object_type(struct save_context *ctx, const struct object *o)
{
	const char *typestr = "UNKN";

	/* get a string representation of the type enum */
	switch (o->type) {
	case OBJECTDB_CONF:
		typestr = "CONF";
		break;

	case OBJECTDB_TEAM:
		typestr = "TEAM";
		break;

	case OBJECTDB_GAME:
		typestr = "GAME";
		break;

	case OBJECTDB_BLOB:
		typestr = "BLOB";
		break;
	}

	if (emit_scalar(ctx, "type", typestr) != OBJECTDB_OK)
		return OBJECTDB_ERROR;

	return OBJECTDB_OK;
}

static int emit_object_sha1(struct save_context *ctx, const struct object *o)
{
	char buf[OBJECTID_MD_STR_SIZE];

	objectid_string(&o->id, buf);

	if (emit_scalar(ctx, "sha1", buf) != OBJECTDB_OK)
		return OBJECTDB_ERROR;

	return OBJECTDB_OK;
}

static int emit_conference(struct save_context *ctx, const struct object *o)
{
	const char *subdivision = "UNK";

	if (emit_scalar_map(ctx, "conference") != OBJECTDB_OK)
		return OBJECTDB_ERROR;

	if (emit_scalar(ctx, "name", o->data.conf->name) != OBJECTDB_OK)
		return OBJECTDB_ERROR;

	switch (o->data.conf->subdivision) {
	case CONFERENCE_FBS:
		subdivision = "FBS";
		break;

	case CONFERENCE_FCS:
		subdivision = "FCS";
		break;
	}

	if (emit_scalar(ctx, "subdivision", subdivision) != OBJECTDB_OK)
		return OBJECTDB_ERROR;

	if (emit_map_end(ctx) != OBJECTDB_OK)
		return OBJECTDB_ERROR;

	return OBJECTDB_OK;
}

static int emit_team(struct save_context *ctx, const struct object *o)
{
	char buf[OBJECTID_MD_STR_SIZE];

	objectid_string(&o->data.team->conf_oid, buf);

	if (emit_scalar_map(ctx, "team") != OBJECTDB_OK)
		return OBJECTDB_ERROR;

	if (emit_scalar(ctx, "name", o->data.team->name) != OBJECTDB_OK)
		return OBJECTDB_ERROR;

	if (emit_scalar(ctx, "conf_sha1", buf) != OBJECTDB_OK)
		return OBJECTDB_ERROR;

	if (emit_map_end(ctx) != OBJECTDB_OK)
		return OBJECTDB_ERROR;

	return OBJECTDB_OK;
}

static int emit_game(struct save_context *ctx, const struct object *o)
{
	const int date_len = 11;
	char buf[OBJECTID_MD_STR_SIZE];
	char date[date_len];
	struct tm tm;

	if (emit_scalar_map(ctx, "game") != OBJECTDB_OK)
		return OBJECTDB_ERROR;

	gmtime_r(&o->data.game->date, &tm);
	strftime(date, date_len, "%F", &tm);

	if (emit_scalar(ctx, "date", date) != OBJECTDB_OK)
		return OBJECTDB_ERROR;

	objectid_string(&o->data.game->home_oid, buf);
	if (emit_scalar(ctx, "home_sha1", buf) != OBJECTDB_OK)
		return OBJECTDB_ERROR;

	objectid_string(&o->data.game->away_oid, buf);
	if (emit_scalar(ctx, "away_sha1", buf) != OBJECTDB_OK)
		return OBJECTDB_ERROR;

	if (emit_map_end(ctx) != OBJECTDB_OK)
		return OBJECTDB_ERROR;

	return OBJECTDB_OK;
}

static int emit_object(struct save_context *ctx, const struct object *o)
{
	if (emit_map_begin(ctx) != OBJECTDB_OK)
		return OBJECTDB_ERROR;

	if (emit_object_type(ctx, o) != OBJECTDB_OK)
		return OBJECTDB_ERROR;

	if (emit_object_sha1(ctx, o) != OBJECTDB_OK)
		return OBJECTDB_ERROR;

	switch (o->type) {
	case OBJECTDB_CONF:
		if (emit_conference(ctx, o) != OBJECTDB_OK)
			return OBJECTDB_ERROR;
		break;

	case OBJECTDB_TEAM:
		if (emit_team(ctx, o) != OBJECTDB_OK)
			return OBJECTDB_ERROR;
		break;

	case OBJECTDB_GAME:
		if (emit_game(ctx, o) != OBJECTDB_OK)
			return OBJECTDB_ERROR;
		break;

	case OBJECTDB_BLOB:
		/* do nothing for now */
		break;
	}

	if (emit_map_end(ctx) != OBJECTDB_OK)
		return OBJECTDB_ERROR;

	return OBJECTDB_OK;
}

static int emit_objects(struct save_context *ctx)
{
	int i;

	if (emit_sequence_begin(ctx) != OBJECTDB_OK)
		return OBJECTDB_ERROR;

	for (i = 0; i < ctx->num_objects; i++) {
		if (emit_object(ctx, &ctx->objects[i]) != OBJECTDB_OK)
			return OBJECTDB_ERROR;
	}

	if (emit_sequence_end(ctx) != OBJECTDB_OK)
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
