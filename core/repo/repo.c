#include "repo/repo.h"

/*
 * The layout of a repository:
 * mnt/br/a:             path the branch is modified at
 * repo:                 repository
 * repo/id:              last ID that was used
 * repo/br/a:            branch handle in the repo
 * repo/br/a/oldroots:   Old roots are copied here once committed
 * repo/br/a/stage:      bundle in the staging state for current changes
 * repo/br/a/stage/root: the ID of the root dir for this bundle
 * repo/br/a/stage/objs: where objects mutated by this branch are
 * repo/objs:            where read-only-already-committed objects are stored
 *
 * NOTE: objs stores directory, file, and piece objects.
 */

const char* DIR_TYPE = "dirs";
const char* PIECE_TYPE = "pieces";
const char* FILE_TYPE = "files";

void repo_init(repo* rep) {
	memset(rep, 0, sizeof(repo));
	rwlock_init(&rep->fslock, NULL);
	mutex_init(&rep->idlock, NULL);
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

	err = setmaxid(rep);
	if (err) {
		goto err;
	}

	return 0;
err:
	repo_destroy(rep);
	return err;
}

int repo_commit(repo* rep, const char* branchpath) {
	int err = 0;
	char* srcpath = gen_malloc(MAX_PATH_LEN);
	if (!srcpath) {
		err = -ENOMEM;
		goto exit;
	}
	char* dstpath = gen_malloc(MAX_PATH_LEN);
	if (!dstpath) {
		err = -ENOMEM;
		goto exit;
	}

	// Quiesce all FS activity and wait for outstanding meta-data updates
	// to the underlying FS to flush.
	rwlock_wrlock(&rep->fslock);
	sync();

	// All objects in stage are now frozen, they can be moved into
	// the globally shared object stores.
	gen_sprintf(srcpath, "%s/stage/objs", branchpath);
	gen_sprintf(dstpath, "%s/objs", rep->repo);
	err = moveobjects(dstpath, srcpath);
	if (err)
		goto exit_unlock;

	// Now move the staged root into the set of old roots
	uint64_t id = repo_newid(rep);
	gen_sprintf(srcpath, "%s/stage/root", branchpath);
	gen_sprintf(dstpath, "%s/oldroots/i%lu", branchpath, id);
	err = gen_rename(srcpath, dstpath);

exit_unlock:
	rwlock_wrunlock(&rep->fslock);
exit:
	if (srcpath)
		gen_free(srcpath);
	if (dstpath)
		gen_free(dstpath);
	return err;
}

int repo_new(repo* rep, const char* newname) {
	int err = 0;
	// New branches track empty directories. The branch has no old roots.
	// It is staging and the current root being staged points to the only
	// object already staged: an empty directory.

	// Create a string buffer for making paths
	char* path = gen_malloc(MAX_PATH_LEN);
	if (!path) {
		err = -ENOMEM;
		goto exit;
	}
	gen_sprintf(path, "%s/br/%s", rep->repo, newname);

	// mkdir <rep->repo>/br/<newname>
	err = gen_mkdir(path, (mode_t)0700);
	if (err)
		goto exit;

	// mkdir <rep->repo>/br/<newname>/oldroots
	strcat(path, "/oldroots");
	err = gen_mkdir(path, (mode_t)0700);
	if (err)
		goto exit;

	// mkdir <rep->repo>/br/<newname>/stage
	gen_sprintf(path, "%s/br/%s/stage", rep->repo, newname);
	err = gen_mkdir(path, (mode_t)0700);
	if (err)
		goto exit;

	// mkdir <rep->repo>/br/<newname>/stage/objs
	strcat(path, "/objs");
	err = gen_mkdir(path, (mode_t)0700);
	if (err)
		goto exit;

	// mkdir <rep->repo>/br/<newname>/stage/objs/<nextid>
	uint64_t nextid = repo_newid(rep);
	gen_sprintf(path, "%s/br/%s/%lu", rep->repo, newname, nextid);
	err = gen_mkdir(path, (mode_t)0700);
	if (err)
		goto exit;

	// echo `repo_newid` > <rep->repo>/br/<newname>/stage/root
	gen_sprintf(path, "%s/br/%s/root", rep->repo, newname);
	int fd = gen_open(path, O_CREAT|O_RDWR, (mode_t)0700);
	if (fd < 0) {
		err = fd;
		goto exit;
	}
	gen_sprintf(path, "%lu\n", nextid);
	int nbytes;
	err = gen_write(fd, path, strlen(path), &nbytes);
	if (err)
		goto exit;
	gen_close(fd);
	sync();
exit:
	gen_free(path);
	return err;
}

int repo_read(repo* repo, const char* path, char* buf, off_t offset, size_t size) {
	return 0;
}

int repo_write(repo* repo, const char* path, char* buf, off_t offset, size_t size) {
	return 0;
}

uint64_t repo_newid(repo* rep) {
	mutex_lock(&rep->idlock);
	uint64_t id = rep->id++;
	mutex_unlock(&rep->idlock);
	return id;
}

void repo_destroy(repo* rep) {
	if (rep->mountpt)
		gen_free(rep->mountpt);
	if (rep->repo)
		gen_free(rep->repo);
	rwlock_destroy(&rep->fslock);
	mutex_destroy(&rep->idlock);
}

