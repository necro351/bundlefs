#ifndef CONTEXT_H
#define CONTEXT_H

/* Context provides the necessary underlying operations needed
 * for the core library to compile and build either in a POSIX
 * userspace or a Linux kernel environment. For now only the
 * POSIX user space implementation is provided, but other
 * contexts can be added later on.
 *
 * For functions that are frequently aliased, prefixed, etc..., a special
 * 'gen_' prefix is used to make sure they are differentiated (e.g., gen_alloc
 * for 'malloc' or 'kalloc' since we can't likely just use 'alloc').
 *
 * For functions that are defined with the same name no matter the context and
 * have no ambiguity they are documented with a comment, e.g., 'defined by
 * includes: X'.
 */

// POSIX context definitions
#ifdef POSIX_CONTEXT

#include <string.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/stat.h>
#include <unistd.h>
#include <inttypes.h>
#include <fcntl.h>

typedef pthread_rwlock_t rwlock;
#define rwlock_init pthread_rwlock_init
#define rwlock_destroy pthread_rwlock_destroy
#define rwlock_rdlock pthread_rwlock_rdlock
#define rwlock_wrlock pthread_rwlock_wrlock
#define rwlock_rdunlock pthread_rwlock_unlock
#define rwlock_wrunlock pthread_rwlock_unlock

typedef pthread_mutex_t mutex;
#define mutex_init pthread_mutex_init
#define mutex_destroy pthread_mutex_destroy
#define mutex_lock pthread_mutex_lock
#define mutex_unlock pthread_mutex_unlock

#define gen_realloc realloc
#define gen_malloc malloc
#define gen_free free
// defined by includes: exit

#define gen_sprintf sprintf
#define gen_printf printf
#define gen_sscanf sscanf
// defined by includes: strdup
// defined by includes: strnlen
// defined by includes: strcmp
// defined by includes: memset

// defined by includes: stat
// defined by includes: sync
typedef struct stat stat_t;
int gen_stat(const char* path, stat_t* buf);
int gen_rename(const char* oldpath, const char* newpath);
int gen_mkdir(const char* path, mode_t mode);
typedef int(*entryitor_t)(void* ctx, const char* name);
int listdir(const char* dirpath, void* ctx, entryitor_t itor);
int gen_open(const char* path, int options, mode_t mode);
int gen_close(int fd);
int gen_write(int fd, const char* buffer, size_t size);
int gen_read(int fd, char* buffer, size_t size);
// defined by includes: O_CREAT
// defined by includes: O_RDWR

// defined by includes: uint64_t

// defined by includes: ENOMEM

#endif

#endif
