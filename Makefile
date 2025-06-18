
# ARCH should be set to the kernel's arch
ARCH := $(shell uname -m)

# KDIR should be set to kernel dir
KDIR ?= /usr/src/linux

# point to the root of the kernel
srctree := $(KDIR)

# OUTPUT is only set when run from the main makefile, otherwise
# it defaults to this nolibc directory.
OUTPUT ?= $(CURDIR)/

ifeq ($(V),1)
Q=
else
Q=@
endif

nolibc_arch := $(patsubst arm64,aarch64,$(ARCH))
arch_file := arch-$(nolibc_arch).h
all_files := \
		compiler.h \
		crt.h \
		ctype.h \
		dirent.h \
		elf.h \
		errno.h \
		fcntl.h \
		getopt.h \
		limits.h \
		math.h \
		nolibc.h \
		poll.h \
		sched.h \
		signal.h \
		stackprotector.h \
		std.h \
		stdarg.h \
		stdbool.h \
		stddef.h \
		stdint.h \
		stdlib.h \
		string.h \
		sys.h \
		sys/auxv.h \
		sys/ioctl.h \
		sys/mman.h \
		sys/mount.h \
		sys/prctl.h \
		sys/random.h \
		sys/reboot.h \
		sys/resource.h \
		sys/stat.h \
		sys/syscall.h \
		sys/sysmacros.h \
		sys/time.h \
		sys/timerfd.h \
		sys/types.h \
		sys/utsname.h \
		sys/wait.h \
		time.h \
		types.h \
		unistd.h \
		stdio.h \


# install all headers needed to support a bare-metal compiler
all: headers

install: help

help:
	@echo "Supported targets under nolibc:"
	@echo "  all                 call \"headers\""
	@echo "  clean               clean the sysroot"
	@echo "  headers             prepare a sysroot in \$${OUTPUT}sysroot"
	@echo "  headers_standalone  like \"headers\", and also install kernel headers"
	@echo "  help                this help"
	@echo ""
	@echo "These targets may also be called from tools as \"make nolibc_<target>\"."
	@echo ""
	@echo "Currently using the following variables:"
	@echo "  ARCH    = $(ARCH)"
	@echo "  OUTPUT  = $(OUTPUT)"
	@echo "  KDIR    = $(KDIR)"
	@echo ""

# Note: when ARCH is "x86" we concatenate both x86_64 and i386
headers:
	$(Q)mkdir -p $(OUTPUT)sysroot
	$(Q)mkdir -p $(OUTPUT)sysroot/include
	$(Q)cp --parents $(all_files) $(OUTPUT)sysroot/include/
	$(Q)if [ "$(ARCH)" = "x86" ]; then      \
		sed -e                          \
		  's,^#ifndef _NOLIBC_ARCH_X86_64_H,#if !defined(_NOLIBC_ARCH_X86_64_H) \&\& defined(__x86_64__),' \
		  arch-x86_64.h;                \
		sed -e                          \
		  's,^#ifndef _NOLIBC_ARCH_I386_H,#if !defined(_NOLIBC_ARCH_I386_H) \&\& !defined(__x86_64__),' \
		  arch-i386.h;                  \
	elif [ -e "$(arch_file)" ]; then        \
		cat $(arch_file);               \
	else                                    \
		echo "Fatal: architecture $(ARCH) not yet supported by nolibc." >&2; \
		exit 1;                         \
	fi > $(OUTPUT)sysroot/include/arch.h

headers_standalone: headers
	$(Q)$(MAKE) -C $(srctree) headers
	$(Q)$(MAKE) -C $(srctree) headers_install INSTALL_HDR_PATH=$(OUTPUT)sysroot

clean:
	$(Q)rm -rf "$(OUTPUT)sysroot"
