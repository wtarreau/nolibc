/*
 * ctype function definitions for NOLIBC
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

#ifndef _NOLIBC_CTYPE_H
#define _NOLIBC_CTYPE_H

#include "std.h"

/*
 * As much as possible, please keep functions alphabetically sorted.
 */

static __attribute__((unused))
int isascii(int c)
{
	/* 0x00..0x7f */
	return (unsigned int)c <= 0x7f;
}

static __attribute__((unused))
int isblank(int c)
{
	return c == '\t' || c == ' ';
}

static __attribute__((unused))
int iscntrl(int c)
{
	/* 0x00..0x1f, 0x7f */
	return (unsigned int)c < 0x20 || c == 0x7f;
}

static __attribute__((unused))
int isdigit(int c)
{
	return (unsigned int)(c - '0') < 10;
}

static __attribute__((unused))
int isgraph(int c)
{
	/* 0x21..0x7e */
	return (unsigned int)(c - 0x21) < 0x5e;
}

static __attribute__((unused))
int islower(int c)
{
	return (unsigned int)(c - 'a') < 26;
}

static __attribute__((unused))
int isprint(int c)
{
	/* 0x20..0x7e */
	return (unsigned int)(c - 0x20) < 0x5f;
}

static __attribute__((unused))
int isspace(int c)
{
	/* \t is 0x9, \n is 0xA, \v is 0xB, \f is 0xC, \r is 0xD */
	return ((unsigned int)c == ' ') || (unsigned int)(c - 0x09) < 5;
}

static __attribute__((unused))
int isupper(int c)
{
	return (unsigned int)(c - 'A') < 26;
}

static __attribute__((unused))
int isxdigit(int c)
{
	return isdigit(c) || (unsigned int)(c - 'A') < 6 || (unsigned int)(c - 'a') < 6;
}

static __attribute__((unused))
int isalpha(int c)
{
	return islower(c) || isupper(c);
}

static __attribute__((unused))
int isalnum(int c)
{
	return isalpha(c) || isdigit(c);
}

static __attribute__((unused))
int ispunct(int c)
{
	return isgraph(c) && !isalnum(c);
}

#endif /* _NOLIBC_CTYPE_H */
