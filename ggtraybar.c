/*
 * ggtraybar.c
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

#include "eggtraymanager.h"
#include <gnome-globalmenu/globalmenu-server.h>
#include <gtk/gtk.h>
#include <stdlib.h>
#include <X11/X.h>
#include <X11/Xatom.h>


#ifndef GGTRAYBAR_HEIGHT
#define GGTRAYBAR_HEIGHT 24
#endif /* !GGTRAYBAR_HEIGHT */


static Atom a_NET_WM_STRUT_PARTIAL = 0;
static Atom a_NET_WM_STRUT         = 0;


static void
intern_x_atoms (void)
{
    a_NET_WM_STRUT_PARTIAL = XInternAtom (GDK_DISPLAY (), "_NET_WM_STRUT_PARTIAL", False);
    a_NET_WM_STRUT         = XInternAtom (GDK_DISPLAY (), "_NET_WM_STRUT",         False);
}


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


static void
configure_window (GtkWindow *w)
{
    g_assert (w);

    gtk_window_stick                 (w);
    gtk_window_set_title             (w, "Menu");
    gtk_window_set_role              (w, "traybar");
    gtk_window_set_icon_name         (w, "gtk-preferences");
    gtk_window_set_wmclass           (w, "GGTrayBar", "ggtraybar");
    gtk_window_set_type_hint         (w, GDK_WINDOW_TYPE_HINT_DOCK);
    gtk_window_set_resizable         (w, FALSE);
    gtk_window_set_decorated         (w, FALSE);
    gtk_window_set_focus_on_map      (w, FALSE);
    gtk_window_set_keep_above        (w, TRUE);
    gtk_window_set_skip_pager_hint   (w, TRUE);
    gtk_window_set_skip_taskbar_hint (w, TRUE);
    gtk_container_set_border_width   (GTK_CONTAINER (w), 0);
}


static void
set_xwin_properties (GtkWindow *w)
{
    Window xwin = GDK_WINDOW_XWINDOW (gtk_widget_get_window (GTK_WIDGET (w)));
    gulong data[NET_WM_STRUT_NELEM];

    memset (data, 0x00, sizeof (gulong) * NET_WM_STRUT_NELEM);

    data[NET_WM_STRUT_TOP]         = GGTRAYBAR_HEIGHT;
    data[NET_WM_STRUT_TOP_START_X] = 0;
    data[NET_WM_STRUT_TOP_END_X]   = gdk_screen_width ();

    XChangeProperty (GDK_DISPLAY (),
                     xwin,
                     a_NET_WM_STRUT_PARTIAL,
                     XA_CARDINAL,
                     32,
                     PropModeReplace,
                     (unsigned char*) data,
                     NET_WM_STRUT_NELEM);

    XChangeProperty (GDK_DISPLAY (),
                     xwin,
                     a_NET_WM_STRUT,
                     XA_CARDINAL,
                     32,
                     PropModeReplace,
                     (unsigned char*) data,
                     NET_WM_STRUT_COMPAT_NELEM);

    XMoveWindow (GDK_DISPLAY (), xwin, 0, 0);
    XSync (GDK_DISPLAY (), False);

    gtk_window_move (w, 0, 0);
}



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

    traybox = gtk_hbox_new (TRUE, 2);

    g_signal_connect (traymgr,
                      "tray_icon_added",
                      G_CALLBACK (tray_added),
                      traybox);

    g_signal_connect (traymgr,
                      "tray_icon_removed",
                      G_CALLBACK (tray_removed),
                      traybox);

    g_signal_connect (traymgr,
                      "message_sent",
                      G_CALLBACK (message_sent),
                      NULL);

    g_signal_connect (traymgr,
                      "message_cancelled",
                      G_CALLBACK (message_cancel),
                      NULL);

    return traybox;
}



int
main (int argc, char **argv)
{
    GnomenuGlobalMenuBar *menubar;
    GtkWidget *window;
    GtkWidget *hbox;
    GdkScreen *screen;

    g_type_init ();
    gtk_init (&argc, &argv);

    window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
    screen = gtk_widget_get_screen (window);

    configure_window (GTK_WINDOW (window));

    menubar = gnomenu_global_menu_bar_new ();

    hbox = gtk_hbox_new (FALSE, 5);

    gtk_box_pack_start (GTK_BOX (hbox),
                        GTK_WIDGET (menubar),
                        FALSE,
                        FALSE,
                        0);

    if (egg_tray_manager_check_running (screen)) {
        g_warning ("Another tray manager already running, tray disabled.\n");
    }
    else {
        EggTrayManager *traymgr = egg_tray_manager_new ();
        if (!egg_tray_manager_manage_screen (traymgr, screen))
            g_printerr ("Tray manager could not manage screen");
        gtk_box_pack_end (GTK_BOX (hbox),
                          make_traybar_widget (traymgr),
                          FALSE,
                          FALSE,
                          0);
    }

    gtk_container_add (GTK_CONTAINER (window),
                       GTK_WIDGET (hbox));

    gtk_widget_set_size_request (window,
                                 gdk_screen_width (),
                                 GGTRAYBAR_HEIGHT);

    gtk_widget_set_name (window, "PanelWidget");
    gtk_widget_show_all (window);

    intern_x_atoms ();
    set_xwin_properties (GTK_WINDOW (window));

    gtk_main ();

    return EXIT_SUCCESS;
}


