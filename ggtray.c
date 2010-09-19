/*
 * ggtray.c
 * Copyright (C) 2010 Adrian Perez <aperez@igalia.com>
 *
 * Distributed under terms of the MIT license.
 */

#include "ggtraybar.h"
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


static GtkWidget*
make_traybar_widget (EggTrayManager *traymgr)
{
    GtkWidget *traybox;

    g_assert (traymgr);

}


GtkWidget*
gg_tray_init (ggtraybar_t *app)
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

    g_object_set_data (gadget, "traymanager", traymgr);

    return gadget;
}

