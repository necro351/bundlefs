#ifndef REPO_H
#define REPO_H

#include "context/context.h"

typedef struct {
	char* mountpt;
	char* repo;
	rwlock lock;
} repo;

void repo_init(repo* rep);
int repo_open(repo* rep, const char* mountpt, const char* repo);
int repo_commit(repo* rep, const char* abspath);
void repo_free(repo* rep);

#endif
