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

#define rwlock pthread_rwlock_t
#define rwlock_init pthread_rwlock_init
#define rwlock_rdlock pthread_rwlock_rdlock
#define rwlock_wrlock pthread_rwlock_wrlock
#define rwlock_rdunlock pthread_rwlock_unlock
#define rwlock_wrunlock pthread_rwlock_unlock

#define gen_alloc malloc
#define gen_free free

// defined by includes: strdup
// defined by includes: strnlen

#define print printf

#endif

#endif
