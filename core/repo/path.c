#include "context/context.h"
#include "repo/repo.h"

/*
 * All changes are immediately staged. This is because any time a file in a
 * branch is modified, first the path is faulted, and then individual 16MB
 * pieces of files are faulted to apply writes. Faulting makes copies of just
 * the portions of the branch that are being modified while still linking to
 * the remaining portions that have not been modified.
 *
 * The effect of a faultpath is that new objects are created and inserted into
 * the branch's stage directory. Here is the effect of faultpath on
 * "mnt/br/a/home/rick/photos/732.jpg"
 * PRE-EXISTING (objects in /repo/objs):
 * repo/objs/42: [usr:38, home:37, etc:36]
 * repo/objs/37: [rick:14, sean:13]
 * repo/objs/14: [docs:8, photos:7, desktop:6]
 * repo/objs/7:  [731.jpg:5, 732.jpg:4]
 * repo/objs/4:  [0: 3]
 * NEWLY ADDED (new objects below are in /repo/br/a/stage/objs until commit):
 * repo/objs/43: [usr:38, home:44, etc:36]
 * repo/objs/44: [rick:45, sean:13]
 * repo/objs/45: [docs:8, photos:46, desktop:6]
 * repo/objs/46: [731.jpg:5, 732.jpg:4]
 */

char* path_head(char** path);
int getobjtype(char* object, int* type);
int getvalue(char* objectstore, char* obj);
int fillcomps(path* pth, const char* path);

int path_init(path* pth) {
	memset(pth, 0, sizeof(path));
	strv_init(&pth->components);
	return 0;
}

void path_destroy(path* pth) {
	path_clear(pth);
	strv_destroy(&pth->components);
}

void path_clear(path* pth) {
	strv_clear(&pth->components);
	if (!pth->objids)
		return;
	int i;
	for (i = 0; i < pth->components.size; i++)
		gen_free(pth->objids[i]);
	gen_free(pth->objids);
	pth->objids = NULL;
}

/*
 * Clear the existing path components and object IDs and then using the given
 * path perform a lookup and populate the components and object IDs.
 */
int path_lookup(path* pth, const char* stage, const char* path) {
	int err = 0;
	char* objpathbuf = gen_malloc(MAX_PATH_LEN);
	if (!objpathbuf) {
		err = -ENOMEM;
		goto exit;
	}
	path_clear(pth);

	err = fillcomps(pth, path);
	if (err)
		goto exit;

	gen_sprintf(objpathbuf, "%s/root", stage);
	char rootobjid[MAX_OBJID_LEN];
	err = getvalue(objpathbuf, rootobjid);
	if (err)
		goto exit;

	gen_sprintf(objpathbuf, "%s/objs", stage);
	object obj;
	err = object_init(&obj, objpathbuf, rootobjid);
	if (err)
		goto exit;
	int type;
	err = object_type(&obj, &type);
	if (err)
		goto exit;
	pth->objids[0] = strdup(rootobjid);
	if (!pth->objids[0]) {
		err = -ENOMEM;
		goto exit;
	}
	int i;
	for (i = 1; i < pth->components.size; i++) {
		char* name = pth->components.strings[i];
		if (type != OBJECT_TYPE_DIR) {
			err = -ENOENT;
			goto exit;
		}
		int err = object_member(&obj, name);
		if (err)
			goto exit;
		pth->objids[i] = strdup(obj.value);
		if (!pth->objids[i-1]) {
			err = -ENOMEM;
			goto exit;
		}
		object_chobj(&obj, obj.value);
	}
exit:
	if (objpathbuf)
		gen_free(objpathbuf);
	return err;
}

int fillcomps(path* pth, const char* path) {
	int err = 0;
	char* pathbuf = strdup(path);
	if (!pathbuf) {
		err = -ENOMEM;
		goto exit;
	}
	char* pathcopy = pathbuf;
	if (path[0] == '\0' || path[0] == '/') {
		err = -EINVAL;
		goto exit;
	}
	err = strv_append(&pth->components, "/");
	if (err)
		goto exit;
	char* component = path_head(&pathcopy);
	while (component) {
		err = strv_append(&pth->components, component);
		if (err)
			goto exit;
		component = path_head(&pathcopy);
	}

	pth->objids = gen_malloc(pth->components.size*sizeof(char*));
	if (!pth->objids) {
		err = -ENOMEM;
		goto exit;
	}
	int i;
	for (i = 0; i < pth->components.size; i++)
		pth->objids[i] = NULL;
exit:
	if (pathbuf)
		gen_free(pathbuf);
	return err;
}

int faultrange(repo* rep, const char* path, off_t offset, size_t size) {
	// Is the file faulted in? If not, fault it in now.
	
	// What pieces does the range overlap with? Are they all faulted in?
	// If not, fault them in now.
	return 0;
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

int getvalue(char* object, char* value) {
	int err = 0;
	int fd = gen_open(object, O_RDONLY, 0400);
	if (fd < 0) {
		return fd;
	}
	int nbytes;
	err = gen_read(fd, value, MAX_OBJID_LEN-1, &nbytes);
	if (err)
		goto exit;
	value[nbytes-1] = '\0';
exit:
	gen_close(fd);
	return err;
}
