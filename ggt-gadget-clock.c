/*
 * ggt-gadget-clock.c
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
#include <time.h>
#include <sys/time.h>
#include <sys/types.h>


static gint
clock_tick_update (void *data)
{
    gchar buf[20];
    time_t tnow;
    struct tm *now;

    g_assert (data);

    time (&tnow);
    now = localtime (&tnow);
    strftime (buf, 20, "%H:%M", now);

    gtk_button_set_label (GTK_BUTTON (data), buf);

    return TRUE;
}


static gboolean
on_click_show_calendar (GtkWidget *widget, GdkEventButton *event, void *priv)
{
    return TRUE;
}


GtkWidget*
ggt_clock_init (ggtraybar_t *app)
{
    GtkWidget *button = gtk_toggle_button_new_with_label ("HH:MM");

    g_assert (app);

    gtk_button_set_relief (GTK_BUTTON (button),
                           GTK_RELIEF_NONE);

    clock_tick_update (button);

    g_signal_connect (G_OBJECT (button),
                      "clicked",
                      G_CALLBACK (on_click_show_calendar),
                      NULL);

    g_timeout_add (1000, (GSourceFunc) clock_tick_update, button);

    return button;
}

