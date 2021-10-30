/*
 * minimal stdio function definitions for NOLIBC
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

#ifndef _NOLIBC_STDIO_H
#define _NOLIBC_STDIO_H

#include "std.h"
#include "arch.h"
#include "types.h"
#include "sys.h"
#include "stdlib.h"
#include "string.h"

#ifndef EOF
#define EOF (-1)
#endif

static __attribute__((unused))
int getchar(void)
{
	unsigned char ch;

	if (read(0, &ch, 1) <= 0)
		return EOF;
	return ch;
}

static __attribute__((unused))
int putchar(int c)
{
	unsigned char ch = c;

	if (write(1, &ch, 1) <= 0)
		return EOF;
	return ch;
}

static __attribute__((unused))
int puts(const char *s)
{
	size_t len = strlen(s);
	ssize_t ret;

	while (len > 0) {
		ret = write(1, s, len);
		if (ret <= 0)
			return EOF;
		s += ret;
		len -= ret;
	}
	return putchar('\n');
}

#endif /* _NOLIBC_STDIO_H */
