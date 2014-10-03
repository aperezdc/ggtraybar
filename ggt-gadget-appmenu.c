/*
 * menuserver.c
 * Copyright (C) 2014 Adrian Perez <aperez@igalia.com>
 *
 * Distributed under terms of the MIT license.
 */

#include "ggt.h"
#include <libbamf/libbamf.h>
#include <gio/gio.h>
#include <gio/gdesktopappinfo.h>
#include <gtk/gtk.h>
#include <string.h>
#include "dg-util.h"


typedef struct {
  gint32 app_xid;
  gchar *app_name;
  gchar *unique_bus_name;
  gchar *app_menu_object_path;
  GMenuModel *app_menu_model;
} Application;


static GHashTable *app_by_xid = NULL;
static Application *current_app = NULL;


static Application*
make_app (BamfApplication *application,
          BamfWindow      *window)
{
  Application app = {
    .app_xid = bamf_window_get_xid (window),
    .app_name = NULL,
  };

  app.unique_bus_name = bamf_window_get_utf8_prop (window,
                                                   "_GTK_UNIQUE_BUS_NAME");
  if (!app.unique_bus_name)
    return NULL;

  const gchar *desktop_path = bamf_application_get_desktop_file (application);
  if (desktop_path) {
    dg_lobj GDesktopAppInfo *app_info = g_desktop_app_info_new_from_filename (desktop_path);
    if (app_info) {
      app.app_name = g_strdup (g_app_info_get_name (G_APP_INFO (app_info)));
    }
  }
  if (!app.app_name) {
    app.app_name = bamf_view_get_name (BAMF_VIEW (application));
  }
  g_assert (app.app_name);

  dg_lobj GDBusConnection *bus = g_bus_get_sync (G_BUS_TYPE_SESSION,
                                                 NULL,
                                                 NULL);
  app.app_menu_object_path = bamf_window_get_utf8_prop (window,
                                                        "_GTK_APP_MENU_OBJECT_PATH");
  app.app_menu_model = G_MENU_MODEL (g_dbus_menu_model_get (bus,
                                                            app.unique_bus_name,
                                                            app.app_menu_object_path));

  Application *newapp = g_slice_new0 (Application);
  memcpy (newapp, &app, sizeof (Application));
  return newapp;
}


static void
on_active_window_changed (BamfMatcher *matcher,
                          BamfWindow  *window1,
                          BamfWindow  *window2,
                          GtkWidget   *button)
{
  if (!window2)
    return;

  gint32 xid = bamf_window_get_xid (window2);
  Application *app = g_hash_table_lookup (app_by_xid, GINT_TO_POINTER (xid));
  if (!app) {
    BamfApplication *application = bamf_matcher_get_application_for_window (matcher, window2);
    g_assert (application);
    app = make_app (application, window2);
    if (!app)
      return;
    g_hash_table_insert (app_by_xid, GINT_TO_POINTER (xid), app);
  }
  g_assert (app);
  current_app = app;
  gtk_button_set_label (GTK_BUTTON (button),
                        app->app_name);
}


static void
on_menu_button_clicked (GtkButton *button,
                        gpointer  *userdata)
{
  if (!current_app)
    return;

  GtkWidget *menu = gtk_menu_new_from_model (current_app->app_menu_model);
  g_object_ref_sink (menu);
  gtk_menu_popup (GTK_MENU (menu),
                  NULL,
                  NULL,
                  NULL,
                  NULL,
                  0,
                  gtk_get_current_event_time ());
}


GtkWidget*
ggt_appmenu_new (void)
{
  GtkWidget *button = gtk_button_new_with_label ("(no app)");

  BamfMatcher *matcher = bamf_matcher_get_default ();
  app_by_xid = g_hash_table_new (g_direct_hash, g_direct_equal);

  g_signal_connect (matcher,
                    "active-window-changed",
                    G_CALLBACK (on_active_window_changed),
                    button);

  /* Pre-populate for the current active window */
  on_active_window_changed (matcher,
                            NULL,
                            bamf_matcher_get_active_window (matcher),
                            button);

  g_signal_connect (button,
                    "clicked",
                    G_CALLBACK (on_menu_button_clicked),
                    NULL);

  return button;
}

