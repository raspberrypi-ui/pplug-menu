/*============================================================================
Copyright (c) 2022-2025 Raspberry Pi
All rights reserved.

Some code taken from the lxpanel and pcmanfm projects

Copyright (c) 2006-2010 Hong Jen Yee (PCMan) <pcman.tw@gmail.com>
            2006-2008 Jim Huang <jserv.tw@gmail.com>
            2008 Fred Chien <fred@lxde.org>
            2009 Ying-Chun Liu (PaulLiu) <grandpaul@gmail.com>
            2009-2010 Marty Jack <martyj19@comcast.net>
            2010 Jürgen Hötzel <juergen@archlinux.org>
            2010-2011 Julien Lavergne <julien.lavergne@gmail.com>
            2012-2013 Henry Gebhardt <hsggebhardt@gmail.com>
            2012 Michael Rawson <michaelrawson76@gmail.com>
            2014 Max Krummenacher <max.oss.09@gmail.com>
            2014 SHiNE CsyFeK <csyfek@users.sourceforge.net>
            2014 Andriy Grytsenko <andrej@rep.kiev.ua>

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
    * Neither the name of the copyright holder nor the
      names of its contributors may be used to endorse or promote products
      derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
============================================================================*/

#include <glib/gstdio.h>
#include <gtk/gtk.h>
#include <menu-cache.h>

GtkWidget *dlg, *lbl_file, *lbl_loc, *lbl_target, *entry_name, *entry_cmd, *entry_dir, *entry_desc, *entry_tooltip, *img_icon, *sw_notif, *sw_terminal, *btn_ok, *btn_cancel;

static gboolean update_if_changed (GKeyFile *kf, const char *param, GtkWidget *widget)
{
    char *str;
    const char *ent;
    gboolean update;

    if (GTK_IS_ENTRY (widget))
    {
        str = g_key_file_get_string (kf, "Desktop Entry", param, NULL);
        ent = gtk_entry_get_text (GTK_ENTRY (widget));
        if (!str && ent[0] == 0) update = FALSE;
        else if (!g_strcmp0 (str, ent)) update = FALSE;
        else
        {
            g_key_file_set_string (kf, "Desktop Entry", param, ent);
            update = TRUE;
        }
        g_free (str);
    }
    else if (GTK_IS_SWITCH (widget))
    {
        if (gtk_switch_get_state (GTK_SWITCH (widget)) != g_key_file_get_boolean (kf, "Desktop Entry", param, NULL))
        {
            g_key_file_set_boolean (kf, "Desktop Entry", param, gtk_switch_get_state (GTK_SWITCH (widget)));
            update = TRUE;
        }
        else update = FALSE;
    }

    return update;
}

static void prop_dialog_ok (GtkButton *, gpointer)
{
    GKeyFile *kf;
    char *path, *str, *name, *comment;
    gsize len;
    gboolean update = FALSE;

    // use the target file as source
    kf = g_key_file_new ();
    g_key_file_load_from_file (kf, gtk_label_get_text (GTK_LABEL (lbl_target)), G_KEY_FILE_KEEP_COMMENTS | G_KEY_FILE_KEEP_TRANSLATIONS, NULL);

    str = g_strdup (getenv ("LANG"));
    if (strchr (str, '.')) *strchr (str, '.') = 0;
    name = g_strdup_printf ("Name[%s]", str);
    comment = g_strdup_printf ("Comment[%s]", str);
    g_free (str);

    if (!g_key_file_has_key (kf, "Desktop Entry", name, NULL))
    {
        g_free (name);
        name = g_strdup ("Name");
    }

    if (!g_key_file_has_key (kf, "Desktop Entry", comment, NULL))
    {
        g_free (comment);
        comment = g_strdup ("Comment");
    }

    update |= update_if_changed (kf, name, entry_name);
    update |= update_if_changed (kf, comment, entry_desc);
    update |= update_if_changed (kf, "Exec", entry_cmd);
    update |= update_if_changed (kf, "Path", entry_dir);
    update |= update_if_changed (kf, "StartupNotify", sw_notif);
    update |= update_if_changed (kf, "Terminal", sw_terminal);

    g_free (name);
    g_free (comment);

    // write to the override in local
    if (update)
    {
        str = g_path_get_basename (gtk_label_get_text (GTK_LABEL (lbl_target)));
        path = g_build_filename (g_get_home_dir (), ".local", "share", "applications", str, NULL);
        g_free (str);

        str = g_path_get_dirname (path);
        g_mkdir_with_parents (str, S_IRUSR | S_IWUSR | S_IXUSR);
        g_free (str);

        str = g_key_file_to_data (kf, &len, NULL);
        g_file_set_contents (path, str, len, NULL);
        g_free (str);

        g_free (path);
    }
    g_key_file_free (kf);

    gtk_widget_destroy (dlg);
}

static void prop_dialog_cancel (GtkButton *, gpointer)
{
    gtk_widget_destroy (dlg);
}

void show_properties_dialog (MenuCacheItem *item)
{
    GtkBuilder *builder;
    char *str, *path;

    builder = gtk_builder_new_from_file (PACKAGE_DATA_DIR "/ui/properties.ui");
    dlg = (GtkWidget *) gtk_builder_get_object (builder, "wd_properties");
    lbl_file = (GtkWidget *) gtk_builder_get_object (builder, "lbl_file");
    lbl_loc = (GtkWidget *) gtk_builder_get_object (builder, "lbl_loc");
    lbl_target = (GtkWidget *) gtk_builder_get_object (builder, "lbl_target");
    entry_name = (GtkWidget *) gtk_builder_get_object (builder, "entry_name");
    entry_cmd = (GtkWidget *) gtk_builder_get_object (builder, "entry_cmd");
    entry_dir = (GtkWidget *) gtk_builder_get_object (builder, "entry_dir");
    entry_desc = (GtkWidget *) gtk_builder_get_object (builder, "entry_desc");
    img_icon = (GtkWidget *) gtk_builder_get_object (builder, "img_icon");
    sw_notif = (GtkWidget *) gtk_builder_get_object (builder, "sw_notif");
    sw_terminal = (GtkWidget *) gtk_builder_get_object (builder, "sw_terminal");
    btn_ok = (GtkWidget *) gtk_builder_get_object (builder, "btn_ok");
    btn_cancel = (GtkWidget *) gtk_builder_get_object (builder, "btn_cancel");

    gtk_image_set_from_icon_name (GTK_IMAGE (img_icon), menu_cache_item_get_icon (item), GTK_ICON_SIZE_DND);
    gtk_label_set_text (GTK_LABEL (lbl_file), menu_cache_item_get_file_basename (item));
    gtk_entry_set_text (GTK_ENTRY (entry_name), menu_cache_item_get_name (item));
    gtk_entry_set_text (GTK_ENTRY (entry_cmd), menu_cache_app_get_exec (MENU_CACHE_APP (item)));
    if (menu_cache_item_get_comment (item))
        gtk_entry_set_text (GTK_ENTRY (entry_desc), menu_cache_item_get_comment (item));
    if (menu_cache_app_get_working_dir (MENU_CACHE_APP (item)))
        gtk_entry_set_text (GTK_ENTRY (entry_dir), menu_cache_app_get_working_dir (MENU_CACHE_APP (item)));

    gtk_switch_set_active (GTK_SWITCH (sw_notif), menu_cache_app_get_use_sn (MENU_CACHE_APP (item)));
    gtk_switch_set_active (GTK_SWITCH (sw_terminal), menu_cache_app_get_use_terminal (MENU_CACHE_APP (item)));

    path = menu_cache_item_get_file_path (item);
    gtk_label_set_text (GTK_LABEL (lbl_target), path);
    g_free (path);

    MenuCacheDir *parent = menu_cache_item_dup_parent (item);
    path = menu_cache_dir_make_path (parent);
    str = g_strdup_printf ("menu:/%s", path);
    gtk_label_set_text (GTK_LABEL (lbl_loc), str);
    g_free (str);
    g_free (path);
    menu_cache_item_unref (MENU_CACHE_ITEM (parent));

    g_signal_connect (btn_ok, "clicked", G_CALLBACK (prop_dialog_ok), NULL);
    g_signal_connect (btn_cancel, "clicked", G_CALLBACK (prop_dialog_cancel), NULL);

    gtk_window_set_default_size (GTK_WINDOW (dlg), 500, -1);
    gtk_widget_show (dlg);
    g_object_unref (builder);
}

/* End of file */
/*----------------------------------------------------------------------------*/
