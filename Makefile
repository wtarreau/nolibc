
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

arch_files := arch.h $(wildcard arch-*.h)
all_files := \
		compiler.h \
		crt.h \
		ctype.h \
		dirent.h \
		elf.h \
		errno.h \
		fcntl.h \
		getopt.h \
		inttypes.h \
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
		sys/select.h \
		sys/stat.h \
		sys/syscall.h \
		sys/sysmacros.h \
		sys/time.h \
		sys/timerfd.h \
		sys/types.h \
		sys/uio.h \
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
	@echo "  headers             prepare a multi-arch sysroot in \$${OUTPUT}sysroot"
	@echo "  headers_standalone  like \"headers\", and also install kernel headers"
	@echo "  help                this help"
	@echo "  install_khdr_all    install multi-arch kernel headers in \$${OUTPUT}sysroot"
	@echo "  install_all         install a multi-arch sysroot in \$${OUTPUT}sysroot"
	@echo ""
	@echo "These targets may also be called from tools as \"make nolibc_<target>\"."
	@echo ""
	@echo "Currently using the following variables:"
	@echo "  ARCH    = $(ARCH)"
	@echo "  OUTPUT  = $(OUTPUT)"
	@echo "  KDIR    = $(KDIR)"
	@echo ""

# installs nolibc headers and kernel headers for all archs
install_all: headers install_khdr_all

# installs headers for all archs at once.
headers:
	$(Q)mkdir -p "$(OUTPUT)sysroot"
	$(Q)mkdir -p "$(OUTPUT)sysroot/include"
	$(Q)cp --parents $(arch_files) $(all_files) "$(OUTPUT)sysroot/include/"

headers_standalone: headers
	$(Q)$(MAKE) -C $(srctree) headers
	$(Q)$(MAKE) -C $(srctree) headers_install INSTALL_HDR_PATH=$(OUTPUT)sysroot

install_khdr_all:
	@# install common headers for any arch, take them all. This will clear everything.
	$(Q)$(MAKE) -C $(srctree) ARCH=x86 mrproper
	$(Q)$(MAKE) -C $(srctree) ARCH=x86 headers_install no-export-headers= INSTALL_HDR_PATH="$(OUTPUT)sysroot"
	@# remove the contents of the unused asm dir which we will rebuild from the arch ones
	$(Q)rm -rf "$(OUTPUT)sysroot/include/asm"
	$(Q)mkdir -p "$(OUTPUT)sysroot/include/asm"
	@# Now install headers for all archs
	$(Q)for arch in $(patsubst arch-%.h,%,$(filter arch-%.h,$(arch_files))); do \
		echo "# installing $$arch"; \
		if ! [ -d $(OUTPUT)sysroot/include/asm-arch-$$arch ]; then \
			$(MAKE) -C $(srctree) ARCH=$$arch mrproper; \
			$(MAKE) -C $(srctree) ARCH=$$arch headers_install no-export-headers= \
				INSTALL_HDR_PATH="$(OUTPUT)sysroot/include/$$arch" >/dev/null; \
			mv "$(OUTPUT)sysroot/include/$$arch/include/asm" "$(OUTPUT)sysroot/include/asm-arch-$$arch"; \
			rm -rf "$(OUTPUT)sysroot/include/$$arch"; \
		fi;\
	done; \
	mkdir -p "$(OUTPUT)sysroot/include/asm"; \
	for file in $$(find "$(OUTPUT)sysroot/include/"asm-arch-* -maxdepth 1 -name '*.h' -printf '%P\n'); do \
		sed -e "s!_ASMFILE_!$$file!" asm-template.h > "$(OUTPUT)sysroot/include/asm/$$file"; \
	done

clean:
	$(Q)rm -rf "$(OUTPUT)sysroot"
