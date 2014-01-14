/*
 * ggt-gadget-clock.c
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
#include <time.h>
#include <sys/time.h>
#include <sys/types.h>


static gint
clock_tick_update (GtkButton *button)
{
    gchar buf[20];
    time_t tnow;
    struct tm *now;

    g_assert (button);

    time (&tnow);
    now = localtime (&tnow);
    strftime (buf, 20, "%H:%M", now);

    gtk_button_set_label (button, buf);

    return TRUE;
}


static GtkWidget*
make_calendar_window (void)
{
    GtkWidget *window   = gtk_window_new (GTK_WINDOW_POPUP);
    GtkWidget *calendar = gtk_calendar_new ();

    gtk_window_set_default_size (GTK_WINDOW (window), 180, 180);

    gtk_calendar_set_display_options (GTK_CALENDAR (calendar),
                                      GTK_CALENDAR_SHOW_WEEK_NUMBERS |
                                      GTK_CALENDAR_SHOW_DAY_NAMES |
                                      GTK_CALENDAR_SHOW_HEADING);

    gtk_container_set_border_width (GTK_CONTAINER (window), 5);
    gtk_container_add (GTK_CONTAINER (window), calendar);

    return window;
}


static void
on_calendar_toggle (GtkToggleButton *button, GtkWidget *calendar)
{
    gint x, y;

    g_assert (button);
    g_assert (calendar);

    if (gtk_toggle_button_get_active (button)) {
        GtkAllocation allocation;
        gtk_window_set_position (GTK_WINDOW (calendar), GTK_WIN_POS_MOUSE);
        gtk_widget_show_all (calendar);
        gtk_window_get_position (GTK_WINDOW (calendar), &x, &y);
        gtk_widget_get_allocation (GTK_WIDGET (button), &allocation);
        gtk_window_move (GTK_WINDOW (calendar), x, allocation.height);
    }
    else
        gtk_widget_hide (calendar);
}


static gboolean
on_tooltip_show (GtkWidget  *button,
                 gint        x,
                 gint        y,
                 gboolean    keyboardmode,
                 GtkTooltip *tooltip,
                 gpointer    data)
{
    UNUSED (x);
    UNUSED (y);
    UNUSED (keyboardmode);
    UNUSED (data);

    char buf[60];
    time_t tnow;
    struct tm *now;

    g_assert (button);
    g_assert (tooltip);

    time (&tnow);
    now = localtime (&tnow);
    strftime (buf, 60, "%x", now);

    gtk_tooltip_set_text (tooltip, buf);

    return TRUE;
}



GtkWidget*
ggt_clock_new (GGTraybar *app)
{
    GtkWidget *button = gtk_toggle_button_new_with_label ("HH:MM");
    GtkWidget *calwin = make_calendar_window ();

    g_assert (app);

    gtk_button_set_relief (GTK_BUTTON (button),
                           GTK_RELIEF_NONE);

    clock_tick_update (GTK_BUTTON (button));

    g_signal_connect (G_OBJECT (button), "toggled",
                      G_CALLBACK (on_calendar_toggle),
                      calwin);

    g_signal_connect (G_OBJECT (button), "query-tooltip",
                      G_CALLBACK (on_tooltip_show),
                      NULL);

    gtk_widget_set_has_tooltip (button, TRUE);
    gtk_window_set_attached_to (GTK_WINDOW (calwin), button);

    g_timeout_add (1000 * 15, (GSourceFunc) clock_tick_update, button);

    return button;
}

