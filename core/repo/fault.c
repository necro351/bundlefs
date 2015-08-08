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

const int LOOKUP_DIR = 0;
const int LOOKUP_FILE = 1;
const int LOOKUP_NOENT = 2;
char* path_head(char** path);
int dirobjlookup(char* objectstore, char* objectstoreend, const char* name, char* obj);
int getsmallobj(char* objectstore, char* obj);

/*
 * Create new directory objects in the branch's stage such that the file or
 * directory indicated by path can be mutated without modifying any already
 * committed objects.
 *
 * brname is the branch name (e.g., a)
 * path is the path rooted at brname (e.g., home/rick/photos/738.jp)
 * obj is a buffer large enough to hold the ID of the file or directory at the
 *   end of the path
 *
 * The example brname and path above are what would be used for an operation on
 * /bundlefs/mnt/br/a/home/rick/photos/738.jpg.
 */
int faultpath(repo* rep, const char* brname, const char* path, char* obj) {
	int err = 0;
	char* pathbuffer = strdup(path);
	char* pathcopy = pathbuffer;
	if (!pathcopy) {
		err = -ENOMEM;
		goto exit;
	}
	char* objectstore = gen_malloc(MAX_PATH_LEN);
	if (!objectstore) {
		err = -ENOMEM;
		goto exit;
	}

	gen_sprintf(objectstore, "%s/br/%s/stage/root", rep->repo, brname);
	err = getsmallobj(objectstore, obj);
	if (err)
		goto exit;
	char* name = obj;
	int objectstorelen = gen_sprintf(objectstore, "%s/br/%s/stage/objs/", rep->repo, brname);
	char* endobjectstore = objectstore + objectstorelen;
	err = -EIO; // if err not set then there is no root (!?) treat it as EIO
	while (name != NULL) {
		err = dirobjlookup(objectstore, endobjectstore, name, obj);
		if (err < 0)
			goto exit;
		else if (err == LOOKUP_FILE) {
			// It's OK for the component to be a file iff it's last
			char* next = path_head(&pathcopy);
			if (next) {
				err = -ENOENT;
			}
			goto exit;
		} else if (err == LOOKUP_NOENT) {
			gen_sprintf(endobjectstore, "i%lu", repo_newid(rep));
			err = gen_mkdir(objectstore, (mode_t)0700);
			if (err)
				goto exit;
		} else /* err == LOOKUP_DIR */
			strcpy(endobjectstore, obj);
		name = path_head(&pathcopy);
	}
exit:
	if (pathbuffer)
		gen_free(pathbuffer);
	if (objectstore)
		gen_free(objectstore);
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

int dirobjlookup(char* objectstore, char* objectstoreend, const char* name, char* obj) {
	strcpy(objectstoreend, name);
	int err = getsmallobj(objectstore, obj);
	*objectstoreend = '\0';
	return err;
}

int getsmallobj(char* objectstore, char* obj) {
	int err = 0;
	int fd = gen_open(objectstore, O_RDONLY, 0400);
	if (fd < 0) {
		return fd;
	}
	int nbytes = gen_read(fd, obj, MAX_OBJID_LEN-1);
	if (nbytes < 0) {
		err = -errno;
		goto exit;
	}
	obj[nbytes-1] = '\0';
exit:
	gen_close(fd);
	return err;
}
