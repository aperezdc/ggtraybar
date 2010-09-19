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


static GtkWidget*
make_calendar_window (void)
{
    GtkWidget *window   = gtk_window_new (GTK_WINDOW_TOPLEVEL);
    GtkWidget *calendar = gtk_calendar_new ();

    gtk_window_set_default_size      (GTK_WINDOW (window), 180, 180);
    gtk_window_set_decorated         (GTK_WINDOW (window), FALSE);
    gtk_window_set_resizable         (GTK_WINDOW (window), FALSE);
    gtk_window_set_skip_taskbar_hint (GTK_WINDOW (window), TRUE);
    gtk_window_set_skip_pager_hint   (GTK_WINDOW (window), TRUE);
    gtk_window_set_position          (GTK_WINDOW (window), GTK_WIN_POS_MOUSE);
    gtk_window_stick                 (GTK_WINDOW (window));

    gtk_calendar_display_options (GTK_CALENDAR (calendar),
                                  GTK_CALENDAR_SHOW_WEEK_NUMBERS |
                                  GTK_CALENDAR_SHOW_DAY_NAMES |
                                  GTK_CALENDAR_SHOW_HEADING);

    gtk_container_set_border_width (GTK_CONTAINER (window), 5);
    gtk_container_add (GTK_CONTAINER (window), calendar);

    return window;
}


static void
on_calendar_toggle (GtkToggleButton *button, gpointer data)
{
    GtkWidget *calendar;

    g_assert (button);
    g_assert (data);

    calendar = GTK_WIDGET (data);

    if (gtk_toggle_button_get_active (button))
        gtk_widget_show_all (calendar);
    else
        gtk_widget_hide (calendar);
}


GtkWidget*
ggt_clock_init (ggtraybar_t *app)
{
    GtkWidget *button = gtk_toggle_button_new_with_label ("HH:MM");
    GtkWidget *calwin = make_calendar_window ();

    g_assert (app);

    gtk_button_set_relief (GTK_BUTTON (button),
                           GTK_RELIEF_NONE);

    clock_tick_update (button);

    g_signal_connect (G_OBJECT (button), "toggled",
                      G_CALLBACK (on_calendar_toggle),
                      calwin);

    g_timeout_add (1000, (GSourceFunc) clock_tick_update, button);

    return button;
}

