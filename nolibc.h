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


/* Below are the C functions used to declare the raw syscalls. They try to be
 * architecture-agnostic, and return either a success or -errno. Declaring them
 * static will lead to them being inlined in most cases, but it's still possible
 * to reference them by a pointer if needed.
 */
static __attribute__((noreturn))
void sys_exit(int status)
{
	my_syscall1(__NR_exit, status & 255);
	while(1); // shut the "noreturn" warnings.
}

static
int sys_chdir(const char *path)
{
	return my_syscall1(__NR_chdir, path);
}

static
int sys_chmod(const char *path, mode_t mode)
{
	return my_syscall2(__NR_chmod, path, mode);
}

static
int sys_chown(const char *path, uid_t owner, gid_t group)
{
        return my_syscall3(__NR_chown, path, owner, group);
}

static
int sys_chroot(const char *path)
{
        return my_syscall1(__NR_chroot, path);
}

static
int sys_close(int fd)
{
        return my_syscall1(__NR_close, fd);
}

static
int sys_dup(int fd)
{
        return my_syscall1(__NR_dup, fd);
}

static
int sys_dup2(int old, int new)
{
        return my_syscall2(__NR_dup2, old, new);
}

static
int sys_execve(const char *filename, char *const argv[], char *const envp[])
{
        return my_syscall3(__NR_execve, filename, argv, envp);
}

static
pid_t sys_fork(void)
{
        return my_syscall0(__NR_fork);
}

static
pid_t sys_getpgrp(void)
{
        return my_syscall0(__NR_getpgrp);
}

static
pid_t sys_getpid(void)
{
        return my_syscall0(__NR_getpid);
}

static
int sys_ioctl(int fd, unsigned long req, void *value)
{
        return my_syscall3(__NR_ioctl, fd, req, value);
}

static
int sys_kill(pid_t pid, int signal)
{
        return my_syscall2(__NR_kill, pid, signal);
}

static
int sys_link(const char *old, const char *new)
{
        return my_syscall2(__NR_link, old, new);
}

static
int sys_mkdir(const char *path, mode_t mode)
{
        return my_syscall2(__NR_mkdir, path, mode);
}

static
long sys_mknod(const char *path, mode_t mode, dev_t dev)
{
        return my_syscall3(__NR_mknod, path, mode, dev);
}

static
int sys_mount(const char *src, const char *tgt, const char *fst,
                     unsigned long flags, const void *data)
{
        return my_syscall5(__NR_mount, src, tgt, fst, flags, data);
}

static
int sys_open(const char *path, int flags, mode_t mode)
{
        return my_syscall3(__NR_open, path, flags, mode);
}

static
int sys_pivot_root(const char *new, const char *old)
{
        return my_syscall2(__NR_pivot_root, new, old);
}

static
int sys_poll(struct pollfd *fds, int nfds, int timeout)
{
	return my_syscall3(__NR_poll, fds, nfds, timeout);
}

static
ssize_t sys_read(int fd, void *buf, size_t count)
{
        return my_syscall3(__NR_read, fd, buf, count);
}

static
int sys_sched_yield(void)
{
        return my_syscall0(__NR_sched_yield);
}

static
int sys_select(int nfds, fd_set *rfds, fd_set *wfds, fd_set *efds, struct timeval *timeout)
{
#ifndef __NR_select
#define __NR_select __NR__newselect
#endif
	return my_syscall5(__NR_select, nfds, rfds, wfds, efds, timeout);
}

static
int sys_setpgid(pid_t pid, pid_t pgid)
{
        return my_syscall2(__NR_setpgid, pid, pgid);
}

static
pid_t sys_setsid(void)
{
        return my_syscall0(__NR_setsid);
}

static
int sys_stat(const char *path, struct stat *buf)
{
        return my_syscall2(__NR_stat, path, buf);
}


static
int sys_symlink(const char *old, const char *new)
{
        return my_syscall2(__NR_symlink, old, new);
}

static
mode_t sys_umask(mode_t mode)
{
        return my_syscall1(__NR_umask, mode);
}

static
int sys_umount2(const char *path, int flags)
{
        return my_syscall2(__NR_umount2, path, flags);
}

static
int sys_unlink(const char *path)
{
        return my_syscall1(__NR_unlink, path);
}

static
pid_t sys_wait4(pid_t pid, int *status, int options, struct rusage *rusage)
{
        return my_syscall4(__NR_wait4, pid, status, options, rusage);
}

static
pid_t sys_waitpid(pid_t pid, int *status, int options)
{
        return sys_wait4(pid, status, options, 0);
}

static
pid_t sys_wait(int *status)
{
        return sys_waitpid(-1, status, 0);
}

static
ssize_t sys_write(int fd, const void *buf, size_t count)
{
        return my_syscall3(__NR_write, fd, buf, count);
}


/* Below are the libc-compatible syscalls which return x or -1 and set errno.
 * They rely on the functions above. Similarly they're marked static so that it
 * is possible to assign pointers to them if needed.
 */

static __attribute__((noreturn))
void exit(int status)
{
	sys_exit(status);
}

static
int chdir(const char *path)
{
	int ret = sys_chdir(path);

	if (ret < 0) {
		SET_ERRNO(-ret);
		ret = -1;
	}
	return ret;
}

static
int chmod(const char *path, mode_t mode)
{
	int ret = sys_chmod(path, mode);

	if (ret < 0) {
		SET_ERRNO(-ret);
		ret = -1;
	}
	return ret;
}

static
int chown(const char *path, uid_t owner, gid_t group)
{
	int ret = sys_chown(path, owner, group);

	if (ret < 0) {
		SET_ERRNO(-ret);
		ret = -1;
	}
	return ret;
}

static
int chroot(const char *path)
{
	int ret = sys_chroot(path);

	if (ret < 0) {
		SET_ERRNO(-ret);
		ret = -1;
	}
	return ret;
}

static
int close(int fd)
{
	int ret = sys_close(fd);

	if (ret < 0) {
		SET_ERRNO(-ret);
		ret = -1;
	}
	return ret;
}

static
int dup2(int old, int new)
{
	int ret = sys_dup2(old, new);

	if (ret < 0) {
		SET_ERRNO(-ret);
		ret = -1;
	}
	return ret;
}

static
int execve(const char *filename, char *const argv[], char *const envp[])
{
	int ret = sys_execve(filename, argv, envp);

	if (ret < 0) {
		SET_ERRNO(-ret);
		ret = -1;
	}
	return ret;
}

static
pid_t fork(void)
{
	pid_t ret = sys_fork();

	if (ret < 0) {
		SET_ERRNO(-ret);
		ret = -1;
	}
	return ret;
}

static
pid_t getpgrp(void)
{
	pid_t ret = sys_getpgrp();

	if (ret < 0) {
		SET_ERRNO(-ret);
		ret = -1;
	}
	return ret;
}

static
pid_t getpid(void)
{
	pid_t ret = sys_getpid();

	if (ret < 0) {
		SET_ERRNO(-ret);
		ret = -1;
	}
	return ret;
}

static
int ioctl(int fd, unsigned long req, void *value)
{
	int ret = sys_ioctl(fd, req, value);

	if (ret < 0) {
		SET_ERRNO(-ret);
		ret = -1;
	}
	return ret;
}

static
int kill(pid_t pid, int signal)
{
	int ret = sys_kill(pid, signal);

	if (ret < 0) {
		SET_ERRNO(-ret);
		ret = -1;
	}
	return ret;
}

static
int link(const char *old, const char *new)
{
	int ret = sys_link(old, new);

	if (ret < 0) {
		SET_ERRNO(-ret);
		ret = -1;
	}
	return ret;
}

static
int mkdir(const char *path, mode_t mode)
{
	int ret = sys_mkdir(path, mode);

	if (ret < 0) {
		SET_ERRNO(-ret);
		ret = -1;
	}
	return ret;
}

static
int mknod(const char *path, mode_t mode, dev_t dev)
{
	int ret = sys_mknod(path, mode, dev);

	if (ret < 0) {
		SET_ERRNO(-ret);
		ret = -1;
	}
	return ret;
}

static
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

static
int open(const char *path, int flags, mode_t mode)
{
	int ret = sys_open(path, flags, mode);

	if (ret < 0) {
		SET_ERRNO(-ret);
		ret = -1;
	}
	return ret;
}

static
int pivot_root(const char *new, const char *old)
{
	int ret = sys_pivot_root(new, old);

	if (ret < 0) {
		SET_ERRNO(-ret);
		ret = -1;
	}
	return ret;
}

static
int poll(struct pollfd *fds, int nfds, int timeout)
{
	int ret = sys_poll(fds, nfds, timeout);

	if (ret < 0) {
		SET_ERRNO(-ret);
		ret = -1;
	}
	return ret;
}

static
ssize_t read(int fd, void *buf, size_t count)
{
	ssize_t ret = sys_read(fd, buf, count);

	if (ret < 0) {
		SET_ERRNO(-ret);
		ret = -1;
	}
	return ret;
}

static
int sched_yield(void)
{
	int ret = sys_sched_yield();

	if (ret < 0) {
		SET_ERRNO(-ret);
		ret = -1;
	}
	return ret;
}

static
int select(int nfds, fd_set *rfds, fd_set *wfds, fd_set *efds, struct timeval *timeout)
{
	int ret = sys_select(nfds, rfds, wfds, efds, timeout);

	if (ret < 0) {
		SET_ERRNO(-ret);
		ret = -1;
	}
	return ret;
}

static
int setpgid(pid_t pid, pid_t pgid)
{
	int ret = sys_setpgid(pid, pgid);

	if (ret < 0) {
		SET_ERRNO(-ret);
		ret = -1;
	}
	return ret;
}

static
pid_t setsid(void)
{
	pid_t ret = sys_setsid();

	if (ret < 0) {
		SET_ERRNO(-ret);
		ret = -1;
	}
	return ret;
}

static
unsigned int sleep(unsigned int seconds)
{
	struct timeval my_timeval = { seconds, 0 };

	if (sys_select(0, 0, 0, 0, &my_timeval) < 0)
		return my_timeval.tv_sec + !!my_timeval.tv_usec;
	else
		return 0;
}

static
int stat(const char *path, struct stat *buf)
{
	int ret = sys_stat(path, buf);

	if (ret < 0) {
		SET_ERRNO(-ret);
		ret = -1;
	}
	return ret;
}

static
int symlink(const char *old, const char *new)
{
	int ret = sys_symlink(old, new);

	if (ret < 0) {
		SET_ERRNO(-ret);
		ret = -1;
	}
	return ret;
}

static
int tcsetpgrp(int fd, pid_t pid)
{
	return ioctl(fd, TIOCSPGRP, &pid);
}

static
mode_t umask(mode_t mode)
{
	return sys_umask(mode);
}

static
int umount2(const char *path, int flags)
{
	int ret = sys_umount2(path, flags);

	if (ret < 0) {
		SET_ERRNO(-ret);
		ret = -1;
	}
	return ret;
}

static
int unlink(const char *path)
{
	int ret = sys_unlink(path);

	if (ret < 0) {
		SET_ERRNO(-ret);
		ret = -1;
	}
	return ret;
}

static
pid_t wait4(pid_t pid, int *status, int options, struct rusage *rusage)
{
	pid_t ret = sys_wait4(pid, status, options, rusage);

	if (ret < 0) {
		SET_ERRNO(-ret);
		ret = -1;
	}
	return ret;
}

static
pid_t waitpid(pid_t pid, int *status, int options)
{
	pid_t ret = sys_waitpid(pid, status, options);

	if (ret < 0) {
		SET_ERRNO(-ret);
		ret = -1;
	}
	return ret;
}

static
pid_t wait(int *status)
{
	pid_t ret = sys_wait(status);

	if (ret < 0) {
		SET_ERRNO(-ret);
		ret = -1;
	}
	return ret;
}

static
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

static
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

static
void *memset(void *dst, int b, size_t len)
{
	char *p = dst;

	while (len--)
		*(p++) = b;
	return dst;
}

static
char *strcpy(char *dst, const char *src)
{
	char *ret = dst;

	while ((*dst++ = *src++));
	return ret;
}

static
size_t strlen(const char *str)
{
	size_t len;

	for (len = 0; str[len]; len++);
	return len;
}

__attribute__((weak))
void *memcpy(void *dst, const void *src, size_t len)
{
	return memmove(dst, src, len);
}

/* needed by libgcc for divide by zero */
__attribute__((weak))
int raise(int signal)
{
	return kill(getpid(), signal);
}

/* Here come a few helper functions */

static
void FD_ZERO(fd_set *set)
{
	memset(set, 0, sizeof(*set));
}

static
void FD_SET(int fd, fd_set *set)
{
	if (fd < 0 || fd >= FD_SETSIZE)
		return;
	set->fd32[fd / 32] |= 1 << (fd & 31);
}

/* WARNING, it only deals with the 4096 first majors and 256 first minors */
static
dev_t makedev(unsigned int major, unsigned int minor)
{
	return ((major & 0xfff) << 8) | (minor & 0xff);
}
