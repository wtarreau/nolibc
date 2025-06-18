
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

nolibc_supported_archs := aarch64 arm loongarch m68k mips powerpc riscv s390 sparc x86

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
	@echo "  headers_all_archs   prepare a multi-arch sysroot in \$${OUTPUT}sysroot"
	@echo "  headers_standalone  like \"headers\", and also install kernel headers"
	@echo "  help                this help"
	@echo "  install_all_archs   install a multi-arch sysroot + kernel headers in \$${OUTPUT}sysroot"
	@echo ""
	@echo "These targets may also be called from tools as \"make nolibc_<target>\"."
	@echo ""
	@echo "Currently using the following variables:"
	@echo "  ARCH    = $(ARCH)"
	@echo "  OUTPUT  = $(OUTPUT)"
	@echo "  KDIR    = $(KDIR)"
	@echo ""

headers:
	$(Q)mkdir -p $(OUTPUT)sysroot
	$(Q)mkdir -p $(OUTPUT)sysroot/include
	$(Q)cp --parents $(all_files) $(OUTPUT)sysroot/include/
	$(Q)if [ "$(ARCH)" = "i386" -o "$(ARCH)" = "x86_64" ]; then \
		cat arch-x86.h;                 \
	elif [ -e "$(arch_file)" ]; then        \
		cat $(arch_file);               \
	else                                    \
		echo "Fatal: architecture $(ARCH) not yet supported by nolibc." >&2; \
		exit 1;                         \
	fi > $(OUTPUT)sysroot/include/arch.h

headers_standalone: headers
	$(Q)$(MAKE) -C $(srctree) headers
	$(Q)$(MAKE) -C $(srctree) headers_install INSTALL_HDR_PATH=$(OUTPUT)sysroot

# installs headers for all archs at once.
headers_all_archs:
	$(Q)mkdir -p "$(OUTPUT)sysroot"
	$(Q)mkdir -p "$(OUTPUT)sysroot/include"
	$(Q)cp --parents $(all_files) arch.h "$(OUTPUT)sysroot/include/"
	$(Q)cp $(addsuffix .h,$(addprefix arch-,$(nolibc_supported_archs))) "$(OUTPUT)sysroot/include/"

install_all_archs: headers_all_archs
	@# install common headers for any arch, take them all. This will clear everything.
	$(Q)$(MAKE) -C $(srctree) ARCH=x86 mrproper
	$(Q)$(MAKE) -C $(srctree) ARCH=x86 headers_install no-export-headers= INSTALL_HDR_PATH="$(OUTPUT)sysroot"
	@# remove the contents of the unused asm dir which we will rebuild from the arch ones
	$(Q)rm -rf "$(OUTPUT)sysroot/include/asm"
	$(Q)mkdir -p "$(OUTPUT)sysroot/include/asm"
	@# Now install headers for all archs
	$(Q)for arch in $(patsubst aarch64,arm64,$(nolibc_supported_archs)); do \
		echo "# installing $$arch"; \
		if ! [ -d $(OUTPUT)sysroot/include/asm-arch-$$arch ]; then \
			$(MAKE) -C $(srctree) ARCH=$$arch mrproper; \
			$(MAKE) -C $(srctree) ARCH=$$arch headers_install no-export-headers= \
				INSTALL_HDR_PATH="$(OUTPUT)sysroot/include/$$arch" >/dev/null; \
			mv "$(OUTPUT)sysroot/include/$$arch/include/asm" "$(OUTPUT)sysroot/include/asm-arch-$$arch"; \
			rm -rf "$(OUTPUT)sysroot/include/$$arch"; \
			case "$$arch" in \
				arm)       cond="defined(__ARM_EABI__)" ;; \
				arm64)     cond="defined(__aarch64__)" ;; \
				loongarch) cond="defined(__loongarch__)" ;; \
				m68k)      cond="defined(__m68k__)" ;; \
				mips)      cond="defined(__mips__)" ;; \
				powerpc)   cond="defined(__powerpc__)" ;; \
				riscv)     cond="defined(__riscv__)" ;; \
				s390)      cond="defined(__s390__) || defined(__s390x__)" ;; \
				sparc)     cond="defined(__sparc__)" ;; \
				x86)       cond="defined(__i386__) || defined(__i486__) || defined(__i586__) || defined(__i686__) || defined(__x86_64__)";; \
				*)         echo "Unsupported arch" >&2; exit 1;; \
			esac;\
			for file in "$(OUTPUT)sysroot/include/asm-arch-$$arch/"*.h; do \
				base="$${file##*/}"; \
				( echo "#if $$cond"; \
				  echo "#include \"../asm-arch-$$arch/$$base\""; \
				  echo "#endif" ) >> "$(OUTPUT)sysroot/include/asm/$$base"; \
			done; \
		fi;\
	done

clean:
	$(Q)rm -rf "$(OUTPUT)sysroot"
