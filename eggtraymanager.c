/*
 * eggtraymanager.c
 *
 * Copyright (C) 2014 Adrian Perez de Castro <aperez@igalia.com>
 * Copyright (C) 2002 Anders Carlsson <andersca@gnu.org>
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

#include <string.h>
#include <gtk/gtk.h>
#include <gtk/gtkx.h>
#include "eggtraymanager.h"
#include "eggmarshalers.h"

G_DEFINE_TYPE (EggTrayManager, egg_tray_manager, G_TYPE_OBJECT)

/* Signals */
enum
{
  TRAY_ICON_ADDED,
  TRAY_ICON_REMOVED,
  MESSAGE_SENT,
  MESSAGE_CANCELLED,
  LOST_SELECTION,
  LAST_SIGNAL
};

typedef struct
{
  long id, len;
  long remaining_len;

  long timeout;
  Window window;
  char *str;
} PendingMessage;

static guint manager_signals[LAST_SIGNAL] = { 0 };

#define SYSTEM_TRAY_REQUEST_DOCK    0
#define SYSTEM_TRAY_BEGIN_MESSAGE   1
#define SYSTEM_TRAY_CANCEL_MESSAGE  2

static gboolean egg_tray_manager_check_running_xscreen (Screen *xscreen);

static void egg_tray_manager_init (EggTrayManager *manager);
static void egg_tray_manager_class_init (EggTrayManagerClass *klass);

static void egg_tray_manager_finalize (GObject *object);

static void egg_tray_manager_unmanage (EggTrayManager *manager);


static void
egg_tray_manager_init (EggTrayManager *manager)
{
  manager->socket_table = g_hash_table_new (NULL, NULL);
}

static void
egg_tray_manager_class_init (EggTrayManagerClass *klass)
{
  G_OBJECT_CLASS (klass)->finalize = egg_tray_manager_finalize;

  manager_signals[TRAY_ICON_ADDED] =
    g_signal_new ("tray-icon-added",
                  G_OBJECT_CLASS_TYPE (klass),
                  G_SIGNAL_RUN_LAST,
                  G_STRUCT_OFFSET (EggTrayManagerClass, tray_icon_added),
                  NULL, NULL,
                  g_cclosure_marshal_VOID__OBJECT,
                  G_TYPE_NONE, 1,
                  GTK_TYPE_SOCKET);

  manager_signals[TRAY_ICON_REMOVED] =
    g_signal_new ("tray-icon-removed",
                  G_OBJECT_CLASS_TYPE (klass),
                  G_SIGNAL_RUN_LAST,
                  G_STRUCT_OFFSET (EggTrayManagerClass, tray_icon_removed),
                  NULL, NULL,
                  g_cclosure_marshal_VOID__OBJECT,
                  G_TYPE_NONE, 1,
                  GTK_TYPE_SOCKET);

  manager_signals[MESSAGE_SENT] =
    g_signal_new ("message-sent",
                  G_OBJECT_CLASS_TYPE (klass),
                  G_SIGNAL_RUN_LAST,
                  G_STRUCT_OFFSET (EggTrayManagerClass, message_sent),
                  NULL, NULL,
                  _egg_marshal_VOID__OBJECT_STRING_LONG_LONG,
                  G_TYPE_NONE, 4,
                  GTK_TYPE_SOCKET,
                  G_TYPE_STRING,
                  G_TYPE_LONG,
                  G_TYPE_LONG);

  manager_signals[MESSAGE_CANCELLED] =
    g_signal_new ("message-cancelled",
                  G_OBJECT_CLASS_TYPE (klass),
                  G_SIGNAL_RUN_LAST,
                  G_STRUCT_OFFSET (EggTrayManagerClass, message_cancelled),
                  NULL, NULL,
                  _egg_marshal_VOID__OBJECT_LONG,
                  G_TYPE_NONE, 2,
                  GTK_TYPE_SOCKET,
                  G_TYPE_LONG);

  manager_signals[LOST_SELECTION] =
    g_signal_new ("lost-selection",
                  G_OBJECT_CLASS_TYPE (klass),
                  G_SIGNAL_RUN_LAST,
                  G_STRUCT_OFFSET (EggTrayManagerClass, lost_selection),
                  NULL, NULL,
                  g_cclosure_marshal_VOID__VOID,
                  G_TYPE_NONE, 0);
}

static void
egg_tray_manager_finalize (GObject *object)
{
  egg_tray_manager_unmanage (EGG_TRAY_MANAGER (object));
  G_OBJECT_CLASS (egg_tray_manager_parent_class)->finalize (object);
}

EggTrayManager *
egg_tray_manager_new (void)
{
  return EGG_TRAY_MANAGER (g_object_new (EGG_TYPE_TRAY_MANAGER, NULL));
}

static gboolean
egg_tray_manager_plug_removed (GtkSocket      *socket,
                               EggTrayManager *manager)
{
  Window *window = g_object_get_data (G_OBJECT (socket), "egg-tray-child-window");

  g_hash_table_remove (manager->socket_table, GINT_TO_POINTER (*window));
  g_object_set_data (G_OBJECT (socket), "egg-tray-child-window", NULL);
  g_signal_emit (manager, manager_signals[TRAY_ICON_REMOVED], 0, socket);

  /* This destroys the socket. */
  return FALSE;
}

#if 0
static gboolean
egg_tray_manager_socket_draw (GtkWidget *widget,
                              cairo_t   *cr)
{
  GtkStyleContext *context = gtk_widget_get_style_context (widget);
  gint h = gtk_widget_get_allocated_height (widget);
  gint w = gtk_widget_get_allocated_width (widget);
  GdkRGBA bg_color;

  gtk_style_context_get_background_color (context,
                                          GTK_STATE_FLAG_NORMAL,
                                          &bg_color);

  cairo_set_source_rgba (cr,
                         bg_color.red,
                         bg_color.green,
                         bg_color.blue,
                         bg_color.alpha);

  cairo_rectangle (cr, 0, 0, w, h);
  cairo_fill (cr);

  return FALSE;
}
#endif

static void
egg_tray_manager_handle_dock_request(EggTrayManager      *manager,
                                     XClientMessageEvent *xevent)
{
  GtkWidget *socket;
  Window *window;

  socket = gtk_socket_new ();
  /* gtk_widget_set_app_paintable (socket, TRUE);
  gtk_widget_set_double_buffered (socket, FALSE);
  g_signal_connect (socket, "draw",
                    G_CALLBACK (egg_tray_manager_socket_draw),
                    NULL); */

  gtk_widget_show (socket);

  /* We need to set the child window here
   * so that the client can call _get functions
   * in the signal handler
   */
  window = g_new (Window, 1);
  *window = xevent->data.l[2];

  g_object_set_data_full (G_OBJECT (socket),
                          "egg-tray-child-window",
                          window, g_free);

  g_signal_emit (manager, manager_signals[TRAY_ICON_ADDED], 0, socket);

  /* Add the socket only if it's been attached */
  if (GTK_IS_WINDOW (gtk_widget_get_toplevel (GTK_WIDGET(socket))))
    {
      GtkRequisition req;
      XWindowAttributes wa;

      gtk_socket_add_id (GTK_SOCKET (socket), xevent->data.l[2]);
      g_signal_connect (socket,
                        "plug-removed",
                        G_CALLBACK (egg_tray_manager_plug_removed),
                        manager);

      gdk_error_trap_push ();
      XGetWindowAttributes (GDK_DISPLAY_XDISPLAY (gdk_display_get_default ()),
                            *window,
                            &wa);
      if (gdk_error_trap_pop ())
        {
          g_signal_emit (manager,
                         manager_signals[TRAY_ICON_REMOVED],
                         0,
                         socket);
          gtk_widget_destroy (socket);
        }
      else
        {
          g_hash_table_insert (manager->socket_table,
                               GINT_TO_POINTER (xevent->data.l[2]),
                               socket);
          req.width = req.height = 1;
          gtk_widget_get_preferred_size (socket, &req, NULL);
        }
    }
}

static void
pending_message_free (PendingMessage *message)
{
  g_free (message->str);
  g_free (message);
}

static void
egg_tray_manager_handle_message_data (EggTrayManager      *manager,
                                      XClientMessageEvent *xevent)
{
  GList *p;

  /* Try to see if we can find the
   * pending message in the list
   */
  for (p = manager->messages; p; p = p->next)
    {
      PendingMessage *msg = p->data;

      if (xevent->window == msg->window)
        {
          /* Append the message */
          gint len = MIN (msg->remaining_len, 20);

          memcpy (msg->str + msg->len - msg->remaining_len,
                  &xevent->data,
                  len);
          msg->remaining_len -= len;

          if (msg->remaining_len == 0)
            {
              GtkSocket *socket = g_hash_table_lookup (manager->socket_table,
                                                       GINT_TO_POINTER (msg->window));
              if (socket)
                g_signal_emit (manager,
                               manager_signals[MESSAGE_SENT],
                               0,
                               socket,
                               msg->str,
                               msg->id,
                               msg->timeout);

              manager->messages = g_list_remove_link (manager->messages, p);
              pending_message_free (msg);
            }
          return;
        }
    }
}

static void
egg_tray_manager_handle_begin_message (EggTrayManager      *manager,
                                       XClientMessageEvent *xevent)
{
  GList *p;

  /*
   * Check if the same message is already in the queue and remove it if so
   */
  for (p = manager->messages; p; p = p->next)
    {
      PendingMessage *msg = p->data;

      if (xevent->window == msg->window && xevent->data.l[4] == msg->id)
        {
          /* Hmm, we found it, now remove it */
          pending_message_free (msg);
          manager->messages = g_list_remove_link (manager->messages, p);
          break;
        }
    }

  /* Now add the new message to the queue */
  PendingMessage *msg = g_new0 (PendingMessage, 1);
  msg->window         = xevent->window;
  msg->timeout        = xevent->data.l[2];
  msg->len            = xevent->data.l[3];
  msg->id             = xevent->data.l[4];
  msg->remaining_len  = msg->len;
  msg->str            = g_malloc (msg->len + 1);
  msg->str[msg->len]  = '\0';
  manager->messages   = g_list_prepend (manager->messages, msg);
}

static void
egg_tray_manager_handle_cancel_message (EggTrayManager      *manager,
                                        XClientMessageEvent *xevent)
{
  GtkSocket *socket = g_hash_table_lookup (manager->socket_table,
                                           GINT_TO_POINTER (xevent->window));
  if (socket)
    g_signal_emit (manager,
                   manager_signals[MESSAGE_CANCELLED],
                   0,
                   socket,
                   xevent->data.l[2]);
}

static GdkFilterReturn
egg_tray_manager_handle_event (EggTrayManager      *manager,
                               XClientMessageEvent *xevent)
{
  switch (xevent->data.l[1])
    {
    case SYSTEM_TRAY_REQUEST_DOCK:
      egg_tray_manager_handle_dock_request (manager, xevent);
      return GDK_FILTER_REMOVE;

    case SYSTEM_TRAY_BEGIN_MESSAGE:
      egg_tray_manager_handle_begin_message (manager, xevent);
      return GDK_FILTER_REMOVE;

    case SYSTEM_TRAY_CANCEL_MESSAGE:
      egg_tray_manager_handle_cancel_message (manager, xevent);
      return GDK_FILTER_REMOVE;
    default:
      break;
    }

  return GDK_FILTER_CONTINUE;
}

static GdkFilterReturn
egg_tray_manager_window_filter (GdkXEvent *xev,
                                GdkEvent  *event,
                                gpointer   data)
{
  XEvent *xevent = (GdkXEvent *)xev;
  EggTrayManager *manager = EGG_TRAY_MANAGER (data);

  (void) event; /* Unused, avoid compiler warning */

  if (xevent->type == ClientMessage)
    {
      if (xevent->xclient.message_type == manager->opcode_atom)
        {
          return egg_tray_manager_handle_event (manager, (XClientMessageEvent *)xevent);
        }
      else if (xevent->xclient.message_type == manager->message_data_atom)
        {
          egg_tray_manager_handle_message_data (manager, (XClientMessageEvent *)xevent);
          return GDK_FILTER_REMOVE;
        }
    }
  else if (xevent->type == SelectionClear)
    {
      g_signal_emit (manager, manager_signals[LOST_SELECTION], 0);
      egg_tray_manager_unmanage (manager);
    }

  return GDK_FILTER_CONTINUE;
}

static void
egg_tray_manager_unmanage (EggTrayManager *manager)
{
  if (manager->invisible == NULL)
    return;

  GtkWidget *invisible = manager->invisible;

  g_assert (GTK_IS_INVISIBLE (invisible));
  g_assert (gtk_widget_get_realized (invisible));

  Display *display = GDK_WINDOW_XDISPLAY (gtk_widget_get_window (invisible));

  if (XGetSelectionOwner (display, manager->selection_atom) ==
      gdk_x11_window_get_xid (gtk_widget_get_window (invisible)))
    {
      guint32 timestamp = gdk_x11_get_server_time (gtk_widget_get_window (invisible));
      XSetSelectionOwner (display, manager->selection_atom, None, timestamp);
    }

  gdk_window_remove_filter (gtk_widget_get_window (invisible),
                            egg_tray_manager_window_filter,
                            manager);

  manager->invisible = NULL; /* prior to destroy for reentrancy paranoia */
  gtk_widget_destroy (invisible);
  g_object_unref (G_OBJECT (invisible));
}

static gboolean
egg_tray_manager_manage_xscreen (EggTrayManager *manager, Screen *xscreen)
{
  g_return_val_if_fail (EGG_IS_TRAY_MANAGER (manager), FALSE);
  g_return_val_if_fail (manager->screen == NULL, FALSE);

  /*
   * If there's already a manager running on the screen
   * we can't create another one.
   */
#if 0
  if (egg_tray_manager_check_running_xscreen (xscreen))
    return FALSE;
#endif
  GdkScreen *screen = gdk_display_get_screen (gdk_x11_lookup_xdisplay (DisplayOfScreen (xscreen)),
                                              XScreenNumberOfScreen (xscreen));

  GtkWidget *invisible = gtk_invisible_new_for_screen (screen);
  gtk_widget_realize (invisible);
  gtk_widget_add_events (invisible,
                         GDK_PROPERTY_CHANGE_MASK | GDK_STRUCTURE_MASK);

  gchar* selection_atom_name = g_strdup_printf ("_NET_SYSTEM_TRAY_S%d",
                                                XScreenNumberOfScreen (xscreen));
  manager->selection_atom = XInternAtom (DisplayOfScreen (xscreen), selection_atom_name, False);
  g_free (selection_atom_name);

  GdkWindow *invwindow = gtk_widget_get_window (invisible);
  guint32 timestamp = gdk_x11_get_server_time (invwindow);
  XSetSelectionOwner (DisplayOfScreen (xscreen), manager->selection_atom,
                      gdk_x11_window_get_xid (invwindow), timestamp);

  /* Check if we were could set the selection owner successfully */
  if (XGetSelectionOwner (DisplayOfScreen (xscreen), manager->selection_atom) ==
      gdk_x11_window_get_xid (invwindow))
    {
      XClientMessageEvent xev;

      xev.type = ClientMessage;
      xev.window = RootWindowOfScreen (xscreen);
      xev.message_type = XInternAtom (DisplayOfScreen (xscreen), "MANAGER", False);

      xev.format = 32;
      xev.data.l[0] = timestamp;
      xev.data.l[1] = manager->selection_atom;
      xev.data.l[2] = gdk_x11_window_get_xid (invwindow);
      xev.data.l[3] = 0;	/* manager specific data */
      xev.data.l[4] = 0;	/* manager specific data */

      XSendEvent (DisplayOfScreen (xscreen),
                  RootWindowOfScreen (xscreen),
                  False,
                  StructureNotifyMask,
                  (XEvent *)&xev);

      manager->invisible = invisible;
      g_object_ref (G_OBJECT (manager->invisible));

      manager->opcode_atom = XInternAtom (DisplayOfScreen (xscreen),
                                          "_NET_SYSTEM_TRAY_OPCODE",
                                          False);

      manager->message_data_atom = XInternAtom (DisplayOfScreen (xscreen),
                                                "_NET_SYSTEM_TRAY_MESSAGE_DATA",
                                                False);

      /* Add a window filter */
      gdk_window_add_filter (invwindow, egg_tray_manager_window_filter, manager);
      return TRUE;
    }
  else
    {
      gtk_widget_destroy (invisible);
      return FALSE;
    }
}

gboolean
egg_tray_manager_manage_screen (EggTrayManager *manager,
                                GdkScreen      *screen)
{
  g_return_val_if_fail (GDK_IS_SCREEN (screen), FALSE);
  g_return_val_if_fail (manager->screen == NULL, FALSE);

  return egg_tray_manager_manage_xscreen (manager,
                                          GDK_SCREEN_XSCREEN (screen));
}

static gboolean
egg_tray_manager_check_running_xscreen (Screen *xscreen)
{
  gchar *selection_atom_name = g_strdup_printf ("_NET_SYSTEM_TRAY_S%d",
                                                XScreenNumberOfScreen (xscreen));
  Atom selection_atom = XInternAtom (DisplayOfScreen (xscreen),
                                     selection_atom_name,
                                     False);
  g_free (selection_atom_name);

  if (XGetSelectionOwner (DisplayOfScreen (xscreen), selection_atom))
    return TRUE;
  else
    return FALSE;
}

gboolean
egg_tray_manager_check_running (GdkScreen *screen)
{
  g_return_val_if_fail (GDK_IS_SCREEN (screen), FALSE);
  return egg_tray_manager_check_running_xscreen (GDK_SCREEN_XSCREEN (screen));
}

gchar*
egg_tray_manager_get_child_title (EggTrayManager      *manager,
                                  EggTrayManagerChild *child)
{
  g_return_val_if_fail (EGG_IS_TRAY_MANAGER (manager), NULL);
  g_return_val_if_fail (GTK_IS_SOCKET (child), NULL);

  Window *child_window = g_object_get_data (G_OBJECT (child),
                                            "egg-tray-child-window");

  Atom utf8_string = XInternAtom (GDK_DISPLAY_XDISPLAY (gdk_display_get_default ()),
                                  "UTF8_STRING",
                                  False);
  Atom atom = XInternAtom (GDK_DISPLAY_XDISPLAY (gdk_display_get_default ()),
                           "_NET_WM_NAME",
                           False);

  gdk_error_trap_push();

  Atom type;
  int format;
  gulong nitems, bytes_after;
  guchar *tmp = NULL;
  gint result = XGetWindowProperty (GDK_DISPLAY_XDISPLAY (gdk_display_get_default ()),
                                    *child_window,
                                    atom,
                                    0,
                                    G_MAXLONG,
                                    False,
                                    utf8_string,
                                    &type,
                                    &format,
                                    &nitems,
                                    &bytes_after,
                                    &tmp);

  gchar *val = (gchar *) tmp;
  if (gdk_error_trap_pop() || result != Success || type != utf8_string)
    return NULL;

  if (format != 8 || nitems == 0) {
      if (val)
          XFree (val);
      return NULL;
  }

  if (!g_utf8_validate (val, nitems, NULL))
    {
      XFree (val);
      return NULL;
    }

  gchar *retval = g_strndup (val, nitems);
  XFree (val);
  return retval;
}
