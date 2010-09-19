#
# Makefile
# Adrian Perez, 2010-09-18 20:56
#

SRCS := $(wildcard *.c)
OBJS := $(patsubst %.c,%.o,$(SRCS))

PKG_MODULE := glib-2.0 gtk+-2.0 globalmenu-server libwnck-1.0

PKG_CFLAGS := $(shell pkg-config $(PKG_MODULE) --cflags)
PKG_LDLIBS := $(shell pkg-config $(PKG_MODULE) --libs)

LDFLAGS   += $(subst -lserver,-lglobalmenu-server,$(PKG_LDLIBS))
CFLAGS    += $(PKG_CFLAGS) \
						 -DGSEAL_ENABLE \
						 -DGTK_DISABLE_DEPRECATED \
						 -DGDK_DISABLE_DEPRECATED

all: ggt

ggt: $(OBJS)

clean:
	$(RM) $(OBJS) ggt

# vim:ft=make
#

