/*
 * ggt-gadget-pager.c
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

#define WNCK_I_KNOW_THIS_IS_UNSTABLE /* Needed for Wnck :P */

#include "ggt.h"
#include <libwnck/libwnck.h>

GtkWidget*
ggt_pager_new (GGTraybar *app)
{
    GtkWidget *pager;
    g_assert (app);

    pager = wnck_pager_new ();
    wnck_pager_set_display_mode (WNCK_PAGER (pager), WNCK_PAGER_DISPLAY_CONTENT);
    wnck_pager_set_orientation (WNCK_PAGER (pager), GTK_ORIENTATION_HORIZONTAL);
    wnck_pager_set_show_all (WNCK_PAGER (pager), TRUE);
    wnck_pager_set_n_rows (WNCK_PAGER (pager), 1);
    gtk_widget_set_size_request (pager, 0, 0);
    return pager;
}
