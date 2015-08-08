/*
FUSE: Filesystem in Userspace
Copyright (C) 2001-2007  Miklos Szeredi <miklos@szeredi.hu>

This program can be distributed under the terms of the GNU GPL.
See the file COPYING.

gcc -Wall bundle.c `pkg-config fuse --cflags --libs` -o bundle
*/

#define FUSE_USE_VERSION 26

#include <fuse.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>

#include "../core/context/context.h"

static const char *bundle_str = "Hello World!\n";
static const char *bundle_path = "/bundle";

static char* repo;
static char* mountpt;

static int bundle_ioctl(const char *path, int cmd, void *arg,
		      struct fuse_file_info *info, unsigned int flags, void *data)
{
	// Wait for ongoing operations to cease so we can ensure all
	// directories, files, and pieces will not be modified once the commit
	// is complete.

	// Determine the branch being committed by removing the mountpt prefix
	// from the absolute path and then removing the head of the path.

	return 0;
}

static int bundle_getattr(const char *path, struct stat *stbuf)
{
	int res = 0;

	memset(stbuf, 0, sizeof(struct stat));
	if (strcmp(path, "/") == 0) {
		stbuf->st_mode = S_IFDIR | 0755;
		stbuf->st_nlink = 2;
	} else if (strcmp(path, bundle_path) == 0) {
		stbuf->st_mode = S_IFREG | 0444;
		stbuf->st_nlink = 1;
		stbuf->st_size = strlen(bundle_str);
	} else
		res = -ENOENT;

	return res;
}

static int bundle_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
		off_t offset, struct fuse_file_info *fi)
{
	(void) offset;
	(void) fi;

	if (strcmp(path, "/") != 0)
		return -ENOENT;

	filler(buf, ".", NULL, 0);
	filler(buf, "..", NULL, 0);
	filler(buf, bundle_path + 1, NULL, 0);

	return 0;
}

static int bundle_open(const char *path, struct fuse_file_info *fi)
{
	if (strcmp(path, bundle_path) != 0)
		return -ENOENT;

	if ((fi->flags & 3) != O_RDONLY)
		return -EACCES;

	return 0;
}

static int bundle_read(const char *path, char *buf, size_t size, off_t offset,
		struct fuse_file_info *fi)
{
	size_t len;
	(void) fi;
	if(strcmp(path, bundle_path) != 0)
		return -ENOENT;

	len = strlen(bundle_str);
	if (offset < len) {
		if (offset + size > len)
			size = len - offset;
		memcpy(buf, bundle_str + offset, size);
	} else
		size = 0;

	return size;
}

static struct fuse_operations bundle_oper = {
	.getattr	= bundle_getattr,
	.readdir	= bundle_readdir,
	.open		= bundle_open,
	.read		= bundle_read,
	.ioctl          = bundle_ioctl,
};

int main(int argc, char *argv[])
{
	return fuse_main(argc, argv, &bundle_oper, NULL);
}
