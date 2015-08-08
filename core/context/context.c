#include "context/context.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <dirent.h>
#include <fcntl.h>

int gen_stat(const char* path, stat_t* buf) {
	if (stat(path, buf)) {
		return -errno;
	}
	return 0;
}

int gen_rename(const char* oldpath, const char* newpath) {
	if (rename(oldpath, newpath)) {
		return -errno;
	}
	return 0;
}

int gen_mkdir(const char* path, mode_t mode) {
	if (mkdir(path, mode)) {
		return -errno;
	}
	return 0;
}

int gen_open(const char* path, int options, mode_t mode) {
	int fd = open(path, options, mode);
	if (fd < 0) {
		return -errno;
	}
	return fd;
}

int gen_write(int fd, const char* buffer, size_t size, int* nbytes) {
	*nbytes = write(fd, buffer, size);
	if (*nbytes < 0) {
		return -errno;
	}
	return 0;
}

int gen_read(int fd, char* buffer, size_t size, int* nbytes) {
	*nbytes = read(fd, buffer, size);
	if (*nbytes < 0) {
		return -errno;
	}
	return 0;
}

int gen_close(int fd) {
	if (close(fd)) {
		return -errno;
	}
	return 0;
}

int listdir(const char* dirpath, void* ctx, entryitor_t itor) {
	int err = 0;
	struct dirent *ep;     
	DIR* dp = opendir(dirpath);

	if (dp == NULL) {
		return -ENOENT;
	}

	while ((ep = readdir(dp))) {
		err = itor(ctx, ep->d_name);
		if (err != 0) {
			break;
		}
	}
	(void) closedir(dp);

	return err;
}
