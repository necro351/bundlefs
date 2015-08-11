#ifndef REPO_H
#define REPO_H

#include "context/context.h"
#include "strvector/strvector.h"

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
// outside of repo. It is OK to use symbols these in repo_test.c.
int moveobjects(const char* dst, const char* src);
int setmaxid(repo* rep);
int getvalue(char* object, char* value);

typedef struct {
	const char* objecstore;
	char* objpath;
	char* storesuffix;
	char* idsuffix;
	char value[MAX_OBJID_LEN];
} object;

#define OBJECT_HIDDEN_FILE ".bundlefs_type"

int object_init(object* obj, const char* objectstore, const char* objid);
extern const int OBJECT_TYPE_DIR;
extern const int OBJECT_TYPE_FILE;
int object_type(object* obj, int* type);
int object_member(object* obj, const char* membername);
void object_chobj(object* obj, const char* objid);
void object_destroy(object* obj);

typedef struct {
	strvector components;
	char** objids;
	int stageddepth;
} path;

int path_init(path* pth);
void path_clear(path* pth);
int path_lookup(path* pth, const char* stage, const char* path);
int path_fault(path* pth);
void path_destroy(path* pth);

#endif
