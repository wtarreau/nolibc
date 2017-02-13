/* some archs (at least aarch64) don't expose the regular syscalls anymore by
 * default, either because they have an "_at" replacement, or because there are
 * more modern alternatives. For now we'd rather still use them.
 */
#define __ARCH_WANT_SYSCALL_NO_AT
#define __ARCH_WANT_SYSCALL_NO_FLAGS
#define __ARCH_WANT_SYSCALL_DEPRECATED

#include <asm/unistd.h>
#include <asm/ioctls.h>
#include <asm/errno.h>
#include <linux/fs.h>
#include <linux/loop.h>

#define NOLIBC

/* Build a static executable this way :
 *      $ gcc -fno-asynchronous-unwind-tables -s -Os -nostdlib -static \
 *            -include nolibc.h -lgcc -o hello hello.c
 *      $ strip -R .comment init-nolib
 *
 * Useful calling convention table found here :
 *      http://man7.org/linux/man-pages/man2/syscall.2.html
 *
 * This doc is even better :
 *      https://w3challs.com/syscalls/
 *
 * Architecture calling conventions
 * Every architecture has its own way of invoking and passing  arguments  to  the
 * kernel.   The  details  for various architectures are listed in the two tables
 * below.
 *
 * The first table lists the instruction  used  to  transition  to  kernel  mode,
 * (which  might  not  be the fastest or best way to transition to the kernel, so
 * you might have to refer to the VDSO), the register used to indicate the system
 * call number, and the register used to return the system call result.
 *
 * arch/ABI    instruction           syscall #  retval  error    Notes
 * --------------------------------------------------------------------
 * alpha       callsys               v0         a0      a3       [1]
 * arc         trap0                 r8         r0      -
 * arm/OABI    swi NR                -          a1      -        [2]
 * arm/EABI    swi 0x0               r7         r0      -
 * arm64       svc #0                x8         x0      -
 * blackfin    excpt 0x0             P0         R0      -
 * i386        int $0x80             eax        eax     -
 * ia64        break 0x100000        r15        r8      r10      [1]
 * m68k        trap #0               d0         d0      -
 * microblaze  brki r14,8            r12        r3      -
 * mips        syscall               v0         v0      a3       [1]
 * nios2       trap                  r2         r2      r7
 * parisc      ble 0x100(%sr2, %r0)  r20        r28     -
 * powerpc     sc                    r0         r3      r0       [1]
 * s390        svc 0                 r1         r2      -        [3]
 * s390x       svc 0                 r1         r2      -        [3]
 * superh      trap #0x17            r3         r0      -        [4]
 * sparc/32    t 0x10                g1         o0      psr/csr  [1]
 * sparc/64    t 0x6d                g1         o0      psr/csr  [1]
 * tile        swint1                R10        R00     R01      [1]
 * x86_64      syscall               rax        rax     -        [5]
 * x32         syscall               rax        rax     -        [5]
 * xtensa      syscall               a2         a2      -
 *
 * [1] On a few architectures, a register is used as a boolean (0
 *     indicating no error, and -1 indicating an error) to signal that
 *     the system call failed.  The actual error value is still
 *     contained in the return register.  On sparc, the carry bit
 *     (csr) in the processor status register (psr) is used instead of
 *     a full register.
 *
 *
 * Registers used :
 *
 * arch/ABI      arg1  arg2  arg3  arg4  arg5  arg6  arg7  Notes
 * --------------------------------------------------------------
 * alpha         a0    a1    a2    a3    a4    a5    -
 * arc           r0    r1    r2    r3    r4    r5    -
 * arm/OABI      a1    a2    a3    a4    v1    v2    v3
 * arm/EABI      r0    r1    r2    r3    r4    r5    r6
 * arm64         x0    x1    x2    x3    x4    x5    -
 * blackfin      R0    R1    R2    R3    R4    R5    -
 * i386          ebx   ecx   edx   esi   edi   ebp   -
 * ia64          out0  out1  out2  out3  out4  out5  -
 * m68k          d1    d2    d3    d4    d5    a0    -
 * microblaze    r5    r6    r7    r8    r9    r10   -
 * mips/o32      a0    a1    a2    a3    -     -     -     [1]
 * mips/n32,64   a0    a1    a2    a3    a4    a5    -
 * nios2         r4    r5    r6    r7    r8    r9    -
 * parisc        r26   r25   r24   r23   r22   r21   -
 * powerpc       r3    r4    r5    r6    r7    r8    r9
 * s390          r2    r3    r4    r5    r6    r7    -
 * s390x         r2    r3    r4    r5    r6    r7    -
 * superh        r4    r5    r6    r7    r0    r1    r2
 * sparc/32      o0    o1    o2    o3    o4    o5    -
 * sparc/64      o0    o1    o2    o3    o4    o5    -
 * tile          R00   R01   R02   R03   R04   R05   -
 * x86_64        rdi   rsi   rdx   r10   r8    r9    -
 * x32           rdi   rsi   rdx   r10   r8    r9    -
 * xtensa        a6    a3    a4    a5    a8    a9    -
 *
 *  [1] The mips/o32 system call convention passes arguments 5 through 8 on the
 *      user stack.
 */


/* this way it will be removed if unused */
static int errno;

#ifndef NOLIBC_IGNORE_ERRNO
#define SET_ERRNO(v) do { errno = (v); } while (0)
#else
#define SET_ERRNO(v) do { } while (0)
#endif

/* errno codes all ensure that they will not conflict with a valid pointer
 * because they all correspond to the highest addressable memry page.
 */
#define MAX_ERRNO 4095

/* Declare a few quite common macros and types that usually are in stdlib.h,
 * stdint.h, ctype.h, unistd.h and a few other common locations.
 */

#define NULL ((void *)0)

/* stdint types */
typedef unsigned char       uint8_t;
typedef   signed char        int8_t;
typedef unsigned short     uint16_t;
typedef   signed short      int16_t;
typedef unsigned int       uint32_t;
typedef   signed int        int32_t;
typedef unsigned long long uint64_t;
typedef   signed long long  int64_t;
typedef unsigned long        size_t;
typedef   signed long       ssize_t;
typedef unsigned long     uintptr_t;
typedef   signed long      intptr_t;
typedef   signed long     ptrdiff_t;

/* for stat() */
typedef unsigned int          dev_t;
typedef unsigned long         ino_t;
typedef unsigned int         mode_t;
typedef   signed int          pid_t;
typedef unsigned int          uid_t;
typedef unsigned int          gid_t;
typedef unsigned long       nlink_t;
typedef   signed long         off_t;
typedef   signed long     blksize_t;
typedef   signed long      blkcnt_t;
typedef   signed long        time_t;

/* for poll() */
struct pollfd {
    int fd;
    short int events;
    short int revents;
};

/* for select() */
struct timeval {
	long    tv_sec;
	long    tv_usec;
};

/* for getdents64() */
struct linux_dirent64 {
	uint64_t       d_ino;
	int64_t        d_off;
	unsigned short d_reclen;
	unsigned char  d_type;
	char           d_name[];
};

/* commonly an fd_set represents 256 FDs */
#define FD_SETSIZE 256
typedef struct { uint32_t fd32[FD_SETSIZE/32]; } fd_set;

/* needed by wait4() */
struct rusage {
	struct timeval ru_utime;
	struct timeval ru_stime;
	long   ru_maxrss;
	long   ru_ixrss;
	long   ru_idrss;
	long   ru_isrss;
	long   ru_minflt;
	long   ru_majflt;
	long   ru_nswap;
	long   ru_inblock;
	long   ru_oublock;
	long   ru_msgsnd;
	long   ru_msgrcv;
	long   ru_nsignals;
	long   ru_nvcsw;
	long   ru_nivcsw;
};

/* stat flags (WARNING, octal here) */
#define S_IFDIR       0040000
#define S_IFCHR       0020000
#define S_IFBLK       0060000
#define S_IFREG       0100000
#define S_IFIFO       0010000
#define S_IFLNK       0120000
#define S_IFSOCK      0140000
#define S_IFMT        0170000

#define S_ISDIR(mode)  (((mode) & S_IFDIR) == S_IFDIR)
#define S_ISCHR(mode)  (((mode) & S_IFCHR) == S_IFCHR)
#define S_ISBLK(mode)  (((mode) & S_IFBLK) == S_IFBLK)
#define S_ISREG(mode)  (((mode) & S_IFREG) == S_IFREG)
#define S_ISFIFO(mode) (((mode) & S_IFIFO) == S_IFIFO)
#define S_ISLNK(mode)  (((mode) & S_IFLNK) == S_IFLNK)
#define S_ISSOCK(mode) (((mode) & S_IFSOCK) == S_IFSOCK)

/* fcntl / open */
#define O_RDONLY            0
#define O_WRONLY            1
#define O_RDWR              2
#define O_CREAT          0x40
#define O_EXCL           0x80
#define O_NOCTTY        0x100
#define O_TRUNC         0x200
#define O_APPEND        0x400
#define O_NONBLOCK      0x800
#define O_DIRECTORY   0x10000

#define DT_UNKNOWN 0
#define DT_FIFO    1
#define DT_CHR     2
#define DT_DIR     4
#define DT_BLK     6
#define DT_REG     8
#define DT_LNK    10
#define DT_SOCK   12

/* lseek */
#define SEEK_SET        0
#define SEEK_CUR        1
#define SEEK_END        2

struct stat {
	dev_t     st_dev;     /* ID of device containing file */
	ino_t     st_ino;     /* inode number */
	mode_t    st_mode;    /* protection */
	nlink_t   st_nlink;   /* number of hard links */
	uid_t     st_uid;     /* user ID of owner */
	gid_t     st_gid;     /* group ID of owner */
	dev_t     st_rdev;    /* device ID (if special file) */
	off_t     st_size;    /* total size, in bytes */
	blksize_t st_blksize; /* blocksize for file system I/O */
	blkcnt_t  st_blocks;  /* number of 512B blocks allocated */
	time_t    st_atime;   /* time of last access */
	time_t    st_mtime;   /* time of last modification */
	time_t    st_ctime;   /* time of last status change */
};

#define WEXITSTATUS(status)   (((status) & 0xff00) >> 8)
#define WIFEXITED(status)     (((status) & 0x7f) == 0)


/* Below comes the architecture-specific code. For each architecture, we have
 * the syscall declarations and the _start code definition. This is the only
 * global part. On all architectures the kernel puts everything in the stack
 * before jumping to _start just above us, without any return address (_start
 * is not a function but an entry pint). So at the stack pointer we find argc.
 * Then argv[] begins, and ends at the first NULL. Then we have envp which
 * starts and ends with a NULL as well. So envp=argv+argc+1.
 */

#if defined(__x86_64__)
/* Syscalls for x86_64 :
 *   - registers are 64-bit
 *   - syscall number is passed in rax
 *   - arguments are in rdi, rsi, rdx, r10, r8, r9 respectively
 *   - the system call is performed by calling the syscall instruction
 *   - syscall return comes in rax
 *   - rcx and r8..r11 may be clobbered, others are preserved.
 *   - the arguments are cast to long and assigned into the target registers
 *     which are then simply passed as registers to the asm code, so that we
 *     don't have to experience issues with register contraints.
 *   - the syscall number is always specified last in order to allow to force
 *     some registers before (gcc refuses a %-register at the last position).
 */

#define my_syscall0(num)                                                      \
({                                                                            \
	long _ret;                                                            \
	register long _num  asm("rax") = (num);                               \
                                                                              \
	asm volatile (                                                        \
		"syscall\n"                                                   \
		: "=a" (_ret)                                                 \
		: "0"(_num)                                                   \
		: "rcx", "r8", "r9", "r10", "r11", "memory", "cc"                                \
	);                                                                    \
	_ret;                                                                 \
})

#define my_syscall1(num, arg1)                                                \
({                                                                            \
	long _ret;                                                            \
	register long _num  asm("rax") = (num);                               \
	register long _arg1 asm("rdi") = (long)(arg1);                        \
                                                                              \
	asm volatile (                                                        \
		"syscall\n"                                                   \
		: "=a" (_ret)                                                 \
		: "r"(_arg1),                                                 \
		  "0"(_num)                                                   \
		: "rcx", "r8", "r9", "r10", "r11", "memory", "cc"                                \
	);                                                                    \
	_ret;                                                                 \
})

#define my_syscall2(num, arg1, arg2)                                          \
({                                                                            \
	long _ret;                                                            \
	register long _num  asm("rax") = (num);                               \
	register long _arg1 asm("rdi") = (long)(arg1);                        \
	register long _arg2 asm("rsi") = (long)(arg2);                        \
                                                                              \
	asm volatile (                                                        \
		"syscall\n"                                                   \
		: "=a" (_ret)                                                 \
		: "r"(_arg1), "r"(_arg2),                                     \
		  "0"(_num)                                                   \
		: "rcx", "r8", "r9", "r10", "r11", "memory", "cc"                                \
	);                                                                    \
	_ret;                                                                 \
})

#define my_syscall3(num, arg1, arg2, arg3)                                    \
({                                                                            \
	long _ret;                                                            \
	register long _num  asm("rax") = (num);                               \
	register long _arg1 asm("rdi") = (long)(arg1);                        \
	register long _arg2 asm("rsi") = (long)(arg2);                        \
	register long _arg3 asm("rdx") = (long)(arg3);                        \
                                                                              \
	asm volatile (                                                        \
		"syscall\n"                                                   \
		: "=a" (_ret)                                                 \
		: "r"(_arg1), "r"(_arg2), "r"(_arg3),                         \
		  "0"(_num)                                                   \
		: "rcx", "r8", "r9", "r10", "r11", "memory", "cc"                                \
	);                                                                    \
	_ret;                                                                 \
})

#define my_syscall4(num, arg1, arg2, arg3, arg4)                              \
({                                                                            \
	long _ret;                                                            \
	register long _num  asm("rax") = (num);                               \
	register long _arg1 asm("rdi") = (long)(arg1);                        \
	register long _arg2 asm("rsi") = (long)(arg2);                        \
	register long _arg3 asm("rdx") = (long)(arg3);                        \
	register long _arg4 asm("r10") = (long)(arg4);                        \
                                                                              \
	asm volatile (                                                        \
		"syscall\n"                                                   \
		: "=a" (_ret), "=r"(_arg4)                                    \
		: "r"(_arg1), "r"(_arg2), "r"(_arg3), "r"(_arg4),             \
		  "0"(_num)                                                   \
		: "rcx", "r8", "r9", "r11", "memory", "cc"                    \
	);                                                                    \
	_ret;                                                                 \
})

#define my_syscall5(num, arg1, arg2, arg3, arg4, arg5)                        \
({                                                                            \
	long _ret;                                                            \
	register long _num  asm("rax") = (num);                               \
	register long _arg1 asm("rdi") = (long)(arg1);                        \
	register long _arg2 asm("rsi") = (long)(arg2);                        \
	register long _arg3 asm("rdx") = (long)(arg3);                        \
	register long _arg4 asm("r10") = (long)(arg4);                        \
	register long _arg5 asm("r8")  = (long)(arg5);                        \
                                                                              \
	asm volatile (                                                        \
		"syscall\n"                                                   \
		: "=a" (_ret), "=r"(_arg4), "=r"(_arg5)                       \
		: "r"(_arg1), "r"(_arg2), "r"(_arg3), "r"(_arg4), "r"(_arg5), \
		  "0"(_num)                                                   \
		: "rcx", "r9", "r11", "memory", "cc"                          \
	);                                                                    \
	_ret;                                                                 \
})

/* startup code */
asm(".section .text\n"
    ".global _start\n"
    "_start:\n"
    "pop %rdi\n"                // argc   (first arg, %rdi)
    "mov %rsp, %rsi\n"          // argv[] (second arg, %rsi)
    "lea 8(%rsi,%rdi,8),%rdx\n" // then a NULL then envp (third arg, %rdx)
    "and $-16, %rsp\n"          // x86 ABI : esp must be 16-byte aligned when
    "sub $8, %rsp\n"            // entering the callee
    "call main\n"               // main() returns the status code, we'll exit with it.
    "movzb %al, %rdi\n"         // retrieve exit code from 8 lower bits
    "mov $60, %rax\n"           // NR_exit == 60
    "syscall\n"                 // really exit
    "hlt\n"                     // ensure it does not return
    "");

#elif defined(__i386__) || defined(__i486__) || defined(__i586__) || defined(__i686__)
/* Syscalls for i386 :
 *   - mostly similar to x86_64
 *   - registers are 32-bit
 *   - syscall number is passed in eax
 *   - arguments are in ebx, ecx, edx, esi, edi, ebp respectively
 *   - all registers are preserved (except eax of course)
 *   - the system call is performed by calling int $0x80
 *   - syscall return comes in eax
 *   - the arguments are cast to long and assigned into the target registers
 *     which are then simply passed as registers to the asm code, so that we
 *     don't have to experience issues with register contraints.
 *   - the syscall number is always specified last in order to allow to force
 *     some registers before (gcc refuses a %-register at the last position).
 */

#define my_syscall0(num)                                                      \
({                                                                            \
	long _ret;                                                            \
	register long _num asm("eax") = (num);                                \
                                                                              \
	asm volatile (                                                        \
		"int $0x80\n"                                                 \
		: "=a" (_ret)                                                 \
		: "0"(_num)                                                   \
		: "memory", "cc"                                              \
	);                                                                    \
	_ret;                                                                 \
})

#define my_syscall1(num, arg1)                                                \
({                                                                            \
	long _ret;                                                            \
	register long _num asm("eax") = (num);                                \
	register long _arg1 asm("ebx") = (long)(arg1);                        \
                                                                              \
	asm volatile (                                                        \
		"int $0x80\n"                                                 \
		: "=a" (_ret)                                                 \
		: "r"(_arg1),                                                 \
		  "0"(_num)                                                   \
		: "memory", "cc"                                              \
	);                                                                    \
	_ret;                                                                 \
})

#define my_syscall2(num, arg1, arg2)                                          \
({                                                                            \
	long _ret;                                                            \
	register long _num asm("eax") = (num);                                \
	register long _arg1 asm("ebx") = (long)(arg1);                        \
	register long _arg2 asm("ecx") = (long)(arg2);                        \
                                                                              \
	asm volatile (                                                        \
		"int $0x80\n"                                                 \
		: "=a" (_ret)                                                 \
		: "r"(_arg1), "r"(_arg2),                                     \
		  "0"(_num)                                                   \
		: "memory", "cc"                                              \
	);                                                                    \
	_ret;                                                                 \
})

#define my_syscall3(num, arg1, arg2, arg3)                                    \
({                                                                            \
	long _ret;                                                            \
	register long _num asm("eax") = (num);                                \
	register long _arg1 asm("ebx") = (long)(arg1);                        \
	register long _arg2 asm("ecx") = (long)(arg2);                        \
	register long _arg3 asm("edx") = (long)(arg3);                        \
                                                                              \
	asm volatile (                                                        \
		"int $0x80\n"                                                 \
		: "=a" (_ret)                                                 \
		: "r"(_arg1), "r"(_arg2), "r"(_arg3),                         \
		  "0"(_num)                                                   \
		: "memory", "cc"                                              \
	);                                                                    \
	_ret;                                                                 \
})

#define my_syscall4(num, arg1, arg2, arg3, arg4)                              \
({                                                                            \
	long _ret;                                                            \
	register long _num asm("eax") = (num);                                \
	register long _arg1 asm("ebx") = (long)(arg1);                        \
	register long _arg2 asm("ecx") = (long)(arg2);                        \
	register long _arg3 asm("edx") = (long)(arg3);                        \
	register long _arg4 asm("esi") = (long)(arg4);                        \
                                                                              \
	asm volatile (                                                        \
		"int $0x80\n"                                                 \
		: "=a" (_ret)                                                 \
		: "r"(_arg1), "r"(_arg2), "r"(_arg3), "r"(_arg4),             \
		  "0"(_num)                                                   \
		: "memory", "cc"                                              \
	);                                                                    \
	_ret;                                                                 \
})

#define my_syscall5(num, arg1, arg2, arg3, arg4, arg5)                        \
({                                                                            \
	long _ret;                                                            \
	register long _num asm("eax") = (num);                                \
	register long _arg1 asm("ebx") = (long)(arg1);                        \
	register long _arg2 asm("ecx") = (long)(arg2);                        \
	register long _arg3 asm("edx") = (long)(arg3);                        \
	register long _arg4 asm("esi") = (long)(arg4);                        \
	register long _arg5 asm("edi") = (long)(arg5);                        \
                                                                              \
	asm volatile (                                                        \
		"int $0x80\n"                                                 \
		: "=a" (_ret)                                                 \
		: "r"(_arg1), "r"(_arg2), "r"(_arg3), "r"(_arg4), "r"(_arg5), \
		  "0"(_num)                                                   \
		: "memory", "cc"                                              \
	);                                                                    \
	_ret;                                                                 \
})

/* startup code */
asm(".section .text\n"
    ".global _start\n"
    "_start:\n"
    "pop %eax\n"                // argc   (first arg, %eax)
    "mov %esp, %ebx\n"          // argv[] (second arg, %ebx)
    "lea 4(%ebx,%eax,4),%ecx\n" // then a NULL then envp (third arg, %ecx)
    "and $-16, %esp\n"          // x86 ABI : esp must be 16-byte aligned when
    "push %ecx\n"               // push all registers on the stack so that we
    "push %ebx\n"               // support both regparm and plain stack modes
    "push %eax\n"
    "call main\n"               // main() returns the status code in %eax
    "movzbl %al, %ebx\n"        // retrieve exit code from lower 8 bits
    "movl   $1, %eax\n"         // NR_exit == 1
    "int    $0x80\n"            // exit now
    "hlt\n"                     // ensure it does not
    "");

#elif defined(__ARM_EABI__)
/* Syscalls for ARM in ARM or Thumb modes :
 *   - registers are 32-bit
 *   - stack is 8-byte aligned
 *     ( http://infocenter.arm.com/help/index.jsp?topic=/com.arm.doc.faqs/ka4127.html)
 *   - syscall number is passed in r7
 *   - arguments are in r0, r1, r2, r3, r4, r5
 *   - the system call is performed by calling svc #0
 *   - syscall return comes in r0.
 *   - only lr is clobbered.
 *   - the arguments are cast to long and assigned into the target registers
 *     which are then simply passed as registers to the asm code, so that we
 *     don't have to experience issues with register contraints.
 *   - the syscall number is always specified last in order to allow to force
 *     some registers before (gcc refuses a %-register at the last position).
 */

#define my_syscall0(num)                                                      \
({                                                                            \
	register long _num asm("r7") = (num);                                 \
	register long _arg1 asm("r0");                                        \
                                                                              \
	asm volatile (                                                        \
		"svc #0\n"                                                    \
		: "=r"(_arg1)                                                 \
		: "r"(_num)                                                   \
		: "memory", "cc", "lr"                                        \
	);                                                                    \
	_arg1;                                                                \
})

#define my_syscall1(num, arg1)                                                \
({                                                                            \
	register long _num asm("r7") = (num);                                 \
	register long _arg1 asm("r0") = (long)(arg1);                         \
                                                                              \
	asm volatile (                                                        \
		"svc #0\n"                                                    \
		: "=r"(_arg1)                                                 \
		: "r"(_arg1),                                                 \
		  "r"(_num)                                                   \
		: "memory", "cc", "lr"                                        \
	);                                                                    \
	_arg1;                                                                \
})

#define my_syscall2(num, arg1, arg2)                                          \
({                                                                            \
	register long _num asm("r7") = (num);                                 \
	register long _arg1 asm("r0") = (long)(arg1);                         \
	register long _arg2 asm("r1") = (long)(arg2);                         \
                                                                              \
	asm volatile (                                                        \
		"svc #0\n"                                                    \
		: "=r"(_arg1)                                                 \
		: "r"(_arg1), "r"(_arg2),                                     \
		  "r"(_num)                                                   \
		: "memory", "cc", "lr"                                        \
	);                                                                    \
	_arg1;                                                                \
})

#define my_syscall3(num, arg1, arg2, arg3)                                    \
({                                                                            \
	register long _num asm("r7") = (num);                                 \
	register long _arg1 asm("r0") = (long)(arg1);                         \
	register long _arg2 asm("r1") = (long)(arg2);                         \
	register long _arg3 asm("r2") = (long)(arg3);                         \
                                                                              \
	asm volatile (                                                        \
		"svc #0\n"                                                    \
		: "=r"(_arg1)                                                 \
		: "r"(_arg1), "r"(_arg2), "r"(_arg3),                         \
		  "r"(_num)                                                   \
		: "memory", "cc", "lr"                                        \
	);                                                                    \
	_arg1;                                                                \
})

#define my_syscall4(num, arg1, arg2, arg3, arg4)                              \
({                                                                            \
	register long _num asm("r7") = (num);                                 \
	register long _arg1 asm("r0") = (long)(arg1);                         \
	register long _arg2 asm("r1") = (long)(arg2);                         \
	register long _arg3 asm("r2") = (long)(arg3);                         \
	register long _arg4 asm("r3") = (long)(arg4);                         \
                                                                              \
	asm volatile (                                                        \
		"svc #0\n"                                                    \
		: "=r"(_arg1)                                                 \
		: "r"(_arg1), "r"(_arg2), "r"(_arg3), "r"(_arg4),             \
		  "r"(_num)                                                   \
		: "memory", "cc", "lr"                                        \
	);                                                                    \
	_arg1;                                                                \
})

#define my_syscall5(num, arg1, arg2, arg3, arg4, arg5)                        \
({                                                                            \
	register long _num asm("r7") = (num);                                 \
	register long _arg1 asm("r0") = (long)(arg1);                         \
	register long _arg2 asm("r1") = (long)(arg2);                         \
	register long _arg3 asm("r2") = (long)(arg3);                         \
	register long _arg4 asm("r3") = (long)(arg4);                         \
	register long _arg5 asm("r4") = (long)(arg5);                         \
                                                                              \
	asm volatile (                                                        \
		"svc #0\n"                                                    \
		: "=r" (_arg1)                                                \
		: "r"(_arg1), "r"(_arg2), "r"(_arg3), "r"(_arg4), "r"(_arg5), \
		  "r"(_num)                                                   \
		: "memory", "cc", "lr"                                        \
	);                                                                    \
	_arg1;                                                                \
})

/* startup code */
asm(".section .text\n"
    ".global _start\n"
    "_start:\n"
#if defined(__THUMBEB__) || defined(__THUMBEL__)
    /* We enter here in 32-bit mode but if some previous functions were in
     * 16-bit mode, the assembler cannot know, so we need to tell it we're in
     * 32-bit now, then switch to 16-bit (is there a better way to do it than
     * adding 1 by hand ?) and tell the asm we're now in 16-bit mode so that
     * it generates correct instructions. Note that we do not support thumb1.
     */
    ".code 32\n"
    "add     r0, pc, #1\n"
    "bx      r0\n"
    ".code 16\n"
#endif
    "pop {%r0}\n"                 // argc was in the stack
    "mov %r1, %sp\n"              // argv = sp
    "add %r2, %r1, %r0, lsl #2\n" // envp = argv + 4*argc ...
    "add %r2, %r2, $4\n"          //        ... + 4
    "and %r3, %r1, $-8\n"         // AAPCS : sp must be 8-byte aligned in the
    "mov %sp, %r3\n"              //         callee, an bl doesn't push (lr=pc)
    "bl main\n"                   // main() returns the status code, we'll exit with it.
    "and %r0, %r0, $0xff\n"       // limit exit code to 8 bits
    "movs r7, $1\n"               // NR_exit == 1
    "svc $0x00\n"
    "");

#elif defined(__aarch64__)
/* Syscalls for AARCH64 :
 *   - registers are 64-bit
 *   - stack is 16-byte aligned
 *   - syscall number is passed in x8
 *   - arguments are in x0, x1, x2, x3, x4, x5
 *   - the system call is performed by calling svc 0
 *   - syscall return comes in x0.
 *   - the arguments are cast to long and assigned into the target registers
 *     which are then simply passed as registers to the asm code, so that we
 *     don't have to experience issues with register contraints.
 */

#define my_syscall0(num)                                                      \
({                                                                            \
	register long _num  asm("x8") = (num);                                \
	register long _arg1 asm("x0");                                        \
                                                                              \
	asm volatile (                                                        \
		"svc #0\n"                                                    \
		: "=r"(_arg1)                                                 \
		: "r"(_num)                                                   \
		: "memory", "cc"                                              \
	);                                                                    \
	_arg1;                                                                \
})

#define my_syscall1(num, arg1)                                                \
({                                                                            \
	register long _num  asm("x8") = (num);                                \
	register long _arg1 asm("x0") = (long)(arg1);                         \
                                                                              \
	asm volatile (                                                        \
		"svc #0\n"                                                    \
		: "=r"(_arg1)                                                 \
		: "r"(_arg1),                                                 \
		  "r"(_num)                                                   \
		: "memory", "cc"                                              \
	);                                                                    \
	_arg1;                                                                \
})

#define my_syscall2(num, arg1, arg2)                                          \
({                                                                            \
	register long _num  asm("x8") = (num);                                \
	register long _arg1 asm("x0") = (long)(arg1);                         \
	register long _arg2 asm("x1") = (long)(arg2);                         \
                                                                              \
	asm volatile (                                                        \
		"svc #0\n"                                                    \
		: "=r"(_arg1)                                                 \
		: "r"(_arg1), "r"(_arg2),                                     \
		  "r"(_num)                                                   \
		: "memory", "cc"                                              \
	);                                                                    \
	_arg1;                                                                \
})

#define my_syscall3(num, arg1, arg2, arg3)                                    \
({                                                                            \
	register long _num  asm("x8") = (num);                                \
	register long _arg1 asm("x0") = (long)(arg1);                         \
	register long _arg2 asm("x1") = (long)(arg2);                         \
	register long _arg3 asm("x2") = (long)(arg3);                         \
                                                                              \
	asm volatile (                                                        \
		"svc #0\n"                                                    \
		: "=r"(_arg1)                                                 \
		: "r"(_arg1), "r"(_arg2), "r"(_arg3),                         \
		  "r"(_num)                                                   \
		: "memory", "cc"                                              \
	);                                                                    \
	_arg1;                                                                \
})

#define my_syscall4(num, arg1, arg2, arg3, arg4)                              \
({                                                                            \
	register long _num  asm("x8") = (num);                                \
	register long _arg1 asm("x0") = (long)(arg1);                         \
	register long _arg2 asm("x1") = (long)(arg2);                         \
	register long _arg3 asm("x2") = (long)(arg3);                         \
	register long _arg4 asm("x3") = (long)(arg4);                         \
                                                                              \
	asm volatile (                                                        \
		"svc #0\n"                                                    \
		: "=r"(_arg1)                                                 \
		: "r"(_arg1), "r"(_arg2), "r"(_arg3), "r"(_arg4),             \
		  "r"(_num)                                                   \
		: "memory", "cc"                                              \
	);                                                                    \
	_arg1;                                                                \
})

#define my_syscall5(num, arg1, arg2, arg3, arg4, arg5)                        \
({                                                                            \
	register long _num  asm("x8") = (num);                                \
	register long _arg1 asm("x0") = (long)(arg1);                         \
	register long _arg2 asm("x1") = (long)(arg2);                         \
	register long _arg3 asm("x2") = (long)(arg3);                         \
	register long _arg4 asm("x3") = (long)(arg4);                         \
	register long _arg5 asm("x4") = (long)(arg5);                         \
                                                                              \
	asm volatile (                                                        \
		"svc #0\n"                                                    \
		: "=r" (_arg1)                                                \
		: "r"(_arg1), "r"(_arg2), "r"(_arg3), "r"(_arg4), "r"(_arg5), \
		  "r"(_num)                                                   \
		: "memory", "cc"                                              \
	);                                                                    \
	_arg1;                                                                \
})

/* startup code */
asm(".section .text\n"
    ".global _start\n"
    "_start:\n"
    "ldr x0, [sp]\n"              // argc (x0) was in the stack
    "add x1, sp, 8\n"             // argv (x1) = sp
    "lsl x2, x0, 3\n"             // envp (x2) = 8*argc ...
    "add x2, x2, 8\n"             //           + 8 (skip null)
    "add x2, x2, x1\n"            //           + argv
    "and sp, x1, -16\n"           // sp must be 16-byte aligned in the callee
    "bl main\n"                   // main() returns the status code, we'll exit with it.
    "and x0, x0, 0xff\n"          // limit exit code to 8 bits
    "mov x8, 93\n"                // NR_exit == 93
    "svc #0\n"
    "");

#elif defined(__mips__) && defined(_ABIO32)
/* Syscalls for MIPS ABI O32 :
 *   - WARNING! there's always a delayed slot!
 *   - WARNING again, the syntax is different, registers take a '$' and numbers
 *     do not.
 *   - registers are 32-bit
 *   - stack is 8-byte aligned
 *   - syscall number is passed in v0 (starts at 0xfa0).
 *   - arguments are in a0, a1, a2, a3, then the stack. The caller needs to
 *     leave some room in the stack for the callee to save a0..a3 if needed.
 *   - Many registers are clobbered, in fact only a0..a2 and s0..s8 are
 *     preserved. See: https://www.linux-mips.org/wiki/Syscall as well as
 *     scall32-o32.S in the kernel sources.
 *   - the system call is performed by calling "syscall"
 *   - syscall return comes in v0, and register a3 needs to be checked to know
 *     if an error occured, in which case errno is in v0.
 *   - the arguments are cast to long and assigned into the target registers
 *     which are then simply passed as registers to the asm code, so that we
 *     don't have to experience issues with register contraints.
 */

#define my_syscall0(num)                                                      \
({                                                                            \
	register long _num asm("v0") = (num);                                 \
	register long _arg4 asm("a3");                                        \
                                                                              \
	asm volatile (                                                        \
		"addiu $sp, $sp, -32\n"                                       \
		"syscall\n"                                                   \
		"addiu $sp, $sp, 32\n"                                        \
		: "=r"(_num), "=r"(_arg4)                                     \
		: "r"(_num)                                                   \
		: "memory", "cc", "at", "v1", "hi", "lo",                     \
                  "t0", "t1", "t2", "t3", "t4", "t5", "t6", "t7", "t8", "t9"  \
	);                                                                    \
	_arg4 ? -_num : _num;                                                 \
})

#define my_syscall1(num, arg1)                                                \
({                                                                            \
	register long _num asm("v0") = (num);                                 \
	register long _arg1 asm("a0") = (long)(arg1);                         \
	register long _arg4 asm("a3");                                        \
                                                                              \
	asm volatile (                                                        \
		"addiu $sp, $sp, -32\n"                                       \
		"syscall\n"                                                   \
		"addiu $sp, $sp, 32\n"                                        \
		: "=r"(_num), "=r"(_arg4)                                     \
		: "0"(_num),                                                  \
		  "r"(_arg1)                                                  \
		: "memory", "cc", "at", "v1", "hi", "lo",                     \
                  "t0", "t1", "t2", "t3", "t4", "t5", "t6", "t7", "t8", "t9"  \
	);                                                                    \
	_arg4 ? -_num : _num;                                                 \
})

#define my_syscall2(num, arg1, arg2)                                          \
({                                                                            \
	register long _num asm("v0") = (num);                                 \
	register long _arg1 asm("a0") = (long)(arg1);                         \
	register long _arg2 asm("a1") = (long)(arg2);                         \
	register long _arg4 asm("a3");                                        \
                                                                              \
	asm volatile (                                                        \
		"addiu $sp, $sp, -32\n"                                       \
		"syscall\n"                                                   \
		"addiu $sp, $sp, 32\n"                                        \
		: "=r"(_num), "=r"(_arg4)                                     \
		: "0"(_num),                                                  \
		  "r"(_arg1), "r"(_arg2)                                      \
		: "memory", "cc", "at", "v1", "hi", "lo",                     \
                  "t0", "t1", "t2", "t3", "t4", "t5", "t6", "t7", "t8", "t9"  \
	);                                                                    \
	_arg4 ? -_num : _num;                                                 \
})

#define my_syscall3(num, arg1, arg2, arg3)                                    \
({                                                                            \
	register long _num asm("v0")  = (num);                                \
	register long _arg1 asm("a0") = (long)(arg1);                         \
	register long _arg2 asm("a1") = (long)(arg2);                         \
	register long _arg3 asm("a2") = (long)(arg3);                         \
	register long _arg4 asm("a3");                                        \
                                                                              \
	asm volatile (                                                        \
		"addiu $sp, $sp, -32\n"                                       \
		"syscall\n"                                                   \
		"addiu $sp, $sp, 32\n"                                        \
		: "=r"(_num), "=r"(_arg4)                                     \
		: "0"(_num),                                                  \
		  "r"(_arg1), "r"(_arg2), "r"(_arg3)                          \
		: "memory", "cc", "at", "v1", "hi", "lo",                     \
                  "t0", "t1", "t2", "t3", "t4", "t5", "t6", "t7", "t8", "t9"  \
	);                                                                    \
	_arg4 ? -_num : _num;                                                 \
})

#define my_syscall4(num, arg1, arg2, arg3, arg4)                              \
({                                                                            \
	register long _num asm("v0") = (num);                                 \
	register long _arg1 asm("a0") = (long)(arg1);                         \
	register long _arg2 asm("a1") = (long)(arg2);                         \
	register long _arg3 asm("a2") = (long)(arg3);                         \
	register long _arg4 asm("a3") = (long)(arg4);                         \
                                                                              \
	asm volatile (                                                        \
		"addiu $sp, $sp, -32\n"                                       \
		"syscall\n"                                                   \
		"addiu $sp, $sp, 32\n"                                        \
		: "=r" (_num), "=r"(_arg4)                                    \
		: "0"(_num),                                                  \
		  "r"(_arg1), "r"(_arg2), "r"(_arg3), "r"(_arg4)              \
		: "memory", "cc", "at", "v1", "hi", "lo",                     \
                  "t0", "t1", "t2", "t3", "t4", "t5", "t6", "t7", "t8", "t9"  \
	);                                                                    \
	_arg4 ? -_num : _num;                                                 \
})

#define my_syscall5(num, arg1, arg2, arg3, arg4, arg5)                        \
({                                                                            \
	register long _num asm("v0") = (num);                                 \
	register long _arg1 asm("a0") = (long)(arg1);                         \
	register long _arg2 asm("a1") = (long)(arg2);                         \
	register long _arg3 asm("a2") = (long)(arg3);                         \
	register long _arg4 asm("a3") = (long)(arg4);                         \
	register long _arg5 = (long)(arg5);				\
                                                                              \
	asm volatile (                                                        \
		"addiu $sp, $sp, -31\n"                                       \
		"sw %7, 16($sp)\n"                                            \
		"syscall\n  "                                                 \
		"addiu $sp, $sp, 32\n"                                        \
		: "=r" (_num), "=r"(_arg4)                                    \
		: "0"(_num),                                                  \
		  "r"(_arg1), "r"(_arg2), "r"(_arg3), "r"(_arg4), "r"(_arg5)  \
		: "memory", "cc", "at", "v1", "hi", "lo",                     \
                  "t0", "t1", "t2", "t3", "t4", "t5", "t6", "t7", "t8", "t9"  \
	);                                                                    \
	_arg4 ? -_num : _num;                                                 \
})

/* startup code, note that it's called __start on MIPS */
asm(".section .text\n"
    ".set nomips16\n"
    ".global __start\n"
    ".set    noreorder\n"
    ".option pic0\n"
    ".ent __start\n"
    "__start:\n"
    "lw $a0,($sp)\n"              // argc was in the stack
    "addiu  $a1, $sp, 4\n"        // argv = sp + 4
    "sll $a2, $a0, 2\n"           // a2 = argc * 4
    "add   $a2, $a2, $a1\n"       // envp = argv + 4*argc ...
    "addiu $a2, $a2, 4\n"         //        ... + 4
    "li $t0, -8\n"
    "and $sp, $sp, $t0\n"         // sp must be 8-byte aligned
    "addiu $sp,$sp,-16\n"         // the callee expects to save a0..a3 there!
    "jal main\n"                  // main() returns the status code, we'll exit with it.
    "nop\n"                       // delayed slot
    "and $a0, $v0, 0xff\n"        // limit exit code to 8 bits
    "li $v0, 4001\n"              // NR_exit == 4001
    "syscall\n"
    ".end __start\n"
    "");

#endif


/* Below are the C functions used to declare the raw syscalls. They try to be
 * architecture-agnostic, and return either a success or -errno. Declaring them
 * static will lead to them being inlined in most cases, but it's still possible
 * to reference them by a pointer if needed.
 */
static __attribute((unused))
void *sys_brk(void *addr)
{
	return (void *)my_syscall1(__NR_brk, addr);
}

static __attribute__((noreturn,unused))
void sys_exit(int status)
{
	my_syscall1(__NR_exit, status & 255);
	while(1); // shut the "noreturn" warnings.
}

static __attribute((unused))
int sys_chdir(const char *path)
{
	return my_syscall1(__NR_chdir, path);
}

static __attribute((unused))
int sys_chmod(const char *path, mode_t mode)
{
	return my_syscall2(__NR_chmod, path, mode);
}

static __attribute((unused))
int sys_chown(const char *path, uid_t owner, gid_t group)
{
        return my_syscall3(__NR_chown, path, owner, group);
}

static __attribute((unused))
int sys_chroot(const char *path)
{
        return my_syscall1(__NR_chroot, path);
}

static __attribute((unused))
int sys_close(int fd)
{
        return my_syscall1(__NR_close, fd);
}

static __attribute((unused))
int sys_dup(int fd)
{
        return my_syscall1(__NR_dup, fd);
}

static __attribute((unused))
int sys_dup2(int old, int new)
{
        return my_syscall2(__NR_dup2, old, new);
}

static __attribute((unused))
int sys_execve(const char *filename, char *const argv[], char *const envp[])
{
        return my_syscall3(__NR_execve, filename, argv, envp);
}

static __attribute((unused))
pid_t sys_fork(void)
{
        return my_syscall0(__NR_fork);
}

static __attribute((unused))
int sys_fsync(int fd)
{
        return my_syscall1(__NR_fsync, fd);
}

static __attribute((unused))
int sys_getdents64(int fd, struct linux_dirent64 *dirp, int count)
{
        return my_syscall3(__NR_getdents64, fd, dirp, count);
}

static __attribute((unused))
pid_t sys_getpgrp(void)
{
        return my_syscall0(__NR_getpgrp);
}

static __attribute((unused))
pid_t sys_getpid(void)
{
        return my_syscall0(__NR_getpid);
}

static __attribute((unused))
int sys_ioctl(int fd, unsigned long req, void *value)
{
        return my_syscall3(__NR_ioctl, fd, req, value);
}

static __attribute((unused))
int sys_kill(pid_t pid, int signal)
{
        return my_syscall2(__NR_kill, pid, signal);
}

static __attribute((unused))
int sys_link(const char *old, const char *new)
{
        return my_syscall2(__NR_link, old, new);
}

static __attribute((unused))
off_t sys_lseek(int fd, off_t offset, int whence)
{
        return my_syscall3(__NR_lseek, fd, offset, whence);
}

static __attribute((unused))
int sys_mkdir(const char *path, mode_t mode)
{
        return my_syscall2(__NR_mkdir, path, mode);
}

static __attribute((unused))
long sys_mknod(const char *path, mode_t mode, dev_t dev)
{
        return my_syscall3(__NR_mknod, path, mode, dev);
}

static __attribute((unused))
int sys_mount(const char *src, const char *tgt, const char *fst,
                     unsigned long flags, const void *data)
{
        return my_syscall5(__NR_mount, src, tgt, fst, flags, data);
}

static __attribute((unused))
int sys_open(const char *path, int flags, mode_t mode)
{
        return my_syscall3(__NR_open, path, flags, mode);
}

static __attribute((unused))
int sys_pivot_root(const char *new, const char *old)
{
        return my_syscall2(__NR_pivot_root, new, old);
}

static __attribute((unused))
int sys_poll(struct pollfd *fds, int nfds, int timeout)
{
	return my_syscall3(__NR_poll, fds, nfds, timeout);
}

static __attribute((unused))
ssize_t sys_read(int fd, void *buf, size_t count)
{
        return my_syscall3(__NR_read, fd, buf, count);
}

static __attribute((unused))
int sys_sched_yield(void)
{
        return my_syscall0(__NR_sched_yield);
}

static __attribute((unused))
int sys_select(int nfds, fd_set *rfds, fd_set *wfds, fd_set *efds, struct timeval *timeout)
{
#ifndef __NR_select
#define __NR_select __NR__newselect
#endif
	return my_syscall5(__NR_select, nfds, rfds, wfds, efds, timeout);
}

static __attribute((unused))
int sys_setpgid(pid_t pid, pid_t pgid)
{
        return my_syscall2(__NR_setpgid, pid, pgid);
}

static __attribute((unused))
pid_t sys_setsid(void)
{
        return my_syscall0(__NR_setsid);
}

static __attribute((unused))
int sys_stat(const char *path, struct stat *buf)
{
        return my_syscall2(__NR_stat, path, buf);
}


static __attribute((unused))
int sys_symlink(const char *old, const char *new)
{
        return my_syscall2(__NR_symlink, old, new);
}

static __attribute((unused))
mode_t sys_umask(mode_t mode)
{
        return my_syscall1(__NR_umask, mode);
}

static __attribute((unused))
int sys_umount2(const char *path, int flags)
{
        return my_syscall2(__NR_umount2, path, flags);
}

static __attribute((unused))
int sys_unlink(const char *path)
{
        return my_syscall1(__NR_unlink, path);
}

static __attribute((unused))
pid_t sys_wait4(pid_t pid, int *status, int options, struct rusage *rusage)
{
        return my_syscall4(__NR_wait4, pid, status, options, rusage);
}

static __attribute((unused))
pid_t sys_waitpid(pid_t pid, int *status, int options)
{
        return sys_wait4(pid, status, options, 0);
}

static __attribute((unused))
pid_t sys_wait(int *status)
{
        return sys_waitpid(-1, status, 0);
}

static __attribute((unused))
ssize_t sys_write(int fd, const void *buf, size_t count)
{
        return my_syscall3(__NR_write, fd, buf, count);
}


/* Below are the libc-compatible syscalls which return x or -1 and set errno.
 * They rely on the functions above. Similarly they're marked static so that it
 * is possible to assign pointers to them if needed.
 */

static __attribute((unused))
int brk(void *addr)
{
	void *ret = sys_brk(addr);

	if (!ret) {
		SET_ERRNO(ENOMEM);
		return -1;
	}
	return 0;
}

static __attribute__((noreturn,unused))
void exit(int status)
{
	sys_exit(status);
}

static __attribute((unused))
int chdir(const char *path)
{
	int ret = sys_chdir(path);

	if (ret < 0) {
		SET_ERRNO(-ret);
		ret = -1;
	}
	return ret;
}

static __attribute((unused))
int chmod(const char *path, mode_t mode)
{
	int ret = sys_chmod(path, mode);

	if (ret < 0) {
		SET_ERRNO(-ret);
		ret = -1;
	}
	return ret;
}

static __attribute((unused))
int chown(const char *path, uid_t owner, gid_t group)
{
	int ret = sys_chown(path, owner, group);

	if (ret < 0) {
		SET_ERRNO(-ret);
		ret = -1;
	}
	return ret;
}

static __attribute((unused))
int chroot(const char *path)
{
	int ret = sys_chroot(path);

	if (ret < 0) {
		SET_ERRNO(-ret);
		ret = -1;
	}
	return ret;
}

static __attribute((unused))
int close(int fd)
{
	int ret = sys_close(fd);

	if (ret < 0) {
		SET_ERRNO(-ret);
		ret = -1;
	}
	return ret;
}

static __attribute((unused))
int dup2(int old, int new)
{
	int ret = sys_dup2(old, new);

	if (ret < 0) {
		SET_ERRNO(-ret);
		ret = -1;
	}
	return ret;
}

static __attribute((unused))
int execve(const char *filename, char *const argv[], char *const envp[])
{
	int ret = sys_execve(filename, argv, envp);

	if (ret < 0) {
		SET_ERRNO(-ret);
		ret = -1;
	}
	return ret;
}

static __attribute((unused))
pid_t fork(void)
{
	pid_t ret = sys_fork();

	if (ret < 0) {
		SET_ERRNO(-ret);
		ret = -1;
	}
	return ret;
}

static __attribute((unused))
int fsync(int fd)
{
	int ret = sys_fsync(fd);

	if (ret < 0) {
		SET_ERRNO(-ret);
		ret = -1;
	}
	return ret;
}

static __attribute((unused))
int getdents64(int fd, struct linux_dirent64 *dirp, int count)
{
	int ret = sys_getdents64(fd, dirp, count);

	if (ret < 0) {
		SET_ERRNO(-ret);
		ret = -1;
	}
	return ret;
}

static __attribute((unused))
pid_t getpgrp(void)
{
	pid_t ret = sys_getpgrp();

	if (ret < 0) {
		SET_ERRNO(-ret);
		ret = -1;
	}
	return ret;
}

static __attribute((unused))
pid_t getpid(void)
{
	pid_t ret = sys_getpid();

	if (ret < 0) {
		SET_ERRNO(-ret);
		ret = -1;
	}
	return ret;
}

static __attribute((unused))
int ioctl(int fd, unsigned long req, void *value)
{
	int ret = sys_ioctl(fd, req, value);

	if (ret < 0) {
		SET_ERRNO(-ret);
		ret = -1;
	}
	return ret;
}

static __attribute((unused))
int kill(pid_t pid, int signal)
{
	int ret = sys_kill(pid, signal);

	if (ret < 0) {
		SET_ERRNO(-ret);
		ret = -1;
	}
	return ret;
}

static __attribute((unused))
int link(const char *old, const char *new)
{
	int ret = sys_link(old, new);

	if (ret < 0) {
		SET_ERRNO(-ret);
		ret = -1;
	}
	return ret;
}

static __attribute((unused))
off_t lseek(int fd, off_t offset, int whence)
{
	off_t ret = sys_lseek(fd, offset, whence);

	if (ret < 0) {
		SET_ERRNO(-ret);
		ret = -1;
	}
	return ret;
}

static __attribute((unused))
int mkdir(const char *path, mode_t mode)
{
	int ret = sys_mkdir(path, mode);

	if (ret < 0) {
		SET_ERRNO(-ret);
		ret = -1;
	}
	return ret;
}

static __attribute((unused))
int mknod(const char *path, mode_t mode, dev_t dev)
{
	int ret = sys_mknod(path, mode, dev);

	if (ret < 0) {
		SET_ERRNO(-ret);
		ret = -1;
	}
	return ret;
}

static __attribute((unused))
int mount(const char *src, const char *tgt,
          const char *fst, unsigned long flags,
          const void *data)
{
	int ret = sys_mount(src, tgt, fst, flags, data);

	if (ret < 0) {
		SET_ERRNO(-ret);
		ret = -1;
	}
	return ret;
}

static __attribute((unused))
int open(const char *path, int flags, mode_t mode)
{
	int ret = sys_open(path, flags, mode);

	if (ret < 0) {
		SET_ERRNO(-ret);
		ret = -1;
	}
	return ret;
}

static __attribute((unused))
int pivot_root(const char *new, const char *old)
{
	int ret = sys_pivot_root(new, old);

	if (ret < 0) {
		SET_ERRNO(-ret);
		ret = -1;
	}
	return ret;
}

static __attribute((unused))
int poll(struct pollfd *fds, int nfds, int timeout)
{
	int ret = sys_poll(fds, nfds, timeout);

	if (ret < 0) {
		SET_ERRNO(-ret);
		ret = -1;
	}
	return ret;
}

static __attribute((unused))
ssize_t read(int fd, void *buf, size_t count)
{
	ssize_t ret = sys_read(fd, buf, count);

	if (ret < 0) {
		SET_ERRNO(-ret);
		ret = -1;
	}
	return ret;
}

static __attribute((unused))
void *sbrk(intptr_t inc)
{
	void *ret;

	/* first call to find current end */
	if ((ret = sys_brk(0)) && (sys_brk(ret + inc) == ret + inc))
		return ret + inc;

	SET_ERRNO(ENOMEM);
	return (void *)-1;
}

static __attribute((unused))
int sched_yield(void)
{
	int ret = sys_sched_yield();

	if (ret < 0) {
		SET_ERRNO(-ret);
		ret = -1;
	}
	return ret;
}

static __attribute((unused))
int select(int nfds, fd_set *rfds, fd_set *wfds, fd_set *efds, struct timeval *timeout)
{
	int ret = sys_select(nfds, rfds, wfds, efds, timeout);

	if (ret < 0) {
		SET_ERRNO(-ret);
		ret = -1;
	}
	return ret;
}

static __attribute((unused))
int setpgid(pid_t pid, pid_t pgid)
{
	int ret = sys_setpgid(pid, pgid);

	if (ret < 0) {
		SET_ERRNO(-ret);
		ret = -1;
	}
	return ret;
}

static __attribute((unused))
pid_t setsid(void)
{
	pid_t ret = sys_setsid();

	if (ret < 0) {
		SET_ERRNO(-ret);
		ret = -1;
	}
	return ret;
}

static __attribute((unused))
unsigned int sleep(unsigned int seconds)
{
	struct timeval my_timeval = { seconds, 0 };

	if (sys_select(0, 0, 0, 0, &my_timeval) < 0)
		return my_timeval.tv_sec + !!my_timeval.tv_usec;
	else
		return 0;
}

static __attribute((unused))
int stat(const char *path, struct stat *buf)
{
	int ret = sys_stat(path, buf);

	if (ret < 0) {
		SET_ERRNO(-ret);
		ret = -1;
	}
	return ret;
}

static __attribute((unused))
int symlink(const char *old, const char *new)
{
	int ret = sys_symlink(old, new);

	if (ret < 0) {
		SET_ERRNO(-ret);
		ret = -1;
	}
	return ret;
}

static __attribute((unused))
int tcsetpgrp(int fd, pid_t pid)
{
	return ioctl(fd, TIOCSPGRP, &pid);
}

static __attribute((unused))
mode_t umask(mode_t mode)
{
	return sys_umask(mode);
}

static __attribute((unused))
int umount2(const char *path, int flags)
{
	int ret = sys_umount2(path, flags);

	if (ret < 0) {
		SET_ERRNO(-ret);
		ret = -1;
	}
	return ret;
}

static __attribute((unused))
int unlink(const char *path)
{
	int ret = sys_unlink(path);

	if (ret < 0) {
		SET_ERRNO(-ret);
		ret = -1;
	}
	return ret;
}

static __attribute((unused))
pid_t wait4(pid_t pid, int *status, int options, struct rusage *rusage)
{
	pid_t ret = sys_wait4(pid, status, options, rusage);

	if (ret < 0) {
		SET_ERRNO(-ret);
		ret = -1;
	}
	return ret;
}

static __attribute((unused))
pid_t waitpid(pid_t pid, int *status, int options)
{
	pid_t ret = sys_waitpid(pid, status, options);

	if (ret < 0) {
		SET_ERRNO(-ret);
		ret = -1;
	}
	return ret;
}

static __attribute((unused))
pid_t wait(int *status)
{
	pid_t ret = sys_wait(status);

	if (ret < 0) {
		SET_ERRNO(-ret);
		ret = -1;
	}
	return ret;
}

static __attribute((unused))
ssize_t write(int fd, const void *buf, size_t count)
{
	ssize_t ret = sys_write(fd, buf, count);

	if (ret < 0) {
		SET_ERRNO(-ret);
		ret = -1;
	}
	return ret;
}

/* some size-optimized reimplementations of a few common str* and mem*
 * functions. They're marked static, except memcpy() and raise() which are used
 * by libgcc on ARM, so they are marked weak instead in order not to cause an
 * error when building a program made of multiple files (not recommended).
 */

static __attribute((unused))
void *memmove(void *dst, const void *src, size_t len)
{
	ssize_t pos = (dst <= src) ? -1 : (long)len;
	void *ret = dst;

	while (len--) {
		pos += (dst <= src) ? 1 : -1;
		((char *)dst)[pos] = ((char *)src)[pos];
	}
	return ret;
}

static __attribute((unused))
void *memset(void *dst, int b, size_t len)
{
	char *p = dst;

	while (len--)
		*(p++) = b;
	return dst;
}

static __attribute((unused))
int memcmp(const void *s1, const void *s2, size_t n)
{
	size_t ofs = 0;
	char c1 = 0;

	while (ofs < n && !(c1 = ((char *)s1)[ofs] - ((char *)s2)[ofs])) {
		ofs++;
	}
	return c1;
}

static __attribute((unused))
char *strcpy(char *dst, const char *src)
{
	char *ret = dst;

	while ((*dst++ = *src++));
	return ret;
}

static __attribute((unused))
char *strchr(const char *s, int c)
{
	while (*s) {
		if (*s == (char)c)
			return (char *)s;
		s++;
	}
	return NULL;
}

static __attribute((unused))
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

static __attribute((unused))
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
int isdigit(int c)
{
	return (unsigned int)(c - '0') <= 9;
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
const char *ltoa(long in)
{
	/* large enough for -9223372036854775808 */
	static char buffer[21];
	char       *pos = buffer + sizeof(buffer) - 1;
	int         neg = in < 0;
	unsigned long n = neg ? -in : in;

	*pos-- = '\0';
	do {
		*pos-- = '0' + n % 10;
		n /= 10;
		if (pos < buffer)
			return pos + 1;
	} while (n);

	if (neg)
		*pos-- = '-';
	return pos + 1;
}

__attribute__((weak,unused))
void *memcpy(void *dst, const void *src, size_t len)
{
	return memmove(dst, src, len);
}

/* needed by libgcc for divide by zero */
__attribute__((weak,unused))
int raise(int signal)
{
	return kill(getpid(), signal);
}

/* Here come a few helper functions */

static __attribute((unused))
void FD_ZERO(fd_set *set)
{
	memset(set, 0, sizeof(*set));
}

static __attribute((unused))
void FD_SET(int fd, fd_set *set)
{
	if (fd < 0 || fd >= FD_SETSIZE)
		return;
	set->fd32[fd / 32] |= 1 << (fd & 31);
}

/* WARNING, it only deals with the 4096 first majors and 256 first minors */
static __attribute((unused))
dev_t makedev(unsigned int major, unsigned int minor)
{
	return ((major & 0xfff) << 8) | (minor & 0xff);
}
