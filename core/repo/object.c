#include "context/context.h"
#include "repo/repo.h"

const int OBJECT_TYPE_DIR = 'd';
const int OBJECT_TYPE_FILE = 'f';

int object_init(object* obj, const char* objectstore, const char* objid) {
	memset(obj, 0, sizeof(object));
	obj->objpath = gen_malloc(MAX_PATH_LEN);
	if (!obj->objpath)
		return -ENOMEM;
	strcpy(obj->objpath, objectstore);
	strcat(obj->objpath, "/");
	obj->storesuffix = obj->objpath + strlen(objectstore) + 1;
	strcpy(obj->storesuffix, objid);
	strcat(obj->storesuffix, "/");
	obj->idsuffix = obj->storesuffix + strlen(objid) + 1;
	return 0;
}

int object_type(object* obj, int* type) {
	strcpy(obj->idsuffix, OBJECT_HIDDEN_FILE);
	obj->value[0] = '\0';
	int err = getvalue(obj->objpath, obj->value);
	*type = obj->value[0];
	return err;
}

void object_chobj(object* obj, const char* objid) {
	strcpy(obj->storesuffix, objid);
	strcat(obj->storesuffix, "/");
	obj->idsuffix = obj->storesuffix + strlen(objid) + 1;
}

int object_member(object* obj, const char* membername) {
	strcpy(obj->idsuffix, membername);
	int err = getvalue(obj->objpath, obj->value);
	return err;
}

void object_destroy(object* obj) {
	gen_free(obj->objpath);
}

