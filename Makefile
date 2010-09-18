#
# Makefile
# Adrian Perez, 2010-09-18 20:56
#

PKG_CFLAGS := $(shell pkg-config gtk+-2.0 globalmenu-server --cflags)
PKG_LDLIBS := $(shell pkg-config gtk+-2.0 globalmenu-server --libs)

LDFLAGS   += $(subst -lserver,-lglobalmenu-server,$(PKG_LDLIBS))
CFLAGS    += $(PKG_CFLAGS)

all: ggtraybar

clean:
	$(RM) ggtraybar.o ggtraybar

# vim:ft=make
#

