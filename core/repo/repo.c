#include "repo/repo.h"
#include "strvector/strvector.h"

/*
 * The layout of a repository:
 * mnt/br/a: path the branch is modified at
 * repo: repository
 * repo/br/a:   branch handle in the repo
 * repo/br/a/head:  ID of the last commit in this branch
 * repo/br/a/stage: bundle in the staging state for current changes
 * repo/br/a/stage/root:   the ID of the root dir for this bundle
 * repo/br/a/stage/dirs:   where directory objects mutated by this branch are
 * repo/br/a/stage/pieces: where pieces mutated by this branch are
 * repo/br/a/stage/files:  where files mutated by this branch are
 * repo/dirs:   where directory objects are stored
 * repo/pieces: where piece objects are stored
 * repo/files:  where file objects are stored
 */

const char* DIR_TYPE = "dirs";
const char* PIECE_TYPE = "pieces";
const char* FILE_TYPE = "files";

char* branch_name(repo* rep, const char* abspath);
int moveobjects(const char* dst, const char* src, const char* objtype);

void repo_init(repo* rep) {
	memset(rep, 0, sizeof(repo));
	rwlock_init(&rep->lock, NULL);
}

int repo_open(repo* rep, const char* mountpt, const char* repo) {
	int err = 0;
	rep->mountpt = strdup(mountpt);
	if (!rep->mountpt) {
		err = -ENOMEM;
		goto err;
	}
	rep->repo = strdup(repo);
	if (!rep->repo) {
		err = -ENOMEM;
		goto err;
	}

	stat_t stbuf;
	err = gen_stat(rep->mountpt, &stbuf);
	if (err) {
		goto err;
	}

	err = gen_stat(rep->repo, &stbuf);
	if (err) {
		goto err;
	}

	return 0;
err:
	repo_free(rep);
	return err;
}

int repo_commit(repo* rep, const char* branchpath) {
	int err = 0;
	char* rootpath = NULL;
	char* headpath = NULL;
	char* brname = basename(branchpath);
	char* brhandle = gen_malloc(strlen(branchpath)+strlen(brname)+1);
	if (!brhandle) {
		err = -ENOMEM;
		goto exit;
	}
	strcpy(brhandle, rep->repo);
	strcat(brhandle, brname);

	// Quiesce all FS activity and wait for outstanding meta-data updates
	// to the underlying FS to flush.
	rwlock_wrlock(&rep->lock);
	sync();

	// All objects in stage are now frozen, they can be moved into
	// the globally shared object stores.
	err = moveobjects(rep->repo, branchpath, DIR_TYPE);
	if (err)
		goto exit;
	err = moveobjects(rep->repo, branchpath, PIECE_TYPE);
	if (err)
		goto exit;
	err = moveobjects(rep->repo, branchpath, FILE_TYPE);
	if (err)
		goto exit;

	// Now move the staged root over the branch head to atomically update
	// head.
	rootpath = gen_malloc(strlen(branchpath) + 1 + 4 + 1);
	if (!rootpath) {
		err = -ENOMEM;
		goto exit;
	}
	strcpy(rootpath, branchpath);
	strcat(rootpath, "/root");
	headpath = gen_malloc(strlen(branchpath) + 1 + 4 + 1);
	if (!headpath) {
		err = -ENOMEM;
		goto exit;
	}
	strcpy(headpath, branchpath);
	strcat(headpath, "/head");
	err = gen_rename(rootpath, headpath);
exit:
	if (rootpath)
		gen_free(rootpath);
	if (headpath)
		gen_free(headpath);
	rwlock_wrunlock(&rep->lock);
	if (brhandle)
		gen_free(brhandle);
	return err;
}

int repo_clone(repo* rep, const char* originalbranch, const char* newname) {
	return 0;
}

void repo_free(repo* rep) {
	if (rep->mountpt)
		gen_free(rep->mountpt);
	if (rep->repo)
		gen_free(rep->repo);
}

const int MAX_NAME_LEN = 256;
struct movectx {
	char* dst_template;
	char* dst_template_end;
	char* src_template;
	char* src_template_end;
	strvector commands;
};
int moveobjects_itor(void* vctx, const char* name) {
	int err = 0;
	struct movectx* ctx = (struct movectx*)vctx;
	if (strlen(name) > MAX_NAME_LEN) {
		return -ENOMEM;
	}
	strcat(ctx->src_template, name);
	strcat(ctx->dst_template, name);
	err = strv_append(&ctx->commands, ctx->src_template);
	if (err)
		return err;
	err = strv_append(&ctx->commands, ctx->dst_template);
	*ctx->src_template_end = '\0';
	*ctx->dst_template_end = '\0';
	return err;
}
int moveobjects(const char* dst, const char* src, const char* objtype) {
	char* srcobj = NULL;
	int err = 0;
	struct movectx ctx;
	memset(&ctx, 0, sizeof(struct movectx));
	strv_init(&ctx.commands);

	int srclen = strlen(src);
	int dstlen = strlen(dst);
	int typelen = strlen(objtype);
	ctx.src_template = gen_malloc(srclen + 1 + typelen + 1 + MAX_NAME_LEN + 1);
	if (!ctx.src_template) {
		err = -ENOMEM;
		goto exit;
	}
	ctx.dst_template = gen_malloc(dstlen + 1 + typelen + 1 + MAX_NAME_LEN + 1);
	if (!ctx.dst_template) {
		err = -ENOMEM;
		goto exit;
	}
	strcpy(ctx.src_template, src);
	strcat(ctx.src_template, "/");
	strcat(ctx.src_template, objtype);
	strcat(ctx.src_template, "/");
	strcpy(ctx.dst_template, dst);
	strcat(ctx.dst_template, "/");
	strcat(ctx.dst_template, objtype);
	strcat(ctx.dst_template, "/");
	ctx.src_template_end = ctx.src_template + srclen + 1 + typelen + 1;
	ctx.dst_template_end = ctx.dst_template + dstlen + 1 + typelen + 1;
	srcobj = strdup(ctx.src_template);
	if (!srcobj)
		goto exit;

	err = listdir(srcobj, &ctx, moveobjects_itor);
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
	if (srcobj)
		gen_free(srcobj);
	strv_free(&ctx.commands);
	return err;
}
