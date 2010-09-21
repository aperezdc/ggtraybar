/*
 * ggt.h
 * Copyright (C) 2010 Adrian Perez <aperez@igalia.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#ifndef __ggtraybar_h__
#define __ggtraybar_h__

#ifndef GGT_HEIGHT
#define GGT_HEIGHT 24
#endif /* !GGT_HEIGHT */

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
#include <gdk/gdk.h>


struct ggtraybar_s {
    GtkWidget    *window;
    GdkRectangle  primary_monitor;
};
typedef struct ggtraybar_s ggtraybar_t;


GtkWidget* ggt_tray_init       (ggtraybar_t *app);
GtkWidget* ggt_globalmenu_init (ggtraybar_t *app);
GtkWidget* ggt_clock_init      (ggtraybar_t *app);
GtkWidget* ggt_pager_init      (ggtraybar_t *app);


#endif /* !__ggtraybar_h__ */

