/*
 * ggt-gadget-tray.c
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

#include "ggt.h"
#include "eggtraymanager.h"


static void
tray_changed (GtkWidget *widget)
{
    gtk_widget_set_size_request (widget,
                                 widget->allocation.width,
                                 widget->allocation.height);

    gtk_widget_hide (widget);

    while (gtk_events_pending ())
        gtk_main_iteration ();

    gtk_widget_show (widget);
    gtk_widget_set_size_request (widget, -1, -1);
}


static void
tray_added (EggTrayManager *traymgr, GtkWidget *icon, void *priv)
{
    GtkBox *box = GTK_BOX (priv);

    g_assert (traymgr);
    g_assert (icon);
    g_assert (priv);

    gtk_box_pack_end (box, icon, FALSE, FALSE, 0);
    gtk_widget_show (icon);
    gdk_display_sync (gtk_widget_get_display (icon));

    tray_changed (GTK_WIDGET (box));
}


static void
tray_removed (EggTrayManager *traymgr, GtkWidget *icon, void *priv)
{
    GtkBox *box = GTK_BOX (priv);

    g_assert (traymgr);
    g_assert (icon);
    g_assert (priv);

    tray_changed (GTK_WIDGET (box));
}


static void
message_sent (EggTrayManager *traymgr,
              GtkWidget      *icon,
              const char     *text,
              glong           mid,
              glong           timeout,
              void           *data)
{
}


static void
message_cancel (EggTrayManager *traymgr,
                GtkWidget      *icon,
                glong           mid,
                void           *data)
{
}




GtkWidget*
ggt_tray_init (ggtraybar_t *app)
{
    EggTrayManager *traymgr;
    GtkWidget *gadget;

    g_assert (app);
    g_assert (app->screen);

    if (egg_tray_manager_check_running (app->screen)) {
        g_warning ("Another tray manager already running, disabling.");
        return NULL;
    }

    traymgr = egg_tray_manager_new ();
    if (!egg_tray_manager_manage_screen (traymgr, app->screen))
        g_printerr ("Tray manager could not manage screen.");

    gadget = gtk_hbox_new (TRUE, 1);

    g_signal_connect (traymgr,
                      "tray_icon_added",
                      G_CALLBACK (tray_added),
                      gadget);

    g_signal_connect (traymgr,
                      "tray_icon_removed",
                      G_CALLBACK (tray_removed),
                      gadget);

    g_signal_connect (traymgr,
                      "message_sent",
                      G_CALLBACK (message_sent),
                      gadget);

    g_signal_connect (traymgr,
                      "message_cancelled",
                      G_CALLBACK (message_cancel),
                      gadget);

    g_object_set_data (G_OBJECT (gadget),
                       "traymanager",
                       traymgr);

    return gadget;
}

