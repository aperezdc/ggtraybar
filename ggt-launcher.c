/*
 * ggt-launcher.c
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

#include "ggt.h"
#include "ggt-launcher.h"
#include <keybinder.h>

struct _GgtLauncher {
    GObject    parent_instance;
    GtkWidget *widget;
    GtkWidget *window;
    GtkWidget *window_child;
    GtkWidget *entry;
};

struct _GgtLauncherClass {
    GtkBinClass parent_class;
};

G_DEFINE_TYPE (GgtLauncher, ggt_launcher, G_TYPE_OBJECT)


static void
ggt_launcher_finalize (GObject *object)
{
    GgtLauncher *launcher = GGT_LAUNCHER (object);
    g_object_unref (launcher->widget);
    g_object_unref (launcher->window);
    g_object_unref (launcher->window_child);
    G_OBJECT_CLASS (ggt_launcher_parent_class)->finalize (object);
}


static void
ggt_launcher_class_init (GgtLauncherClass *klass)
{
    G_OBJECT_CLASS (klass)->finalize = ggt_launcher_finalize;
}


static void
on_entry_activate (GtkEntry *entry, GgtLauncher *launcher)
{
    g_spawn_command_line_async (gtk_entry_get_text (entry), NULL);
    ggt_launcher_hide (launcher);
}


static gboolean
on_entry_key_release (GtkWidget *entry, GdkEventKey *event, GgtLauncher *launcher)
{
    UNUSED (entry);

    if (event->keyval == GDK_KEY_Escape) {
        ggt_launcher_hide (launcher);
        return TRUE;
    }

    return FALSE;
}


static void
ggt_launcher_init (GgtLauncher *launcher)
{
    GtkWidget *label = gtk_label_new ("Command:");
    launcher->widget = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 5);
    launcher->entry  = gtk_entry_new ();

    g_object_ref (launcher->widget);

    gtk_container_set_border_width (GTK_CONTAINER (launcher->widget), 2);
    gtk_box_pack_start (GTK_BOX (launcher->widget), label, FALSE, FALSE, 0);
    gtk_box_pack_start (GTK_BOX (launcher->widget), launcher->entry, TRUE, TRUE, 0);

    g_signal_connect (launcher->entry,
                      "activate",
                      G_CALLBACK (on_entry_activate),
                      launcher);

    g_signal_connect (launcher->entry,
                      "key-release-event",
                      G_CALLBACK (on_entry_key_release),
                      launcher);

    gtk_widget_set_name (launcher->widget, "ggt-launcher");
    gtk_widget_set_name (launcher->entry,  "ggt-launcher-entry");
    gtk_widget_set_name (label,            "ggt-launcher-label");
    gtk_widget_add_events (launcher->entry, GDK_KEY_PRESS_MASK | GDK_KEY_RELEASE_MASK);
}


static void
on_key_activated (const char *keystroke, void *user_data)
{
    UNUSED (keystroke);
    ggt_launcher_show (GGT_LAUNCHER (user_data));
}


GtkWidget*
ggt_launcher_new (const gchar *keystroke, GtkWindow *window)
{
    g_return_val_if_fail (keystroke != NULL, NULL);
    g_return_val_if_fail (GTK_IS_WINDOW (window), NULL);

    GgtLauncher *launcher = GGT_LAUNCHER (g_object_new (GGT_TYPE_LAUNCHER, NULL));
    launcher->window = g_object_ref (window);

    if (!keybinder_bind (keystroke, on_key_activated, launcher))
        g_printerr ("Could not bind '%s' keystroke", keystroke);

    return GTK_WIDGET (launcher);
}


void
ggt_launcher_show (GgtLauncher *launcher)
{
    g_return_if_fail (GGT_IS_LAUNCHER (launcher));
    g_return_if_fail (launcher->window_child == NULL);

    launcher->window_child = gtk_bin_get_child (GTK_BIN (launcher->window));
    g_object_ref (launcher->window_child);
    gtk_widget_hide (launcher->window_child);
    gtk_container_remove (GTK_CONTAINER (launcher->window), launcher->window_child);
    gtk_container_add (GTK_CONTAINER (launcher->window), launcher->widget);
    gtk_widget_reparent (launcher->widget, launcher->window);
    gtk_widget_show_all (launcher->window);
    gtk_window_set_accept_focus (GTK_WINDOW (launcher->window), TRUE);
    gtk_window_set_focus (GTK_WINDOW (launcher->window), launcher->entry);
    gtk_widget_grab_focus (launcher->entry);
    gtk_window_present (GTK_WINDOW (launcher->window));
    gtk_grab_add (launcher->window);
}


void
ggt_launcher_hide (GgtLauncher *launcher)
{
    g_return_if_fail (GGT_IS_LAUNCHER (launcher));
    g_return_if_fail (launcher->window_child != NULL);

    gtk_widget_hide (launcher->widget);
    gtk_container_remove (GTK_CONTAINER (launcher->window), launcher->widget);
    gtk_container_add (GTK_CONTAINER (launcher->window), launcher->window_child);
    gtk_widget_reparent (launcher->window_child, launcher->window);
    gtk_widget_show_all (launcher->window);
    gtk_grab_remove (launcher->window);
    g_object_unref (launcher->window_child);
    launcher->window_child = NULL;
}
