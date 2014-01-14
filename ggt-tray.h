/*
 * ggt-tray.h
 * Copyright (C) 2014 Adrian Perez <aperez@igalia.com>
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

#ifndef GGT_TRAY_H
#define GGT_TRAY_H

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define GGT_TYPE_TRAY         (ggt_tray_get_type ())
#define GGT_TRAY(obj)         (G_TYPE_CHECK_INSTANCE_CAST ((obj), GGT_TYPE_TRAY, GgtTray))
#define GGT_TRAY_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), GGT_TYPE_TRAY, GgtTrayClass)
#define GGT_IS_TRAY(obj)      (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GGT_TYPE_TRAY))

typedef struct _GgtTrayClass GgtTrayClass;
typedef struct _GgtTray      GgtTray;

GType      ggt_tray_get_type (void);
GtkWidget *ggt_tray_new      (void);

G_END_DECLS

#endif /* !GGT_TRAY_H */
