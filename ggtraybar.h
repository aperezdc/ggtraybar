/*
 * ggtraybar.h
 * Copyright (C) 2010 Adrian Perez <aperez@igalia.com>
 *
 * Distributed under terms of the MIT license.
 */

#ifndef __ggtraybar_h__
#define __ggtraybar_h__

#ifndef GGTRAYBAR_HEIGHT
#define GGTRAYBAR_HEIGHT 24
#endif /* !GGTRAYBAR_HEIGHT */

/*
 * Defines for setting _NET_WM_STRUT(_PARTIAL)
 */
#define NET_WM_STRUT_LEFT             0
#define NET_WM_STRUT_RIGHT            1
#define NET_WM_STRUT_TOP              2
#define NET_WM_STRUT_BOTTOM           3
#define NET_WM_STRUT_LEFT_START_Y     4
#define NET_WM_STRUT_LEFT_END_Y       5
#define NET_WM_STRUT_RIGHT_START_Y    6
#define NET_WM_STRUT_RIGHT_END_Y      7
#define NET_WM_STRUT_TOP_START_X      8
#define NET_WM_STRUT_TOP_END_X        9
#define NET_WM_STRUT_BOTTOM_START_X  10
#define NET_WM_STRUT_BOTTOM_END_X    11
#define NET_WM_STRUT_NELEM           12
#define NET_WM_STRUT_COMPAT_NELEM     4


#include <gtk/gtk.h>


struct ggtraybar_s {
    GtkWidget *window;
    GdkScreen *screen;
};
typedef struct ggtraybar_s ggtraybar_t;


GtkWidget* gg_tray_init (ggtraybar_t *app);


#endif /* !__ggtraybar_h__ */

