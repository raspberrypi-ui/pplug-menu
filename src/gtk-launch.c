/* GTK - The GIMP Toolkit
 *
 * Copyright (C) 2012 Red Hat, Inc.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library. If not, see <http://www.gnu.org/licenses/>.
 *
 * Author: Tomas Bzatek <tbzatek@redhat.com>
 */

#include <gtk/gtk.h>
#include <gio/gdesktopappinfo.h>

void gtk_launch (const char *app_name)
{
    gchar *bus_name, *desktop_file_name, *object_path, *p;
    GAppInfo *info = NULL;
    GAppLaunchContext *launch_context;
    GDBusConnection *connection;

    bus_name = g_strdup (app_name);
    if (g_str_has_suffix (app_name, ".desktop"))
    {
        desktop_file_name = g_strdup (app_name);
        bus_name[strlen (bus_name) - strlen(".desktop")] = '\0';
    }
    else desktop_file_name = g_strconcat (app_name, ".desktop", NULL);
    if (!g_dbus_is_name (bus_name)) g_clear_pointer (&bus_name, g_free);

    info = G_APP_INFO (g_desktop_app_info_new (desktop_file_name));
    g_free (desktop_file_name);
    if (!info) return;

    launch_context = G_APP_LAUNCH_CONTEXT (gdk_display_get_app_launch_context (gdk_display_get_default ()));
    if (!g_app_info_launch (info, NULL, launch_context, NULL)) return;
    g_object_unref (info);
    g_object_unref (launch_context);

    if (bus_name != NULL)
    {
        connection = g_bus_get_sync (G_BUS_TYPE_SESSION, NULL, NULL);

        object_path = g_strdup_printf ("/%s", bus_name);
        for (p = object_path; *p != '\0'; p++)
            if (*p == '.')
                *p = '/';

        if (connection)
            g_dbus_connection_call_sync (connection, bus_name, object_path, "org.freedesktop.DBus.Peer",
              "Ping", NULL, NULL, G_DBUS_CALL_FLAGS_NONE, -1, NULL, NULL);

        g_free (object_path);
        g_object_unref (connection);
        g_free (bus_name);
    }
}
