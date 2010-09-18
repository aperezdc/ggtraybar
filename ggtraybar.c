/*
 * ggtraybar.c
 * Copyright (C) 2010 Adrian Perez <aperez@igalia.com>
 *
 * Distributed under terms of the MIT license.
 */

#include <gnome-globalmenu/globalmenu-server.h>
#include <gtk/gtk.h>
#include <stdlib.h>


int
main (int argc, char **argv)
{
    GnomenuGlobalMenuBar *menubar;
    GtkWidget *window;

    g_type_init ();
    gtk_init (&argc, &argv);

    window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title (GTK_WINDOW (window), "Menubar");

    menubar = gnomenu_global_menu_bar_new ();

    gtk_container_add (GTK_CONTAINER (window),
                       GTK_WIDGET (menubar));

    gtk_widget_show_all (window);

    gtk_main ();

    return EXIT_SUCCESS;
}


