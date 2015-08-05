#include "locktree/locktree.h"

void ltnode_init(ltnode *root) {
	memset(root, 0, sizeof(ltnode));
	rwlock_init(&root->lock, NULL);
}

int ltnode_wrlock(ltnode *root, const char* path) {
	/*
	int err = 0;
	char* chopped_path = strdup(path);
	if (!chopped_path) {
		return -ENOMEM;
	}
	char* pathcomp = path_head(&chopped_path);
	while (pathcomp != NULL) {
		rwlock_wrlock(&root->lock);
		ltnode* child = ltv_find(&root->children, pathcomp);
		if (!child) {
			err = ltv_append(root->children, pathcomp);
			if (err < 0) {
				goto out;
			}

		}
	}
out:
	gen_free(chopped_path);
	return err;
	*/
	return 0;
}

int ltnode_rdlock(ltnode *root, const char* path) {
	return 0;
}

int ltnode_wrunlock(ltnode *root, const char* path) {
	return 0;
}

int ltnode_rdunlock(ltnode *root, const char* path) {
	return 0;
}

int find_comp(ltnode* root, char* comp) {
	int i;
	for (i = 0; i < root->children.size; i++) {
		if (!strcmp(root->children.strings[i], comp)) {
			return i;
		}
	}
	return -1;
}

char* path_head(char** path) {
	char* end = *path;
	if (*end == '\0')
		return NULL;
	while (*end != '/' && *end != '\0')
		end++;
	while (*end == '/') {
		*end = '\0';
		end++;
	}
	char* head = *path;
	*path = end;
	return head;
}
