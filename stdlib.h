/*
 * stdlib function definitions for NOLIBC
 * Copyright (C) 2017-2021 Willy Tarreau <w@1wt.eu>
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

#ifndef _NOLIBC_STDLIB_H
#define _NOLIBC_STDLIB_H

#include "std.h"
#include "arch.h"
#include "types.h"
#include "sys.h"
#include "string.h"
#include <linux/auxvec.h>

struct nolibc_heap {
	size_t	len;
	char	user_p[] __attribute__((__aligned__));
};

/* Buffer used to store int-to-ASCII conversions. Will only be implemented if
 * any of the related functions is implemented. The area is large enough to
 * store "18446744073709551615" or "-9223372036854775808" and the final zero.
 */
static __attribute__((unused)) char itoa_buffer[21];

/*
 * As much as possible, please keep functions alphabetically sorted.
 */

static __inline__
int abs(int j)
{
	return j >= 0 ? j : -j;
}

static __inline__
long labs(long j)
{
	return j >= 0 ? j : -j;
}

static __inline__
long long llabs(long long j)
{
	return j >= 0 ? j : -j;
}

/* must be exported, as it's used by libgcc for various divide functions */
void abort(void);
__attribute__((weak,unused,noreturn,section(".text.nolibc_abort")))
void abort(void)
{
	sys_kill(sys_getpid(), SIGABRT);
	for (;;);
}

static __attribute__((unused))
long atol(const char *s)
{
	unsigned long ret = 0;
	unsigned long d;
	int neg = 0;

	if (*s == '-') {
		neg = 1;
		s++;
	}

	while (1) {
		d = (*s++) - '0';
		if (d > 9)
			break;
		ret *= 10;
		ret += d;
	}

	return neg ? -ret : ret;
}

static __attribute__((unused))
int atoi(const char *s)
{
	return atol(s);
}

static __attribute__((unused))
void free(void *ptr)
{
	struct nolibc_heap *heap;

	if (!ptr)
		return;

	heap = container_of(ptr, struct nolibc_heap, user_p);
	munmap(heap, heap->len);
}

/* getenv() tries to find the environment variable named <name> in the
 * environment array pointed to by global variable "environ" which must be
 * declared as a char **, and must be terminated by a NULL (it is recommended
 * to set this variable to the "envp" argument of main()). If the requested
 * environment variable exists its value is returned otherwise NULL is
 * returned.
 */
static __attribute__((unused))
char *getenv(const char *name)
{
	int idx, i;

	if (environ) {
		for (idx = 0; environ[idx]; idx++) {
			for (i = 0; name[i] && name[i] == environ[idx][i];)
				i++;
			if (!name[i] && environ[idx][i] == '=')
				return &environ[idx][i+1];
		}
	}
	return NULL;
}

static __attribute__((unused))
void *malloc(size_t len)
{
	struct nolibc_heap *heap;

	/* Always allocate memory with size multiple of 4096. */
	len  = sizeof(*heap) + len;
	len  = (len + 4095UL) & -4096UL;
	heap = mmap(NULL, len, PROT_READ|PROT_WRITE, MAP_ANONYMOUS|MAP_PRIVATE,
		    -1, 0);
	if (__builtin_expect(heap == MAP_FAILED, 0))
		return NULL;

	heap->len = len;
	return heap->user_p;
}

static __attribute__((unused))
void *calloc(size_t size, size_t nmemb)
{
	size_t x = size * nmemb;

	if (__builtin_expect(size && ((x / size) != nmemb), 0)) {
		SET_ERRNO(ENOMEM);
		return NULL;
	}

	/*
	 * No need to zero the heap, the MAP_ANONYMOUS in malloc()
	 * already does it.
	 */
	return malloc(x);
}

static __attribute__((unused))
void *realloc(void *old_ptr, size_t new_size)
{
	struct nolibc_heap *heap;
	size_t user_p_len;
	void *ret;

	if (!old_ptr)
		return malloc(new_size);

	heap = container_of(old_ptr, struct nolibc_heap, user_p);
	user_p_len = heap->len - sizeof(*heap);
	/*
	 * Don't realloc() if @user_p_len >= @new_size, this block of
	 * memory is still enough to handle the @new_size. Just return
	 * the same pointer.
	 */
	if (user_p_len >= new_size)
		return old_ptr;

	ret = malloc(new_size);
	if (__builtin_expect(!ret, 0))
		return NULL;

	memcpy(ret, heap->user_p, user_p_len);
	munmap(heap, heap->len);
	return ret;
}

/* Converts the unsigned long integer <in> to its hex representation into
 * buffer <buffer>, which must be long enough to store the number and the
 * trailing zero (17 bytes for "ffffffffffffffff" or 9 for "ffffffff"). The
 * buffer is filled from the first byte, and the number of characters emitted
 * (not counting the trailing zero) is returned. The function is constructed
 * in a way to optimize the code size and avoid any divide that could add a
 * dependency on large external functions.
 */
static __attribute__((unused))
int utoh_r(unsigned long in, char *buffer)
{
	signed char pos = (~0UL > 0xfffffffful) ? 60 : 28;
	int digits = 0;
	int dig;

	do {
		dig = in >> pos;
		in -= (uint64_t)dig << pos;
		pos -= 4;
		if (dig || digits || pos < 0) {
			if (dig > 9)
				dig += 'a' - '0' - 10;
			buffer[digits++] = '0' + dig;
		}
	} while (pos >= 0);

	buffer[digits] = 0;
	return digits;
}

/* converts unsigned long <in> to an hex string using the static itoa_buffer
 * and returns the pointer to that string.
 */
static __inline__ __attribute__((unused))
char *utoh(unsigned long in)
{
	utoh_r(in, itoa_buffer);
	return itoa_buffer;
}

/* Converts the unsigned long integer <in> to its string representation into
 * buffer <buffer>, which must be long enough to store the number and the
 * trailing zero (21 bytes for 18446744073709551615 in 64-bit, 11 for
 * 4294967295 in 32-bit). The buffer is filled from the first byte, and the
 * number of characters emitted (not counting the trailing zero) is returned.
 * The function is constructed in a way to optimize the code size and avoid
 * any divide that could add a dependency on large external functions.
 */
static __attribute__((unused))
int utoa_r(unsigned long in, char *buffer)
{
	unsigned long lim;
	int digits = 0;
	int pos = (~0UL > 0xfffffffful) ? 19 : 9;
	int dig;

	do {
		for (dig = 0, lim = 1; dig < pos; dig++)
			lim *= 10;

		if (digits || in >= lim || !pos) {
			for (dig = 0; in >= lim; dig++)
				in -= lim;
			buffer[digits++] = '0' + dig;
		}
	} while (pos--);

	buffer[digits] = 0;
	return digits;
}

/* Converts the signed long integer <in> to its string representation into
 * buffer <buffer>, which must be long enough to store the number and the
 * trailing zero (21 bytes for -9223372036854775808 in 64-bit, 12 for
 * -2147483648 in 32-bit). The buffer is filled from the first byte, and the
 * number of characters emitted (not counting the trailing zero) is returned.
 */
static __attribute__((unused))
int itoa_r(long in, char *buffer)
{
	char *ptr = buffer;
	int len = 0;

	if (in < 0) {
		in = -(unsigned long)in;
		*(ptr++) = '-';
		len++;
	}
	len += utoa_r(in, ptr);
	return len;
}

/* for historical compatibility, same as above but returns the pointer to the
 * buffer.
 */
static __inline__ __attribute__((unused))
char *ltoa_r(long in, char *buffer)
{
	itoa_r(in, buffer);
	return buffer;
}

/* converts long integer <in> to a string using the static itoa_buffer and
 * returns the pointer to that string.
 */
static __inline__ __attribute__((unused))
char *itoa(long in)
{
	itoa_r(in, itoa_buffer);
	return itoa_buffer;
}

/* converts long integer <in> to a string using the static itoa_buffer and
 * returns the pointer to that string. Same as above, for compatibility.
 */
static __inline__ __attribute__((unused))
char *ltoa(long in)
{
	itoa_r(in, itoa_buffer);
	return itoa_buffer;
}

/* converts unsigned long integer <in> to a string using the static itoa_buffer
 * and returns the pointer to that string.
 */
static __inline__ __attribute__((unused))
char *utoa(unsigned long in)
{
	utoa_r(in, itoa_buffer);
	return itoa_buffer;
}

/* Converts the unsigned 64-bit integer <in> to its hex representation into
 * buffer <buffer>, which must be long enough to store the number and the
 * trailing zero (17 bytes for "ffffffffffffffff"). The buffer is filled from
 * the first byte, and the number of characters emitted (not counting the
 * trailing zero) is returned. The function is constructed in a way to optimize
 * the code size and avoid any divide that could add a dependency on large
 * external functions.
 */
static __attribute__((unused))
int u64toh_r(uint64_t in, char *buffer)
{
	signed char pos = 60;
	int digits = 0;
	int dig;

	do {
		if (sizeof(long) >= 8) {
			dig = (in >> pos) & 0xF;
		} else {
			/* 32-bit platforms: avoid a 64-bit shift */
			uint32_t d = (pos >= 32) ? (in >> 32) : in;
			dig = (d >> (pos & 31)) & 0xF;
		}
		if (dig > 9)
			dig += 'a' - '0' - 10;
		pos -= 4;
		if (dig || digits || pos < 0)
			buffer[digits++] = '0' + dig;
	} while (pos >= 0);

	buffer[digits] = 0;
	return digits;
}

/* converts uint64_t <in> to an hex string using the static itoa_buffer and
 * returns the pointer to that string.
 */
static __inline__ __attribute__((unused))
char *u64toh(uint64_t in)
{
	u64toh_r(in, itoa_buffer);
	return itoa_buffer;
}

/* Converts the unsigned 64-bit integer <in> to its string representation into
 * buffer <buffer>, which must be long enough to store the number and the
 * trailing zero (21 bytes for 18446744073709551615). The buffer is filled from
 * the first byte, and the number of characters emitted (not counting the
 * trailing zero) is returned. The function is constructed in a way to optimize
 * the code size and avoid any divide that could add a dependency on large
 * external functions.
 */
static __attribute__((unused))
int u64toa_r(uint64_t in, char *buffer)
{
	unsigned long long lim;
	int digits = 0;
	int pos = 19; /* start with the highest possible digit */
	int dig;

	do {
		for (dig = 0, lim = 1; dig < pos; dig++)
			lim *= 10;

		if (digits || in >= lim || !pos) {
			for (dig = 0; in >= lim; dig++)
				in -= lim;
			buffer[digits++] = '0' + dig;
		}
	} while (pos--);

	buffer[digits] = 0;
	return digits;
}

/* Converts the signed 64-bit integer <in> to its string representation into
 * buffer <buffer>, which must be long enough to store the number and the
 * trailing zero (21 bytes for -9223372036854775808). The buffer is filled from
 * the first byte, and the number of characters emitted (not counting the
 * trailing zero) is returned.
 */
static __attribute__((unused))
int i64toa_r(int64_t in, char *buffer)
{
	char *ptr = buffer;
	int len = 0;

	if (in < 0) {
		in = -(uint64_t)in;
		*(ptr++) = '-';
		len++;
	}
	len += u64toa_r(in, ptr);
	return len;
}

/* converts int64_t <in> to a string using the static itoa_buffer and returns
 * the pointer to that string.
 */
static __inline__ __attribute__((unused))
char *i64toa(int64_t in)
{
	i64toa_r(in, itoa_buffer);
	return itoa_buffer;
}

/* converts uint64_t <in> to a string using the static itoa_buffer and returns
 * the pointer to that string.
 */
static __inline__ __attribute__((unused))
char *u64toa(uint64_t in)
{
	u64toa_r(in, itoa_buffer);
	return itoa_buffer;
}

static __attribute__((unused))
uintmax_t __strtox(const char *nptr, char **endptr, int base, intmax_t lower_limit, uintmax_t upper_limit)
{
	const char signed_ = lower_limit != 0;
	unsigned char neg = 0, overflow = 0;
	uintmax_t val = 0, limit, old_val;
	char c;

	if (base < 0 || base > 36) {
		SET_ERRNO(EINVAL);
		goto out;
	}

	while (isspace(*nptr))
		nptr++;

	if (*nptr == '+') {
		nptr++;
	} else if (*nptr == '-') {
		neg = 1;
		nptr++;
	}

	if (signed_ && neg)
		limit = -(uintmax_t)lower_limit;
	else
		limit = upper_limit;

	if ((base == 0 || base == 16) &&
	    (strncmp(nptr, "0x", 2) == 0 || strncmp(nptr, "0X", 2) == 0)) {
		base = 16;
		nptr += 2;
	} else if (base == 0 && strncmp(nptr, "0", 1) == 0) {
		base = 8;
		nptr += 1;
	} else if (base == 0) {
		base = 10;
	}

	while (*nptr) {
		c = *nptr;

		if (c >= '0' && c <= '9')
			c -= '0';
		else if (c >= 'a' && c <= 'z')
			c = c - 'a' + 10;
		else if (c >= 'A' && c <= 'Z')
			c = c - 'A' + 10;
		else
			goto out;

		if (c >= base)
			goto out;

		nptr++;
		old_val = val;
		val *= base;
		val += c;

		if (val > limit || val < old_val)
			overflow = 1;
	}

out:
	if (overflow) {
		SET_ERRNO(ERANGE);
		val = limit;
	}
	if (endptr)
		*endptr = (char *)nptr;
	return neg ? -val : val;
}

static __attribute__((unused))
long strtol(const char *nptr, char **endptr, int base)
{
	return __strtox(nptr, endptr, base, LONG_MIN, LONG_MAX);
}

static __attribute__((unused))
unsigned long strtoul(const char *nptr, char **endptr, int base)
{
	return __strtox(nptr, endptr, base, 0, ULONG_MAX);
}

static __attribute__((unused))
long long strtoll(const char *nptr, char **endptr, int base)
{
	return __strtox(nptr, endptr, base, LLONG_MIN, LLONG_MAX);
}

static __attribute__((unused))
unsigned long long strtoull(const char *nptr, char **endptr, int base)
{
	return __strtox(nptr, endptr, base, 0, ULLONG_MAX);
}

static __attribute__((unused))
intmax_t strtoimax(const char *nptr, char **endptr, int base)
{
	return __strtox(nptr, endptr, base, INTMAX_MIN, INTMAX_MAX);
}

static __attribute__((unused))
uintmax_t strtoumax(const char *nptr, char **endptr, int base)
{
	return __strtox(nptr, endptr, base, 0, UINTMAX_MAX);
}

#endif /* _NOLIBC_STDLIB_H */
