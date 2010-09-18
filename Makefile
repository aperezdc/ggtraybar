#
# Makefile
# Adrian Perez, 2010-09-18 20:56
#

SRCS := $(wildcard *.c)
OBJS := $(patsubst %.c,%.o,$(SRCS))

PKG_CFLAGS := $(shell pkg-config glib-2.0 gtk+-2.0 globalmenu-server --cflags)
PKG_LDLIBS := $(shell pkg-config glib-2.0 gtk+-2.0 globalmenu-server --libs)

LDFLAGS   += $(subst -lserver,-lglobalmenu-server,$(PKG_LDLIBS))
CFLAGS    += $(PKG_CFLAGS)

all: ggtraybar

ggtraybar: $(OBJS)

clean:
	$(RM) ggtraybar $(OBJS)

# vim:ft=make
#

