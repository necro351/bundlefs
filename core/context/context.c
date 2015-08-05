#include "context/context.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

int gen_stat(const char* path, stat_t* buf) {
	if (stat(path, buf)) {
		return -errno;
	}
	return 0;
}
