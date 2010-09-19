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

#include <gtk/gtk.h>
#include <stdlib.h>
#include <string.h>
#include "ggt.h"

static GdkAtom a_NET_WM_STRUT_PARTIAL = 0;
static GdkAtom a_NET_WM_STRUT         = 0;
static GdkAtom a_XA_CARDINAL          = 0;


static void
intern_atoms (void)
{
    a_NET_WM_STRUT_PARTIAL = gdk_atom_intern_static_string ("_NET_WM_STRUT_PARTIAL");
    a_NET_WM_STRUT         = gdk_atom_intern_static_string ("_NET_WM_STRUT");
    a_XA_CARDINAL          = gdk_atom_intern_static_string ("XA_CARDINAL");
}


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
set_window_properties (GtkWindow *w)
{
    GdkWindow *window = gtk_widget_get_window (GTK_WIDGET (w));
    gulong data[NET_WM_STRUT_NELEM];

    memset (data, 0x00, sizeof (gulong) * NET_WM_STRUT_NELEM);

    data[NET_WM_STRUT_TOP]         = GGT_HEIGHT;
    data[NET_WM_STRUT_TOP_START_X] = 0;
    data[NET_WM_STRUT_TOP_END_X]   = gdk_screen_width ();

    gdk_property_change (window,
                         a_NET_WM_STRUT_PARTIAL,
                         a_XA_CARDINAL, 32,
                         GDK_PROP_MODE_REPLACE,
                         (const gchar*) data,
                         NET_WM_STRUT_NELEM);

    gdk_property_change (window,
                         a_NET_WM_STRUT,
                         a_XA_CARDINAL, 32,
                         GDK_PROP_MODE_REPLACE,
                         (const gchar*) data,
                         NET_WM_STRUT_COMPAT_NELEM);

    gtk_window_move (w, 0, 0);
    gdk_display_sync (gtk_widget_get_display (GTK_WIDGET (w)));
}



int
main (int argc, char **argv)
{
    GtkWidget *gadget = NULL;
    GtkWidget *hbox;

    ggtraybar_t app;

    g_type_init ();
    gtk_init (&argc, &argv);

    memset (&app, 0x00, sizeof (ggtraybar_t));

    app.window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
    app.screen = gtk_widget_get_screen (app.window);

    configure_window (GTK_WINDOW (app.window));

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
    hbox = gtk_hbox_new (FALSE, 5);


    /* Add global menu widget. */
    if ((gadget = ggt_globalmenu_init (&app)))
        gtk_box_pack_start (GTK_BOX (hbox), gadget, FALSE, FALSE, 0);

    /* Add tray widget, if it can be initialized. */
    if ((gadget = ggt_tray_init (&app)))
        gtk_box_pack_end (GTK_BOX (hbox), gadget, FALSE, FALSE, 0);


    /*
     * Finished adding widgets to the panel.
     * Now add the container to the window.
     */
    gtk_container_add (GTK_CONTAINER (app.window),
                       GTK_WIDGET (hbox));

    gtk_widget_set_size_request (app.window,
                                 gdk_screen_width (),
                                 GGT_HEIGHT);

    /*
     * This makes the panel use the same GtkRc styles than those used by the
     * GNOME panel, so themes which modify the panel should blend finely.
     */
    gtk_widget_set_name (app.window, "PanelWidget");

    /*
     * After having the window mapped, we are sure that GDK_SCREEN()
     * will return something sane, so intern atoms and set X properties.
     */
    gtk_widget_show_all (app.window);
    intern_atoms ();
    set_window_properties (GTK_WINDOW (app.window));

    gtk_main ();

    return EXIT_SUCCESS;
}


