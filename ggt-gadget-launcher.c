/*
 * ggt-gadget-launcher.c
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



static void
on_entry_activate (GtkEntry *entry, void *data)
{
    g_assert (entry);
    g_spawn_command_line_async (gtk_entry_get_text (entry), NULL);
}


static void
on_entry_show (GtkWidget *entry, void *data)
{
    g_assert (entry);
    gtk_entry_set_text (GTK_ENTRY (entry), "");
    gtk_widget_grab_focus (entry);
}


GtkWidget*
ggt_launcher_init (ggtraybar_t *app)
{
    GtkWidget *hbox  = gtk_hbox_new (FALSE, 5);
    GtkWidget *label = gtk_label_new ("Command:");
    GtkWidget *entry = gtk_entry_new ();

    g_assert (app);

    gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 0);
    gtk_box_pack_start (GTK_BOX (hbox), entry, TRUE, TRUE, 0);

    g_signal_connect (G_OBJECT (entry), "activate",
                      G_CALLBACK (on_entry_activate),
                      NULL);

    g_signal_connect (G_OBJECT (entry), "show",
                      G_CALLBACK (on_entry_show),
                      NULL);

    return hbox;
}

