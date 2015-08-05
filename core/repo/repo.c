#include "repo/repo.h"

char* branch_name(repo* rep, const char* abspath);

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

int repo_commit(repo* rep, const char* abspath) {
	char* brname = branch_name(rep, abspath);
	gen_free(brname);
	return 0;
}

void repo_free(repo* rep) {
	if (rep->mountpt)
		gen_free(rep->mountpt);
	if (rep->repo)
		gen_free(rep->repo);
}

char* branch_name(repo* rep, const char* abspath) {
	return NULL;
}
