/*
 * ggt-tray.c
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

#include "ggt-tray.h"
#include "eggtraymanager.h"

struct _GgtTray {
    GtkBin          parent_instance;
    EggTrayManager *manager;
    GtkWidget      *box;
};

struct _GgtTrayClass {
    GtkBinClass parent_class;
};

G_DEFINE_TYPE (GgtTray, ggt_tray, GTK_TYPE_BIN)


static void
tray_changed (GgtTray *tray)
{
    GtkAllocation allocation;

    gtk_widget_get_allocation (tray->box, &allocation);
    gtk_widget_set_size_request (tray->box,
                                 allocation.width,
                                 allocation.height);

    gtk_widget_hide (tray->box);

    while (gtk_events_pending ())
        gtk_main_iteration ();

    gtk_widget_show (tray->box);
}


static void
tray_icon_added (EggTrayManager *manager, GtkWidget *icon, GgtTray *tray)
{
    gtk_box_pack_end (GTK_BOX (tray->box), icon, FALSE, FALSE, 0);
    gtk_widget_show (icon);
    tray_changed (tray);
}


static void
tray_icon_removed (EggTrayManager *manager, GtkWidget *icon, GgtTray *tray)
{
    tray_changed (tray);
}


static void
ggt_tray_finalize (GObject *object)
{
    GgtTray *tray = GGT_TRAY (object);
    g_object_unref (tray->manager);
    g_object_unref (tray->box);
}

static void
ggt_tray_init (GgtTray *tray)
{
    GdkScreen *screen;

    gtk_container_set_border_width (GTK_CONTAINER (tray), 4);
    tray->box = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 1);
    gtk_container_add (GTK_CONTAINER (tray), tray->box);

    screen = gtk_widget_get_screen (tray->box);
    if (egg_tray_manager_check_running (screen)) {
        g_warning ("Another tray manager is already running!");
        return;
    }

    tray->manager = egg_tray_manager_new ();
    if (!egg_tray_manager_manage_screen (tray->manager, screen))
        g_printerr ("EggTrayManager failed to manage the screen");

    g_signal_connect (tray->manager,
                      "tray-icon-added",
                      G_CALLBACK (tray_icon_added),
                      tray);

    g_signal_connect (tray->manager,
                      "tray-icon-removed",
                      G_CALLBACK (tray_icon_removed),
                      tray);
}


static void
count_children (GtkWidget *widget, gpointer data)
{
    guint *nitems = (guint*) data;
    (*nitems)++;
}


static void
ggt_tray_get_preferred_width (GtkWidget *widget,
                              gint      *minimum,
                              gint      *natural)
{
    GgtTray *tray = GGT_TRAY (widget);
    guint nitems = 0;

    gtk_container_foreach (GTK_CONTAINER (tray->box),
                           count_children,
                           &nitems);

    *minimum = *natural = 0;
    if (nitems > 0)
        *minimum = *natural = (16 * nitems) + (gtk_box_get_spacing (GTK_BOX (tray->box)) * (nitems - 1));
}


static void
ggt_tray_get_preferred_height (GtkWidget *widget,
                               gint      *minimum,
                               gint      *natural)
{
    GgtTray *tray = GGT_TRAY (widget);
    *minimum = *natural = 16;
}


static void
ggt_tray_class_init (GgtTrayClass *klass)
{
    GTK_WIDGET_CLASS (klass)->get_preferred_height = ggt_tray_get_preferred_height;
    GTK_WIDGET_CLASS (klass)->get_preferred_width = ggt_tray_get_preferred_width;
    G_OBJECT_CLASS (klass)->finalize = ggt_tray_finalize;
}


GtkWidget*
ggt_tray_new (void)
{
    return GTK_WIDGET (g_object_new (GGT_TYPE_TRAY, NULL));
}
