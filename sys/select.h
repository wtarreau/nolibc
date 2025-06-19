/* SPDX-License-Identifier: LGPL-2.1 OR MIT */

/* make sure to include all global symbols */
#include "../nolibc.h"

#ifndef _NOLIBC_SYS_SELECT_H
#define _NOLIBC_SYS_SELECT_H

/* commonly an fd_set represents 256 FDs */
#ifndef FD_SETSIZE
#define FD_SETSIZE     256
#endif

#define FD_SETIDXMASK (8 * sizeof(unsigned long))
#define FD_SETBITMASK (8 * sizeof(unsigned long)-1)

/* for select() */
typedef struct {
	unsigned long fds[(FD_SETSIZE + FD_SETBITMASK) / FD_SETIDXMASK];
} fd_set;

#define FD_CLR(fd, set) do {						\
		fd_set *__set = (set);					\
		int __fd = (fd);					\
		if (__fd >= 0)						\
			__set->fds[__fd / FD_SETIDXMASK] &=		\
				~(1U << (__fd & FD_SETBITMASK));	\
	} while (0)

#define FD_SET(fd, set) do {						\
		fd_set *__set = (set);					\
		int __fd = (fd);					\
		if (__fd >= 0)						\
			__set->fds[__fd / FD_SETIDXMASK] |=		\
				1 << (__fd & FD_SETBITMASK);		\
	} while (0)

#define FD_ISSET(fd, set) ({						\
			fd_set *__set = (set);				\
			int __fd = (fd);				\
		int __r = 0;						\
		if (__fd >= 0)						\
			__r = !!(__set->fds[__fd / FD_SETIDXMASK] &	\
1U << (__fd & FD_SETBITMASK));						\
		__r;							\
	})

#define FD_ZERO(set) do {						\
		fd_set *__set = (set);					\
		int __idx;						\
		int __size = (FD_SETSIZE+FD_SETBITMASK) / FD_SETIDXMASK;\
		for (__idx = 0; __idx < __size; __idx++)		\
			__set->fds[__idx] = 0;				\
	} while (0)

#endif /* _NOLIBC_SYS_SELECT_H */
