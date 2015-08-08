#ifndef REPO_H
#define REPO_H

#include "context/context.h"

#define MAX_OBJID_LEN 128
#define MAX_PATH_LEN  4096

typedef struct {
	char* mountpt;
	char* repo;
	uint64_t id;
	rwlock fslock;
	mutex idlock;
} repo;

void repo_init(repo* rep);
int repo_new(repo* rep, const char* newname);
int repo_open(repo* rep, const char* mountpt, const char* repo);
int repo_commit(repo* rep, const char* abspath);
void repo_destroy(repo* rep);
uint64_t repo_newid(repo* rep);

// Declarations private within the repo object, do not use these symbols
// outside of repo.
extern const int BUNDLE_TYPE_DIR;
extern const int BUNDLE_TYPE_FILE;
int moveobjects(const char* dst, const char* src);
int setmaxid(repo* rep);

#endif
