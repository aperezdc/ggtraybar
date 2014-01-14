/*
 * ggt-gadget-winsel.c
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

#define WNCK_I_KNOW_THIS_IS_UNSTABLE /* Needed for Wnck :P */

#include "ggt.h"
#include <libwnck/libwnck.h>

GtkWidget*
ggt_winlist_init (GGTraybar *app)
{
    GtkWidget *winlist = wnck_tasklist_new ();

    g_assert (app);

    wnck_tasklist_set_grouping (WNCK_TASKLIST (winlist), WNCK_TASKLIST_AUTO_GROUP);
    wnck_tasklist_set_include_all_workspaces (WNCK_TASKLIST (winlist), FALSE);
    wnck_tasklist_set_switch_workspace_on_unminimize (WNCK_TASKLIST (winlist), FALSE);

    return winlist;
}

