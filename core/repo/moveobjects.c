#include "repo/repo.h"
#include "strvector/strvector.h"

const int MAX_NAME_LEN = 256;
struct movectx {
	char* dst_template;
	char* dst_template_end;
	char* src_template;
	char* src_template_end;
	strvector commands;
};

static int moveobjects_itor(void* vctx, const char* name) {
	if (!strcmp(name, "."))
		return 0;
	if (!strcmp(name, ".."))
		return 0;
	int err = 0;
	struct movectx* ctx = (struct movectx*)vctx;
	if (strlen(name) > MAX_NAME_LEN)
		return -ENOMEM;
	strcpy(ctx->src_template_end, name);
	strcpy(ctx->dst_template_end, name);
	err = strv_append(&ctx->commands, ctx->src_template);
	if (err)
		return err;
	err = strv_append(&ctx->commands, ctx->dst_template);
	return err;
}

int moveobjects(const char* dst, const char* src) {
	int err = 0;
	struct movectx ctx;
	memset(&ctx, 0, sizeof(struct movectx));
	strv_init(&ctx.commands);

	int srclen = strlen(src);
	int dstlen = strlen(dst);
	ctx.src_template = gen_malloc(MAX_PATH_LEN);
	if (!ctx.src_template) {
		err = -ENOMEM;
		goto exit;
	}
	ctx.dst_template = gen_malloc(MAX_PATH_LEN);
	if (!ctx.dst_template) {
		err = -ENOMEM;
		goto exit;
	}
	strcpy(ctx.src_template, src);
	ctx.src_template[srclen] = '/';
	strcpy(ctx.dst_template, dst);
	ctx.dst_template[dstlen] = '/';
	ctx.src_template_end = ctx.src_template + srclen + 1;
	ctx.dst_template_end = ctx.dst_template + dstlen + 1;

	err = listdir(src, &ctx, moveobjects_itor);
	if (err)
		goto exit;

	int i;
	for (i = 0; i < ctx.commands.size; i += 2) {
		err = gen_rename(ctx.commands.strings[i], ctx.commands.strings[i+1]);
		if (err)
			goto exit;
	}
exit:
	if (ctx.src_template)
		gen_free(ctx.src_template);
	if (ctx.dst_template)
		gen_free(ctx.dst_template);
	strv_free(&ctx.commands);
	return err;
}
