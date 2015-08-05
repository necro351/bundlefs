#ifndef LOCKTREE_H
#define LOCKTREE_H

#include "context/context.h"
#include "strvector/strvector.h"

typedef struct {
	rwlock lock;
	strvector children;
} ltnode;

void ltnode_init(ltnode *root);
int ltnode_wrlock(ltnode *root, const char* path);
int ltnode_rdlock(ltnode *root, const char* path);
int ltnode_wrunlock(ltnode *root, const char* path);
int ltnode_rdunlock(ltnode *root, const char* path);

#endif
