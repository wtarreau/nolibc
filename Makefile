TOPDIR     := $(PWD)
DESTDIR    :=
PREFIX     := /usr/local

CROSS_COMPILE :=

CC         := $(CROSS_COMPILE)gcc
OPT_CFLAGS := -Os
CPU_CFLAGS := -fno-asynchronous-unwind-tables -include $(TOPDIR)/nolibc.h
DEB_CFLAGS := -W -Wall -Wextra -g
DEF_CFLAGS :=
USR_CFLAGS :=
INC_CFLAGS :=
CFLAGS     := $(OPT_CFLAGS) $(CPU_CFLAGS) $(DEB_CFLAGS) $(DEF_CFLAGS) $(USR_CFLAGS) $(INC_CFLAGS)

LD         := $(CC)
DEB_LFLAGS := -g
USR_LFLAGS :=
LIB_LFLAGS := -lgcc
CPU_LFLAGS := -nostdlib -static
LDFLAGS    := $(DEB_LFLAGS) $(USR_LFLAGS) $(CPU_LFLAGS)

STRIP      := $(CROSS_COMPILE)strip
EXAMPLES   := hello.stripped
OBJS       :=
OBJS       += hello.o

all: $(EXAMPLES)

$(EXAMPLES): %.stripped: %.bin
	$(STRIP) -R .comment -o $@ $<

%.bin: %.o
	$(LD) $(LDFLAGS) -o $@ $< $(LIB_LFLAGS)

%.o: %.c nolibc.h
	$(CC) $(CFLAGS) -o $@ -c $<

clean:
	-rm -f $(OBJS) $(EXAMPLES) *.bin *.stripped *.[oa] *~ */*.[oa] */*~ core a.out
