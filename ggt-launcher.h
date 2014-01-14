/*
 * ggt-launcher.h
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

#ifndef GGT_LAUNCHER_H
#define GGT_LAUNCHER_H

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define GGT_TYPE_LAUNCHER         (ggt_launcher_get_type ())
#define GGT_LAUNCHER(obj)         (G_TYPE_CHECK_INSTANCE_CAST ((obj), GGT_TYPE_LAUNCHER, GgtLauncher))
#define GGT_LAUNCHER_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), GGT_TYPE_LAUNCHER, GgtLauncherClass))
#define GGT_IS_LAUNCHER(obj)      (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GGT_TYPE_LAUNCHER))

typedef struct _GgtLauncherClass GgtLauncherClass;
typedef struct _GgtLauncher      GgtLauncher;

GType      ggt_launcher_get_type (void);
GtkWidget *ggt_launcher_new (const gchar *keystroke, GtkWindow *window);
void       ggt_launcher_hide (GgtLauncher *launcher);
void       ggt_launcher_show (GgtLauncher *launcher);

G_END_DECLS

#endif /* !GGT_LAUNCHER_H */
