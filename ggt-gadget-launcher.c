/*
 * ggt-gadget-launcher.c
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

#include "ggt.h"
#include <keybinder.h>
#include <gdk/gdkkeysyms.h>


static GQuark q_launcher_widget = 0;
static GQuark q_entry_widget    = 0;


static void
hide_launcher (GGTraybar *app, GtkWidget *launcher)
{
    GtkContainer *container;

    g_assert (app);
    g_assert (launcher);

    container = GTK_CONTAINER (gtk_bin_get_child (GTK_BIN (app->window)));

    gtk_widget_hide (launcher);

    gtk_container_remove (container, launcher);
    gtk_box_pack_start (GTK_BOX (container), app->content, TRUE, TRUE, 0);
    gtk_widget_show_all (app->window);
    gtk_grab_remove (app->window);
}


static void
on_entry_activate (GtkEntry *entry, GGTraybar *app)
{
    g_assert (entry);
    g_assert (app);

    g_spawn_command_line_async (gtk_entry_get_text (entry), NULL);
    hide_launcher (app, GTK_WIDGET (g_object_get_qdata (G_OBJECT (entry),
                                                        q_launcher_widget)));
}


static void
on_key_activated (const char *keystring, void *user_data)
{
    GGTraybar *app = (GGTraybar*) user_data;
    GtkContainer *container;
    GtkWidget *entry;

    g_assert (app);
    (void) keystring;

    entry = GTK_WIDGET (g_object_get_qdata (G_OBJECT (app->window), q_entry_widget));
    container = GTK_CONTAINER (gtk_bin_get_child (GTK_BIN (app->window)));

    gtk_widget_hide (app->content);
    gtk_container_remove (container, app->content);

    gtk_box_pack_start (GTK_BOX (container),
                        GTK_WIDGET (g_object_get_qdata (G_OBJECT (app->window),
                                                        q_launcher_widget)),
                        TRUE,
                        TRUE,
                        0);

    gtk_widget_show_all (app->window);

    gtk_window_set_accept_focus (GTK_WINDOW (app->window), TRUE);
    gtk_window_set_focus (GTK_WINDOW (app->window), entry);
    gtk_widget_grab_focus (entry);
    gtk_window_present (GTK_WINDOW (app->window));
    gtk_grab_add (app->window);
}


static gboolean
on_entry_key_release (GtkWidget *entry, GdkEventKey *event, GGTraybar *app)
{
    g_assert (entry);
    g_assert (event);
    g_assert (app);

    if (event->keyval == GDK_KEY_Escape) {
        hide_launcher (app, GTK_WIDGET (g_object_get_qdata (G_OBJECT (entry),
                                                            q_launcher_widget)));
        /* Inhibit event propagation */
        return TRUE;
    }

    return FALSE;
}



GtkWidget*
ggt_launcher_init (GGTraybar *app)
{
    GtkWidget *hbox   = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 5);
    GtkWidget *label  = gtk_label_new ("Command:");
    GtkWidget *entry  = gtk_entry_new ();

    g_assert (app);

    q_launcher_widget = g_quark_from_static_string ("ggt-launcher-widget");
    q_entry_widget    = g_quark_from_static_string ("ggt-entry-widget");

    gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 0);
    gtk_box_pack_start (GTK_BOX (hbox), entry, TRUE, TRUE, 0);

    g_object_set_qdata (G_OBJECT (app->window), q_entry_widget,   entry);
    g_object_set_qdata (G_OBJECT (app->window), q_launcher_widget, hbox);
    g_object_set_qdata (G_OBJECT (entry),       q_launcher_widget, hbox);

    g_signal_connect (G_OBJECT (entry), "activate",
                      G_CALLBACK (on_entry_activate), app);

    g_signal_connect (G_OBJECT (entry), "key-release-event",
                      G_CALLBACK (on_entry_key_release), app);

    if (!keybinder_bind ("<Alt>F2", on_key_activated, app))
        g_printerr ("Could not bind <Alt>F2 keystroke\n");

    /*
     * XXX Surely this is not elegant, but currently I do not know a better
     *     way of making black-on-white with some bogus themes (e.g. Shiki)
     *     for which theme-assigned colors feel right.
     *
     * FIXME Change this to use GtkCssProvider
     */
#if 0
    gtk_rc_parse_string ("style \"launchentry\" {\n"
                         "  fg[NORMAL] = \"#fff\"\n"
                         "  text[NORMAL] = \"#000\"\n"
                         "}\n"
                         "widget \"*ggt-launcher-entry\" style \"launchentry\"");
#endif

    gtk_widget_set_name (entry, "ggt-launcher-entry");
    gtk_widget_add_events (entry, GDK_KEY_PRESS_MASK);

    return NULL;
}
