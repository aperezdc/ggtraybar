/*
 * ggt.c
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
#include <stdlib.h>
#include <string.h>


static GdkAtom a_GLOBALMENU_SETTINGS  = 0;
static GdkAtom a_NET_WM_STRUT_PARTIAL = 0;
static GdkAtom a_NET_WM_STRUT         = 0;
static GdkAtom a_CARDINAL             = 0;
static GdkAtom a_STRING               = 0;


#define GLOBALMENU_ROOT_WINDOW_PAYLOAD "\n" \
    "[GlobalMenu:Client]\n"                 \
    "show-local-menu=false\n"               \
    "show-menu-icons=true\n"                \
    "changed-notify-timeout=500\n"


static void
intern_atoms (void)
{
    a_GLOBALMENU_SETTINGS  = gdk_atom_intern_static_string ("_NET_GLOBALMENU_SETTINGS");
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
    gtk_window_set_title             (window, "Menu");
    gtk_window_set_role              (window, "traybar");
    gtk_window_set_icon_name         (window, "gtk-preferences");
    gtk_window_set_wmclass           (window, "GGTraybar", "ggtraybar");
    gtk_window_set_type_hint         (window, GDK_WINDOW_TYPE_HINT_DOCK);
    gtk_window_set_resizable         (window, FALSE);
    gtk_window_set_decorated         (window, FALSE);
    gtk_window_set_focus_on_map      (window, FALSE);
    gtk_window_set_keep_above        (window, TRUE);
    gtk_window_set_skip_pager_hint   (window, TRUE);
    gtk_window_set_skip_taskbar_hint (window, TRUE);
    gtk_container_set_border_width   (GTK_CONTAINER (window), 0);
}


static void
set_window_properties (GGTraybar *app)
{
    GdkWindow *window;
    gulong data[NET_WM_STRUT_NELEM];

    g_assert (app);

    window = gtk_widget_get_window (GTK_WIDGET (app->window));
    memset (data, 0x00, sizeof (gulong) * NET_WM_STRUT_NELEM);

    data[NET_WM_STRUT_TOP]         = GGT_HEIGHT;
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

    gdk_property_change (gdk_get_default_root_window (),
                         a_GLOBALMENU_SETTINGS,
                         a_STRING, 8,
                         GDK_PROP_MODE_REPLACE,
                         (const guchar*) GLOBALMENU_ROOT_WINDOW_PAYLOAD,
                         sizeof (GLOBALMENU_ROOT_WINDOW_PAYLOAD));

    gtk_window_move (GTK_WINDOW (app->window), app->primary_monitor.x, 0);
    gdk_display_sync (gtk_widget_get_display (GTK_WIDGET (app->window)));
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

    gtk_widget_set_size_request (app->window,
                                 app->primary_monitor.width,
                                 GGT_HEIGHT);

    set_window_properties (app);
}



int
main (int argc, char **argv)
{
    GtkWidget *hbox;
    GGTraybar app;

    g_type_init ();
    gtk_init (&argc, &argv);

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
    hbox = gtk_hbox_new (FALSE, 0);


#define GADGET(_box, _pos, _gad)                                             \
    do {                                                                     \
        GtkWidget *gadget = (_gad);                                          \
        if (gadget)                                                          \
            gtk_box_pack_ ## _pos (GTK_BOX (_box), gadget, FALSE, FALSE, 0); \
    } while (0)

    app.content = ggt_globalmenu_init (&app);
    g_object_ref (app.content);

    GADGET (hbox, start, app.content);
    GADGET (hbox, end,   ggt_clock_init    (&app));
    GADGET (hbox, end,   ggt_tray_init     (&app));
    GADGET (hbox, end,   ggt_pager_init    (&app));
    GADGET (hbox, start, ggt_launcher_init (&app));

    g_object_ref (app.content);

    /*
     * Finished adding widgets to the panel.
     * Now add the container to the window.
     */
    gtk_container_add (GTK_CONTAINER (app.window),
                       GTK_WIDGET (hbox));

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
    gtk_widget_show_all (app.window);

    g_signal_connect (G_OBJECT (gtk_widget_get_screen (app.window)),
                      "monitors_changed",
                      G_CALLBACK (on_monitors_changed),
                      &app);

    gtk_main ();

    return EXIT_SUCCESS;
}


