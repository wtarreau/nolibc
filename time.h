/*
 * time function definitions for NOLIBC
 * Copyright (C) 2017-2022 Willy Tarreau <w@1wt.eu>
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */

/* make sure to include all global symbols */
#include "nolibc.h"

#ifndef _NOLIBC_TIME_H
#define _NOLIBC_TIME_H

#include "std.h"
#include "arch.h"
#include "types.h"
#include "sys.h"

#include <linux/time.h>

static __inline__
void __nolibc_timespec_user_to_kernel(const struct timespec *ts, struct __kernel_timespec *kts)
{
	kts->tv_sec = ts->tv_sec;
	kts->tv_nsec = ts->tv_nsec;
}

static __inline__
void __nolibc_timespec_kernel_to_user(const struct __kernel_timespec *kts, struct timespec *ts)
{
	ts->tv_sec = kts->tv_sec;
	ts->tv_nsec = kts->tv_nsec;
}

/*
 * int clock_getres(clockid_t clockid, struct timespec *res);
 * int clock_gettime(clockid_t clockid, struct timespec *tp);
 * int clock_settime(clockid_t clockid, const struct timespec *tp);
 */

static __attribute__((unused))
int sys_clock_getres(clockid_t clockid, struct timespec *res)
{
#if defined(__NR_clock_getres)
	return my_syscall2(__NR_clock_getres, clockid, res);
#elif defined(__NR_clock_getres_time64)
	struct __kernel_timespec kres;
	int ret;

	ret = my_syscall2(__NR_clock_getres_time64, clockid, &kres);
	if (res)
		__nolibc_timespec_kernel_to_user(&kres, res);
	return ret;
#else
	return __nolibc_enosys(__func__, clockid, res);
#endif
}

static __attribute__((unused))
int clock_getres(clockid_t clockid, struct timespec *res)
{
	return __sysret(sys_clock_getres(clockid, res));
}

static __attribute__((unused))
int sys_clock_gettime(clockid_t clockid, struct timespec *tp)
{
#if defined(__NR_clock_gettime)
	return my_syscall2(__NR_clock_gettime, clockid, tp);
#elif defined(__NR_clock_gettime64)
	struct __kernel_timespec ktp;
	int ret;

	ret = my_syscall2(__NR_clock_gettime64, clockid, &ktp);
	if (tp)
		__nolibc_timespec_kernel_to_user(&ktp, tp);
	return ret;
#else
	return __nolibc_enosys(__func__, clockid, tp);
#endif
}

static __attribute__((unused))
int clock_gettime(clockid_t clockid, struct timespec *tp)
{
	return __sysret(sys_clock_gettime(clockid, tp));
}

static __attribute__((unused))
int sys_clock_settime(clockid_t clockid, struct timespec *tp)
{
#if defined(__NR_clock_settime)
	return my_syscall2(__NR_clock_settime, clockid, tp);
#elif defined(__NR_clock_settime64)
	struct __kernel_timespec ktp;

	__nolibc_timespec_user_to_kernel(tp, &ktp);
	return my_syscall2(__NR_clock_settime64, clockid, &ktp);
#else
	return __nolibc_enosys(__func__, clockid, tp);
#endif
}

static __attribute__((unused))
int clock_settime(clockid_t clockid, struct timespec *tp)
{
	return __sysret(sys_clock_settime(clockid, tp));
}


static __attribute__((unused))
time_t time(time_t *tptr)
{
	struct timeval tv;

	/* note, cannot fail here */
	sys_gettimeofday(&tv, NULL);

	if (tptr)
		*tptr = tv.tv_sec;
	return tv.tv_sec;
}

#endif /* _NOLIBC_TIME_H */
