#include "context/context.h"
#include "repo/repo.h"

static int setmaxid_itor(void* vctx, const char* name) {
	if (!strcmp(name, "."))
		return 0;
	if (!strcmp(name, ".."))
		return 0;
	repo* rep = (repo*)vctx;
	uint64_t id;
	int numread = gen_sscanf(name, "i%lu", &id);
	if (numread == 1 && id > rep->id)
		rep->id = id + 1;
	return 0;
}

int setmaxid(repo* rep) {
	int err = 0;
	char* path = gen_malloc(MAX_PATH_LEN);
	if (!path)
		return -ENOMEM;
	gen_sprintf(path, "%s/objs", rep->repo);
	err = listdir(path, rep, setmaxid_itor);
	// The max ID found is in use, and the invariant is that rep->id is
	// the next available ID, so we have to bump it to make sure its
	// unused.
	rep->id++;
	free(path);
	return err;
}
