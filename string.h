/*
 * string function definitions for NOLIBC
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

#ifndef _NOLIBC_STRING_H
#define _NOLIBC_STRING_H

#include "std.h"

/*
 * As much as possible, please keep functions alphabetically sorted.
 */

static __attribute__((unused))
int memcmp(const void *s1, const void *s2, size_t n)
{
	size_t ofs = 0;
	char c1 = 0;

	while (ofs < n && !(c1 = ((char *)s1)[ofs] - ((char *)s2)[ofs])) {
		ofs++;
	}
	return c1;
}

static __attribute__((unused))
void *_nolibc_memcpy_up(void *dst, const void *src, size_t len)
{
	size_t pos = 0;

	while (pos < len) {
		((char *)dst)[pos] = ((const char *)src)[pos];
		pos++;
	}
	return dst;
}

static __attribute__((unused))
void *_nolibc_memcpy_down(void *dst, const void *src, size_t len)
{
	while (len) {
		len--;
		((char *)dst)[len] = ((const char *)src)[len];
	}
	return dst;
}

/* might be ignored by the compiler without -ffreestanding, then found as
 * missing.
 */
__attribute__((weak,unused,section(".text.nolibc_memmove")))
void *memmove(void *dst, const void *src, size_t len)
{
	size_t dir, pos;

	pos = len;
	dir = -1;

	if (dst < src) {
		pos = -1;
		dir = 1;
	}

	while (len) {
		pos += dir;
		((char *)dst)[pos] = ((const char *)src)[pos];
		len--;
	}
	return dst;
}

/* must be exported, as it's used by libgcc on ARM */
__attribute__((weak,unused,section(".text.nolibc_memcpy")))
void *memcpy(void *dst, const void *src, size_t len)
{
	return _nolibc_memcpy_up(dst, src, len);
}

/* might be ignored by the compiler without -ffreestanding, then found as
 * missing.
 */
__attribute__((weak,unused,section(".text.nolibc_memset")))
void *memset(void *dst, int b, size_t len)
{
	char *p = dst;

	while (len--)
		*(p++) = b;
	return dst;
}

static __attribute__((unused))
char *strchr(const char *s, int c)
{
	while (*s) {
		if (*s == (char)c)
			return (char *)s;
		s++;
	}
	return NULL;
}

static __attribute__((unused))
char *strcpy(char *dst, const char *src)
{
	char *ret = dst;

	while ((*dst++ = *src++));
	return ret;
}

/* this function is only used with arguments that are not constants */
static __attribute__((unused))
size_t nolibc_strlen(const char *str)
{
	size_t len;

	for (len = 0; str[len]; len++);
	return len;
}

#define strlen(str) ({                          \
	__builtin_constant_p((str)) ?           \
		__builtin_strlen((str)) :       \
		nolibc_strlen((str));           \
})

static __attribute__((unused))
size_t strlcat(char *dst, const char *src, size_t size)
{
	size_t len;
	char c;

	for (len = 0; dst[len];	len++)
		;

	for (;;) {
		c = *src;
		if (len < size)
			dst[len] = c;
		if (!c)
			break;
		len++;
		src++;
	}

	return len;
}

static __attribute__((unused))
size_t strlcpy(char *dst, const char *src, size_t size)
{
	size_t len;
	char c;

	for (len = 0;;) {
		c = src[len];
		if (len < size)
			dst[len] = c;
		if (!c)
			break;
		len++;
	}
	return len;
}

static __attribute__((unused))
char *strncat(char *dst, const char *src, size_t size)
{
	char *orig = dst;

	while (*dst)
		dst++;

	while (size && (*dst = *src)) {
		src++;
		dst++;
		size--;
	}

	*dst = 0;
	return orig;
}


static __attribute__((unused))
char *strncpy(char *dst, const char *src, size_t size)
{
	size_t len;

	for (len = 0; len < size; len++)
		if ((dst[len] = *src))
			src++;
	return dst;
}

static __attribute__((unused))
char *strrchr(const char *s, int c)
{
	const char *ret = NULL;

	while (*s) {
		if (*s == (char)c)
			ret = s;
		s++;
	}
	return (char *)ret;
}

#endif /* _NOLIBC_STRING_H */
