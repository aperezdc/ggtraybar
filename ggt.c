/*
 * ggt.c
 * Copyright (C) 2010-2014 Adrian Perez <aperez@igalia.com>
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

#define WNCK_I_KNOW_THIS_IS_UNSTABLE /* Needed for Wnck :P */

#include "ggt.h"
#include "ggt-tray.h"
#include <libwnck/libwnck.h>
#include <keybinder.h>
#include <stdlib.h>
#include <string.h>


static GdkAtom a_NET_WM_STRUT_PARTIAL = 0;
static GdkAtom a_NET_WM_STRUT         = 0;
static GdkAtom a_CARDINAL             = 0;
static GdkAtom a_STRING               = 0;


static void
intern_atoms (void)
{
    a_NET_WM_STRUT_PARTIAL = gdk_atom_intern_static_string ("_NET_WM_STRUT_PARTIAL");
    a_NET_WM_STRUT         = gdk_atom_intern_static_string ("_NET_WM_STRUT");
    a_CARDINAL             = gdk_atom_intern_static_string ("CARDINAL");
    a_STRING               = gdk_atom_intern_static_string ("STRING");
}


static void
configure_window (GGTraybar *app)
{
    GtkWindow *window;

    g_assert (app);

    window = GTK_WINDOW (app->window);

    gtk_window_stick                 (window);
    gtk_window_set_title             (window, "GGTraybar");
    gtk_window_set_role              (window, "traybar");
    gtk_window_set_icon_name         (window, "gtk-preferences");
    gtk_window_set_wmclass           (window, "GGTraybar", "ggtraybar");
    gtk_window_set_type_hint         (window, GDK_WINDOW_TYPE_HINT_DOCK);
    gtk_window_set_resizable         (window, FALSE);
    gtk_window_set_decorated         (window, FALSE);
    gtk_window_set_focus_on_map      (window, FALSE);
    gtk_window_set_keep_above        (window, TRUE);
    gtk_window_set_skip_pager_hint   (window, TRUE);
    gtk_container_set_border_width   (GTK_CONTAINER (window), 0);
}


static inline void
set_window_properties (GGTraybar *app)
{
    GdkWindow *window;
    gulong data[NET_WM_STRUT_NELEM];
    gint window_height;

    g_assert (app);

    gtk_widget_set_size_request (app->window,
                                 app->primary_monitor.width,
                                 0);
    gtk_window_set_default_size (GTK_WINDOW (app->window),
                                 app->primary_monitor.width,
                                 0);

    gtk_window_get_size (GTK_WINDOW (app->window), NULL, &window_height);

    window = gtk_widget_get_window (GTK_WIDGET (app->window));
    memset (data, 0x00, sizeof (gulong) * NET_WM_STRUT_NELEM);

    data[NET_WM_STRUT_TOP]         = window_height;
    data[NET_WM_STRUT_TOP_START_X] = app->primary_monitor.x;
    data[NET_WM_STRUT_TOP_END_X]   = app->primary_monitor.x
                                   + app->primary_monitor.width - 1;

    gdk_property_change (window,
                         a_NET_WM_STRUT_PARTIAL,
                         a_CARDINAL, 32,
                         GDK_PROP_MODE_REPLACE,
                         (const guchar*) data,
                         NET_WM_STRUT_NELEM);

    gdk_property_change (window,
                         a_NET_WM_STRUT,
                         a_CARDINAL, 32,
                         GDK_PROP_MODE_REPLACE,
                         (const guchar*) data,
                         NET_WM_STRUT_COMPAT_NELEM);

    gdk_display_sync (gtk_widget_get_display (GTK_WIDGET (app->window)));

    gtk_window_move (GTK_WINDOW (app->window),
                     app->primary_monitor.x,
                     0);

    gtk_window_resize (GTK_WINDOW (app->window),
                       app->primary_monitor.width,
                       window_height);
}


static void
on_monitors_changed (GdkScreen *screen, gpointer data)
{
    GGTraybar *app = (GGTraybar*) data;

    g_assert (screen);
    g_assert (app);

    gdk_screen_get_monitor_geometry (screen,
                                     gdk_screen_get_primary_monitor (screen),
                                     &app->primary_monitor);

    set_window_properties (app);
}


static void
on_window_map_event (GtkWidget *window, GdkEvent *event, gpointer data)
{
    UNUSED (window);
    UNUSED (event);
    set_window_properties ((GGTraybar*) data);
}


int
main (int argc, char **argv)
{
    GGTraybar app;

    gtk_init (&argc, &argv);
    keybinder_init ();
    wnck_set_client_type (WNCK_CLIENT_TYPE_PAGER);

    memset (&app, 0x00, sizeof (GGTraybar));

    app.window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
    configure_window (&app);

    /*
     * Now create a GtkHBox where all gadgets will be added, and then
     * proceed to add all desired widgets to it. Note that:
     *
     *  - The set of available gadgets is fixed.
     *  - The layout of the gadgets is fixed.
     *  - The order of the gadgets is fixed.
     *
     * Why? Because I want them that way... and less is more :-)
     */
    app.content = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);

#define GADGET(_box, _pos, _gad)                                             \
    do {                                                                     \
        GtkWidget *gadget = (_gad);                                          \
        if (gadget)                                                          \
            gtk_box_pack_ ## _pos (GTK_BOX (_box), gadget, FALSE, FALSE, 0); \
    } while (0)

    GADGET (app.content, start, wnck_selector_new ());
    GADGET (app.content, start, ggt_winlist_new   (&app));
    GADGET (app.content, end,   ggt_clock_new     (&app));
    GADGET (app.content, end,   ggt_tray_new      ());
    /* GADGET (app.content, end,   ggt_pager_new    (&app)); */
    GADGET (app.content, start, ggt_launcher_new (&app));

    /*
     * Finished adding widgets to the panel.
     * Now add the container to the window.
     */
    gtk_container_add (GTK_CONTAINER (app.window),
                       GTK_WIDGET (app.content));

    /*
     * This makes the panel use the same GtkRc styles than those used by the
     * GNOME panel, so themes which modify the panel should blend finely.
     */
    gtk_widget_set_name (app.window, "PanelWidget");

    /*
     * We need X resources to get created so Gdk knows what to do to intern
     * atoms and setting properties: window needs to be realized first!
     */
    gtk_widget_realize  (app.window);
    intern_atoms        ();
    on_monitors_changed (gtk_widget_get_screen (app.window), &app);

    g_signal_connect (G_OBJECT (gtk_widget_get_screen (app.window)),
                      "monitors_changed",
                      G_CALLBACK (on_monitors_changed),
                      &app);

    g_signal_connect (G_OBJECT (app.window),
                      "map-event",
                      G_CALLBACK (on_window_map_event),
                      &app);

    gtk_widget_show_all (app.window);
    gtk_main ();

    return EXIT_SUCCESS;
}


