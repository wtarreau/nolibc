/* SPDX-License-Identifier: LGPL-2.1 OR MIT */

#if defined(__x86_64__) || defined(__i386__) || defined(__i486__) || defined(__i586__) || defined(__i686__)
#include "../asm-arch-x86/_ASMFILE_"
#elif defined(__ARM_EABI__)
#include "../asm-arch-arm/_ASMFILE_"
#elif defined(__aarch64__)
#include "../asm-arch-arm64/_ASMFILE_"
#elif defined(__mips__)
#include "../asm-arch-mips/_ASMFILE_"
#elif defined(__powerpc__)
#include "../asm-arch-powerpc/_ASMFILE_"
#elif defined(__riscv)
#include "../asm-arch-riscv/_ASMFILE_"
#elif defined(__s390x__) || defined(__s390__)
#include "../asm-arch-s390/_ASMFILE_"
#elif defined(__loongarch__)
#include "../asm-arch-loongarch/_ASMFILE_"
#elif defined(__sparc__)
#include "../asm-arch-sparc/_ASMFILE_"
#elif defined(__m68k__)
#include "../asm-arch-m68k/_ASMFILE_"
#elif defined(__sh__)
#include "../asm-arch-sh/_ASMFILE_"
#else
#error Unsupported Architecture
#endif
