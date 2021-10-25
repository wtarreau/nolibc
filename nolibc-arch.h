/*
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

/* Below comes the architecture-specific code. For each architecture, we have
 * the syscall declarations and the _start code definition. This is the only
 * global part. On all architectures the kernel puts everything in the stack
 * before jumping to _start just above us, without any return address (_start
 * is not a function but an entry pint). So at the stack pointer we find argc.
 * Then argv[] begins, and ends at the first NULL. Then we have envp which
 * starts and ends with a NULL as well. So envp=argv+argc+1.
 */

#ifndef _NOLIBC_ARCH_H
#define _NOLIBC_ARCH_H

#if defined(__x86_64__)
#include "nolibc-x86_64.h"
#elif defined(__i386__) || defined(__i486__) || defined(__i586__) || defined(__i686__)
#include "nolibc-i386.h"
#elif defined(__ARM_EABI__)
#include "nolibc-arm.h"
#elif defined(__aarch64__)
#include "nolibc-aarch64.h"
#elif defined(__mips__) && defined(_ABIO32)
#include "nolibc-mips.h"
#elif defined(__riscv)
#include "nolibc-riscv.h"
#endif

#endif /* _NOLIBC_ARCH_H */
