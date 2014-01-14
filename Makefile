#
# Makefile
# Adrian Perez, 2010-09-18 20:56
#

SRCS := $(wildcard *.c) eggmarshalers.c
OBJS := $(patsubst %.c,%.o,$(SRCS))

PKG_MODULE := glib-2.0 gtk+-3.0 gtk+-x11-3.0 libwnck-3.0 keybinder-3.0 x11

PKG_CFLAGS := $(shell pkg-config $(PKG_MODULE) --cflags)
PKG_LDLIBS := $(shell pkg-config $(PKG_MODULE) --libs)

LDFLAGS   += $(PKG_LDLIBS)
CFLAGS    += $(PKG_CFLAGS) $(OPT_CFLAGS) \
             -DGSEAL_ENABLE \
						 -DGTK_DISABLE_DEPRECATED \
						 -DGDK_DISABLE_DEPRECATED \
						 -Wall -W -Wno-unused -std=gnu99

all: ggt

ggt: $(OBJS)

$(OBJS): eggmarshalers.h

clean:
	$(RM) $(OBJS) ggt eggmarshalers.h eggmarshalers.c


prefix  ?= $(HOME)/.local
DESTDIR ?=

install: ggt
	install -d $(DESTDIR)$(prefix)/bin
	install -m 755 ggt $(DESTDIR)$(prefix)/bin

eggmarshalers.h: eggmarshalers.list
	glib-genmarshal --header --internal --prefix=_egg_marshal $< > $@

eggmarshalers.c: eggmarshalers.list
	glib-genmarshal --body --internal --prefix=_egg_marshal $< > $@

# vim:ft=make
#

