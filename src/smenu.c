/*============================================================================
Copyright (c) 2022-2025 Raspberry Pi Holdings Ltd.
All rights reserved.

Some code taken from the lxpanel project

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

#include <locale.h>
#include <glib/gi18n.h>
#include <menu-cache.h>
#include <libfm/fm-gtk.h>

#ifdef LXPLUG
#include "plugin.h"
#else
#include "lxutils.h"
#endif

#include "smenu.h"

#ifndef LXPLUG
#include "launcher.h"
#endif

extern void gtk_run (void);

/*----------------------------------------------------------------------------*/
/* Typedefs and macros                                                        */
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/* Global data                                                                */
/*----------------------------------------------------------------------------*/

static gboolean longpress;

GQuark sys_menu_item_quark = 0;

/*----------------------------------------------------------------------------*/
/* Prototypes                                                                 */
/*----------------------------------------------------------------------------*/

static gboolean _open_dir_in_file_manager (GAppLaunchContext *ctx, GList *folder_infos, gpointer, GError **err);
static void destroy_search (MenuPlugin *m);
static gboolean filter_apps (GtkTreeModel *model, GtkTreeIter *iter, gpointer user_data);
static void append_to_entry (GtkWidget *entry, char val);
static void resize_search (MenuPlugin *m);
static void handle_search_changed (GtkEditable *, gpointer user_data);
static gboolean handle_list_keypress (GtkWidget *, GdkEventKey *event, gpointer user_data);
static gboolean handle_search_keypress (GtkWidget *, GdkEventKey *event, gpointer user_data);
static void handle_list_select (GtkTreeView *tv, GtkTreePath *path, GtkTreeViewColumn *, gpointer user_data);
static void search_destroyed (GtkWidget *, gpointer data);
static void create_search (MenuPlugin *m);
static void handle_menu_item_activate (GtkMenuItem *mi, MenuPlugin *);
static void handle_menu_item_properties (GtkMenuItem *, GtkWidget* mi);
static void handle_restore_submenu (GtkMenuItem *mi, GtkWidget *submenu);
static void handle_menu_item_data_get (FmDndSrc *ds, GtkWidget *mi);
static void show_context_menu (GtkWidget* mi);
static gboolean handle_menu_item_button_press (GtkWidget* mi, GdkEventButton* evt, MenuPlugin* m);
static gboolean handle_key_presses (GtkWidget *, GdkEventKey *event, gpointer user_data);
static GtkWidget *create_system_menu_item (MenuCacheItem *item, MenuPlugin *m);
static int sys_menu_load_submenu (MenuPlugin* m, MenuCacheDir* dir, GtkWidget* menu, int pos);
static void sys_menu_insert_items (MenuPlugin *m, GtkMenu *menu, int position);
static void reload_system_menu (MenuPlugin *m, GtkMenu *menu);
static void handle_reload_menu (MenuCache *, gpointer user_data);
static void read_system_menu (GtkMenu *menu, MenuPlugin *m);
static void handle_run_command (GtkWidget *, gpointer data);
static GtkWidget *read_menu_item (MenuPlugin *m, char *disp_name, char *icon, void (*cmd)(void));
static void mlogout (void);
static gboolean create_menu (MenuPlugin *m);
static void menu_button_clicked (GtkWidget *, MenuPlugin *m);
#ifdef LXPLUG
static void handle_search_resize (GtkWidget *, GtkAllocation *, gpointer user_data);
static void handle_menu_hidden (GtkWidget *, gpointer user_data);
#else
static void handle_menu_item_add_to_launcher (GtkMenuItem *, GtkWidget* mi);
static gboolean handle_menu_item_button_release (GtkWidget* mi, GdkEventButton*, MenuPlugin* m);
static void handle_menu_item_gesture_pressed (GtkGestureLongPress *, gdouble, gdouble, GtkWidget *);
static void handle_popped_up (GtkMenu *menu, gpointer, gpointer, gboolean, gboolean, MenuPlugin *);
#endif

/*----------------------------------------------------------------------------*/
/* Function definitions                                                       */
/*----------------------------------------------------------------------------*/

/* Open a specified path in a file manager */

static gboolean _open_dir_in_file_manager (GAppLaunchContext* ctx, GList* folder_infos,
                                          gpointer, GError** err)
{
    FmFileInfo *fi = folder_infos->data; /* only first is used */
    GAppInfo *app = g_app_info_get_default_for_type("inode/directory", TRUE);
    GFile *gf;
    gboolean ret;

    if (app == NULL)
    {
        g_set_error_literal(err, G_SHELL_ERROR, G_SHELL_ERROR_EMPTY_STRING,
                            _("No file manager is configured."));
        return FALSE;
    }
    gf = fm_path_to_gfile(fm_file_info_get_path(fi));
    folder_infos = g_list_prepend(NULL, gf);
    ret = fm_app_info_launch(app, folder_infos, ctx, err);
    g_list_free(folder_infos);
    g_object_unref(gf);
    g_object_unref(app);
    return ret;
}

/* Search box */

static void destroy_search (MenuPlugin *m)
{
#ifdef LXPLUG
    gtk_widget_destroy (m->swin);
#else
    close_popup ();
#endif
    m->swin = NULL;
}

static gboolean filter_apps (GtkTreeModel *model, GtkTreeIter *iter, gpointer user_data)
{
    MenuPlugin *m = (MenuPlugin *) user_data;
    gboolean res = FALSE;
    char *str, *pstr = NULL;
    GtkTreeIter prev = *iter;

    gtk_tree_model_get (model, iter, 1, &str, -1);
    if (gtk_tree_model_iter_previous (model, &prev)) gtk_tree_model_get (model, &prev, 1, &pstr, -1);
    if ((!pstr || strcmp (str, pstr)) && strcasestr (str, gtk_entry_get_text (GTK_ENTRY (m->srch)))) res = TRUE;
    if (pstr) g_free (pstr);
    g_free (str);
    return res;
}

static void append_to_entry (GtkWidget *entry, char val)
{
    int len = strlen (gtk_entry_get_text (GTK_ENTRY (entry)));
    if (val) gtk_editable_insert_text (GTK_EDITABLE (entry), &val, 1, &len);
    else if (len) gtk_editable_delete_text (GTK_EDITABLE (entry), (len - 1), -1);
    gtk_widget_grab_focus (entry);
    gtk_editable_set_position (GTK_EDITABLE (entry), -1);
}

static void resize_search (MenuPlugin *m)
{
    GtkTreePath *path;
    GdkRectangle rect;
    int nrows, height;

    if (m->fixed)
    {
        height = m->height - gtk_widget_get_allocated_height (m->srch);
        nrows = height;
    }
    else
    {
#ifdef LXPLUG
        gdk_monitor_get_geometry (gdk_display_get_monitor_at_window (gdk_display_get_default (), GDK_WINDOW (m->panel)), &rect);
        height = rect.height - gtk_widget_get_allocated_height (GTK_WIDGET (&(m->panel->window))) - gtk_widget_get_allocated_height (m->srch);
#else
        gdk_monitor_get_geometry (gtk_layer_get_monitor (GTK_WINDOW (m->swin)), &rect);
        height = (rect.height - gtk_layer_get_exclusive_zone (find_panel (m->plugin)))
            - gtk_widget_get_allocated_height (m->srch);
#endif

        /* update the stored row height if current height is bigger */
        path = gtk_tree_path_new_from_indices (0, -1);
        gtk_tree_view_get_cell_area (GTK_TREE_VIEW (m->stv), path, NULL, &rect);
        gtk_tree_path_free (path);
        if (rect.height > m->rheight) m->rheight = rect.height;

        /* calculate the height in pixels from the number of rows */
        nrows = gtk_tree_model_iter_n_children (gtk_tree_view_get_model (GTK_TREE_VIEW (m->stv)), NULL);
        nrows *= (m->rheight + 2);
        if (nrows > height) nrows = height;
    }

    /* set the size of the scrolled window and then redraw the window */
    gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (m->scr), GTK_POLICY_NEVER, nrows < height ? GTK_POLICY_NEVER : GTK_POLICY_AUTOMATIC);

    gtk_widget_set_size_request (m->scr, -1, nrows);
#ifdef LXPLUG
    gtk_window_resize (GTK_WINDOW (m->swin), 1, 1);
#endif
}

static void handle_search_changed (GtkEditable *, gpointer user_data)
{
    MenuPlugin *m = (MenuPlugin *) user_data;
    GtkTreePath *path = gtk_tree_path_new_from_indices (0, -1);

    gtk_tree_model_filter_refilter (GTK_TREE_MODEL_FILTER (gtk_tree_view_get_model (GTK_TREE_VIEW (m->stv))));
    gtk_tree_view_set_cursor (GTK_TREE_VIEW (m->stv), path, NULL, FALSE);
    gtk_tree_path_free (path);

    resize_search (m);
}

static gboolean handle_list_keypress (GtkWidget *, GdkEventKey *event, gpointer user_data)
{
    MenuPlugin *m = (MenuPlugin *) user_data;

#ifdef LXPLUG
    if (event->keyval == GDK_KEY_Escape)
    {
        destroy_search (m);
        return TRUE;
    }
#endif

    if ((event->keyval >= 'a' && event->keyval <= 'z') ||
        (event->keyval >= 'A' && event->keyval <= 'Z'))
    {
        append_to_entry (m->srch, event->keyval);
        return TRUE;
    }

    if (event->keyval == GDK_KEY_BackSpace)
    {
        append_to_entry (m->srch, 0);
        return TRUE;
    }

    return FALSE;
}

static gboolean handle_search_keypress (GtkWidget *, GdkEventKey *event, gpointer user_data)
{
    MenuPlugin *m = (MenuPlugin *) user_data;
    GtkTreeSelection *sel;
    GtkTreeModel *model;
    GtkTreeIter iter;
    GtkTreePath *path;
    gchar *str;
    FmPath *fpath;
    int nrows;

    switch (event->keyval)
    {
        case GDK_KEY_KP_Enter :
        case GDK_KEY_Return :   sel = gtk_tree_view_get_selection (GTK_TREE_VIEW (m->stv));
                                if (gtk_tree_selection_get_selected (sel, &model, &iter))
                                {
                                    gtk_tree_model_get (model, &iter, 2, &str, -1);
                                    fpath = fm_path_new_for_str (str);
                                    fm_launch_path_simple (NULL, NULL, fpath, _open_dir_in_file_manager, NULL);
                                    fm_path_unref (fpath);
                                }
                                destroy_search (m);
                                return TRUE;

#ifdef LXPLUG
        case GDK_KEY_Escape :   destroy_search (m);
                                return TRUE;
#endif

        case GDK_KEY_Up :
        case GDK_KEY_Down :     nrows = event->keyval == GDK_KEY_Down ? 1 : gtk_tree_model_iter_n_children (gtk_tree_view_get_model (GTK_TREE_VIEW (m->stv)), NULL) - 1;
                                path = gtk_tree_path_new_from_indices (nrows, -1);
                                gtk_tree_view_set_cursor (GTK_TREE_VIEW (m->stv), path, NULL, FALSE);
                                gtk_tree_path_free (path);
                                gtk_widget_grab_focus (m->stv);
                                return TRUE;

        default :               return FALSE;
    }
}

static void handle_list_select (GtkTreeView *tv, GtkTreePath *path, GtkTreeViewColumn *, gpointer user_data)
{
    MenuPlugin *m = (MenuPlugin *) user_data;
    GtkTreeModel *mod = gtk_tree_view_get_model (tv);
    GtkTreeIter iter;
    gchar *str;
    FmPath *fpath;

    if (gtk_tree_model_get_iter (mod, &iter, path))
    {
        gtk_tree_model_get (mod, &iter, 2, &str, -1);
        fpath = fm_path_new_for_str (str);
        fm_launch_path_simple (NULL, NULL, fpath, _open_dir_in_file_manager, NULL);
        fm_path_unref (fpath);
    }

    destroy_search (m);
}

#ifdef LXPLUG
static void handle_search_resize (GtkWidget *, GtkAllocation *, gpointer user_data)
{
    MenuPlugin *m = (MenuPlugin *) user_data;
    int x, y;

    lxpanel_plugin_popup_set_position_helper (m->panel, m->plugin, m->swin, &x, &y);
    gdk_window_move (gtk_widget_get_window (m->swin), x, y);
}
#endif

static void search_destroyed (GtkWidget *, gpointer data)
{
    MenuPlugin *m = (MenuPlugin *) data;
    g_signal_handlers_disconnect_matched (m->swin, G_SIGNAL_MATCH_DATA, 0, 0, NULL, NULL, m);
    g_signal_handlers_disconnect_matched (m->srch, G_SIGNAL_MATCH_DATA, 0, 0, NULL, NULL, m);
    g_signal_handlers_disconnect_matched (m->stv, G_SIGNAL_MATCH_DATA, 0, 0, NULL, NULL, m);
    m->swin = NULL;
}

static void create_search (MenuPlugin *m)
{
    GtkCellRenderer *prend, *trend;
    GtkTreeModelSort *slist;
    GtkTreeModelFilter *flist;
    GtkWidget *box;

    /* create the window */
    m->swin = gtk_window_new (GTK_WINDOW_TOPLEVEL);
    gtk_widget_set_name (m->swin, "panelpopup");

    /* add a box */
    box = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
    gtk_container_add (GTK_CONTAINER (m->swin), box);

    /* create the search entry */
    m->srch = gtk_search_entry_new ();
    g_signal_connect (m->srch, "changed", G_CALLBACK (handle_search_changed), m);
    g_signal_connect (m->srch, "key-press-event", G_CALLBACK (handle_search_keypress), m);

    /* create a scrolled window to hold the tree view */
    m->scr = gtk_scrolled_window_new (NULL, NULL);

    /* put in box in the appropriate order */
    if (!m->fixed && wrap_is_at_bottom (m)) gtk_box_pack_start (GTK_BOX (box), m->scr, FALSE, FALSE, 0);
    gtk_box_pack_start (GTK_BOX (box), m->srch, FALSE, FALSE, 0);
    if (m->fixed || !wrap_is_at_bottom (m)) gtk_box_pack_start (GTK_BOX (box), m->scr, FALSE, FALSE, 0);

    /* create the filtered list for the tree view */
    slist = GTK_TREE_MODEL_SORT (gtk_tree_model_sort_new_with_model (GTK_TREE_MODEL (m->applist)));
    gtk_tree_sortable_set_sort_column_id (GTK_TREE_SORTABLE (slist), 1, GTK_SORT_ASCENDING);
    flist = GTK_TREE_MODEL_FILTER (gtk_tree_model_filter_new (GTK_TREE_MODEL (slist), NULL));
    gtk_tree_model_filter_set_visible_func (flist, (GtkTreeModelFilterVisibleFunc) filter_apps, m, NULL);

    /* create the tree view */
    m->stv = gtk_tree_view_new_with_model (GTK_TREE_MODEL (flist));
    g_signal_connect (m->stv, "key-press-event", G_CALLBACK (handle_list_keypress), m);
    g_signal_connect (m->stv, "row-activated", G_CALLBACK (handle_list_select), m);
    gtk_container_add (GTK_CONTAINER (m->scr), m->stv);
    g_object_unref (slist);
    g_object_unref (flist);

    /* set up the tree view */
    prend = gtk_cell_renderer_pixbuf_new ();
    trend = gtk_cell_renderer_text_new ();
    gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (m->stv), -1, NULL, prend, "pixbuf", 0, NULL);
    gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (m->stv), -1, NULL, trend, "text", 1, NULL);
    gtk_tree_view_set_headers_visible (GTK_TREE_VIEW (m->stv), FALSE);
    gtk_tree_view_set_enable_search (GTK_TREE_VIEW (m->stv), FALSE);

    g_signal_connect (m->swin, "destroy", G_CALLBACK (search_destroyed), m);

    m->rheight = 0;

    /* realise */
    wrap_popup_at_button (m, m->swin, m->plugin);
    resize_search (m);

#ifdef LXPLUG
    /* resize window as needed */
    if (!m->fixed && panel_is_at_bottom (m->panel)) g_signal_connect (m->swin, "size-allocate", G_CALLBACK (handle_search_resize), m);
#endif
}

/* Handlers for system menu items */

static void handle_menu_item_activate (GtkMenuItem *mi, MenuPlugin *)
{
    FmFileInfo *fi = g_object_get_qdata (G_OBJECT (mi), sys_menu_item_quark);

    fm_launch_path_simple (NULL, NULL, fm_file_info_get_path (fi), _open_dir_in_file_manager, NULL);
}

static void handle_menu_item_add_to_desktop (GtkMenuItem *, GtkWidget* mi)
{
    FmFileInfo *fi = g_object_get_qdata (G_OBJECT (mi), sys_menu_item_quark);
    FmPathList *files = fm_path_list_new ();

    fm_path_list_push_tail (files, fm_file_info_get_path (fi));
    fm_link_files (NULL, files, fm_path_get_desktop ());
    fm_path_list_unref (files);
}

#ifndef LXPLUG
static void handle_menu_item_add_to_launcher (GtkMenuItem *, GtkWidget* mi)
{
    FmFileInfo *fi = g_object_get_qdata (G_OBJECT (mi), sys_menu_item_quark);
    add_to_launcher (fm_file_info_get_name (fi));
}
#endif

static void handle_menu_item_properties (GtkMenuItem *, GtkWidget* mi)
{
    FmFileInfo *fi = g_object_get_qdata (G_OBJECT (mi), sys_menu_item_quark);
    FmFileInfoList *files = fm_file_info_list_new ();

    fm_file_info_list_push_tail (files, fi);
    fm_show_file_properties (NULL, files);
    fm_file_info_list_unref (files);
}

static void handle_restore_submenu (GtkMenuItem *mi, GtkWidget *submenu)
{
    g_signal_handlers_disconnect_by_func (mi, handle_restore_submenu, submenu);
    gtk_menu_item_set_submenu (mi, submenu);
    g_object_set_data (G_OBJECT (mi), "PanelMenuItemSubmenu", NULL);
}

static void handle_menu_item_data_get (FmDndSrc *ds, GtkWidget *mi)
{
    FmFileInfo *fi = g_object_get_qdata (G_OBJECT (mi), sys_menu_item_quark);

    fm_dnd_src_set_file (ds, fi);
}

static void show_context_menu (GtkWidget* mi)
{
    GtkWidget *item, *menu;

    menu = gtk_menu_new ();

    item = gtk_menu_item_new_with_label (_("Add to desktop"));
    g_signal_connect (item, "activate", G_CALLBACK (handle_menu_item_add_to_desktop), mi);
    gtk_menu_shell_append (GTK_MENU_SHELL (menu), item);

#ifndef LXPLUG
    item = gtk_menu_item_new_with_label (_("Add to Launcher"));
    g_signal_connect (item, "activate", G_CALLBACK (handle_menu_item_add_to_launcher), mi);
    gtk_menu_shell_append (GTK_MENU_SHELL (menu), item);
#endif

    item = gtk_separator_menu_item_new ();
    gtk_menu_shell_append (GTK_MENU_SHELL (menu), item);

    item = gtk_menu_item_new_with_label (_("Properties"));
    g_signal_connect (item, "activate", G_CALLBACK (handle_menu_item_properties), mi);
    gtk_menu_shell_append (GTK_MENU_SHELL (menu), item);

    item = gtk_menu_item_get_submenu (GTK_MENU_ITEM (mi)); /* reuse it */
    if (item)
    {
        /* set object data to keep reference on the submenu we preserve */
        g_object_set_data_full (G_OBJECT (mi), "PanelMenuItemSubmenu", g_object_ref (item), g_object_unref);
        gtk_menu_popdown (GTK_MENU (item));
    }
    gtk_menu_item_set_submenu (GTK_MENU_ITEM (mi), menu);
    g_signal_connect (mi, "deselect", G_CALLBACK (handle_restore_submenu), item);
    gtk_widget_show_all (menu);
}

static gboolean handle_menu_item_button_press (GtkWidget* mi, GdkEventButton* evt, MenuPlugin* m)
{
    longpress = FALSE;
    if (evt->button == 1)
    {
        /* allow drag on clicked item */
        g_signal_handlers_disconnect_matched (m->ds, G_SIGNAL_MATCH_FUNC, 0, 0, NULL, handle_menu_item_data_get, NULL);
        fm_dnd_src_set_widget (m->ds, mi);
        g_signal_connect (m->ds, "data-get", G_CALLBACK (handle_menu_item_data_get), mi);
    }
    else if (evt->button == 3)
    {
        /* don't make duplicates */
        if (g_signal_handler_find (mi, G_SIGNAL_MATCH_FUNC, 0, 0, NULL, handle_restore_submenu, NULL)) return FALSE;
        show_context_menu (mi);
    }
    return FALSE;
}

/* Handler for keyboard events while menu is open */

static gboolean handle_key_presses (GtkWidget *, GdkEventKey *event, gpointer user_data)
{
    MenuPlugin *m = (MenuPlugin *) user_data;

    if ((event->keyval >= 'a' && event->keyval <= 'z') ||
        (event->keyval >= 'A' && event->keyval <= 'Z'))
    {
        gtk_widget_hide (m->menu);
        if (!m->swin) create_search (m);
        gtk_entry_set_text (GTK_ENTRY (m->srch), "");
        append_to_entry (m->srch, event->keyval);
        return TRUE;
    }

#ifdef LXPLUG
    if (event->keyval == GDK_KEY_Super_L)
    {
        gtk_menu_popdown (GTK_MENU (m->menu));
        return TRUE;
    }
#else
    if (event->keyval == GDK_KEY_Return)
    {
        GtkWidget *menu = gtk_menu_shell_get_selected_item (GTK_MENU_SHELL (m->menu));
        GtkWidget *submenu = gtk_menu_item_get_submenu (GTK_MENU_ITEM (menu));
        if (submenu)
        {
            GtkWidget *subitem = gtk_menu_shell_get_selected_item (GTK_MENU_SHELL (submenu));
            if (subitem)
            {
                handle_menu_item_activate (GTK_MENU_ITEM (subitem), m);
                gtk_widget_hide (m->menu);
                return TRUE;
            }
        }
    }
#endif
    return FALSE;
}

#ifndef LXPLUG
static gboolean handle_menu_item_button_release (GtkWidget* mi, GdkEventButton*, MenuPlugin* m)
{
    if (!longpress)
    {
        handle_menu_item_activate (GTK_MENU_ITEM (mi), m);
        gtk_widget_hide (m->menu);
    }
    else
    {
        show_context_menu (mi);
        gtk_menu_item_select (GTK_MENU_ITEM (mi));
    }
    longpress = FALSE;
    return TRUE;
}

static void handle_menu_item_gesture_pressed (GtkGestureLongPress *, gdouble, gdouble, GtkWidget *)
{
    longpress = TRUE;
}
#endif

/* Functions to create system menu items */

static GtkWidget *create_system_menu_item (MenuCacheItem *item, MenuPlugin *m)
{
    GtkWidget* mi, *img, *box, *label;
    GdkPixbuf *icon;
    FmPath *path;
    FmFileInfo *fi;
    char *mpath;

    if (menu_cache_item_get_type (item) == MENU_CACHE_TYPE_SEP)
    {
        mi = gtk_separator_menu_item_new ();
        g_object_set_qdata (G_OBJECT (mi), sys_menu_item_quark, GINT_TO_POINTER (1));
    }
    else
    {
        mi = gtk_menu_item_new ();
        box = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, MENU_ICON_SPACE);
        gtk_container_add (GTK_CONTAINER (mi), box);

        img = gtk_image_new ();
        gtk_container_add (GTK_CONTAINER (box), img);

        label = gtk_label_new (menu_cache_item_get_name (item));
        gtk_container_add (GTK_CONTAINER (box), label);

        mpath = menu_cache_dir_make_path (MENU_CACHE_DIR (item));
        path = fm_path_new_relative (fm_path_get_apps_menu (), mpath + 13);
        g_free (mpath);

        fi = fm_file_info_new_from_menu_cache_item (path, item);
        g_object_set_qdata_full (G_OBJECT (mi), sys_menu_item_quark, fi, (GDestroyNotify) fm_file_info_unref);

#ifdef LXPLUG
        FmIcon *fm_icon = fm_file_info_get_icon (fi);
        if (fm_icon == NULL) fm_icon = fm_icon_from_name ("application-x-executable");
        icon = fm_pixbuf_from_icon_with_fallback (fm_icon, panel_get_safe_icon_size (m->panel), "application-x-executable");
        gtk_image_set_from_pixbuf (GTK_IMAGE (img), icon);
#else
        icon = NULL;
        const char *icon_name = menu_cache_item_get_icon (item);
        if (icon_name)
        {
            if (strstr (icon_name, "/"))
                icon = gdk_pixbuf_new_from_file_at_size (icon_name, m->icon_size, m->icon_size, NULL);
            else
            {
                icon = gtk_icon_theme_load_icon (gtk_icon_theme_get_default (), icon_name,
                    m->icon_size, GTK_ICON_LOOKUP_FORCE_SIZE, NULL);

                // fallback for packages using obsolete icon location
                if (!icon)
                {
                    char *fname = g_strdup_printf ("/usr/share/pixmaps/%s", icon_name);
                    icon = gdk_pixbuf_new_from_file_at_size (fname, m->icon_size, m->icon_size, NULL);
                    g_free (fname);
                }
            }
        }
        if (!icon)
            icon = gtk_icon_theme_load_icon (gtk_icon_theme_get_default (), "application-x-executable",
                m->icon_size, GTK_ICON_LOOKUP_FORCE_SIZE, NULL);
        if (icon) gtk_image_set_from_pixbuf (GTK_IMAGE (img), icon);
#endif
        if (menu_cache_item_get_type (item) == MENU_CACHE_TYPE_APP)
        {
            mpath = fm_path_to_str (path);
            gtk_list_store_insert_with_values (m->applist, NULL, -1, 0, icon, 1, menu_cache_item_get_name (item), 2, mpath, -1);
            g_free (mpath);

            gtk_widget_set_name (mi, "syssubmenu");
#ifdef LXPLUG
            g_signal_connect (mi, "activate", G_CALLBACK (handle_menu_item_activate), m);
#endif
        }
        fm_path_unref (path);
        if (icon) g_object_unref (icon);

        g_signal_connect (mi, "button-press-event", G_CALLBACK (handle_menu_item_button_press), m);
        gtk_drag_source_set (mi, GDK_BUTTON1_MASK, NULL, 0, GDK_ACTION_COPY);
#ifndef LXPLUG
        g_signal_connect (mi, "button-release-event", G_CALLBACK (handle_menu_item_button_release), m);

        m->migesture = gtk_gesture_long_press_new (mi);
        gtk_gesture_single_set_touch_only (GTK_GESTURE_SINGLE (m->migesture), touch_only);
        g_signal_connect (m->migesture, "pressed", G_CALLBACK (handle_menu_item_gesture_pressed), mi);
        gtk_event_controller_set_propagation_phase (GTK_EVENT_CONTROLLER (m->migesture), GTK_PHASE_BUBBLE);
#endif
    }
    gtk_widget_show_all (mi);
    return mi;
}

static int sys_menu_load_submenu (MenuPlugin* m, MenuCacheDir* dir, GtkWidget* menu, int pos)
{
    GSList *l, *children;
    gint count = 0;

    if (!menu_cache_dir_is_visible (dir)) return 0;

    children = menu_cache_dir_list_children (dir);

    for (l = children; l; l = l->next)
    {
        MenuCacheItem* item = MENU_CACHE_ITEM (l->data);
        if ((menu_cache_item_get_type (item) != MENU_CACHE_TYPE_APP) || (menu_cache_app_get_is_visible (MENU_CACHE_APP (item), SHOW_IN_LXDE)))
        {
            GtkWidget *mi = create_system_menu_item (item, m);
            count++;
            if (mi != NULL) gtk_menu_shell_insert ((GtkMenuShell*) menu, mi, pos);
            if (pos >= 0) ++pos;

            /* process subentries */
            if (menu_cache_item_get_type (item) == MENU_CACHE_TYPE_DIR)
            {
                GtkWidget* sub = gtk_menu_new ();
                gtk_menu_set_reserve_toggle_size (GTK_MENU (sub), FALSE);
                g_signal_connect (sub, "key-press-event", G_CALLBACK (handle_key_presses), m);
#ifndef LXPLUG
                g_signal_connect (sub, "popped-up", G_CALLBACK (handle_popped_up), m);
#endif
                gint s_count = sys_menu_load_submenu (m, MENU_CACHE_DIR (item), sub, -1);
                if (s_count)
                {   
                    gtk_widget_set_name (mi, "sysmenu");
                    gtk_menu_item_set_submenu (GTK_MENU_ITEM (mi), sub);
                }
                else
                {
                    /* don't keep empty submenus */
                    gtk_widget_destroy (sub);
                    gtk_widget_destroy (mi);
                    if (pos > 0) pos--;
                }
            }
        }
    }
    return count;
}


/* Functions to load system menu into panel menu in response to 'system' tag */

static void sys_menu_insert_items (MenuPlugin *m, GtkMenu *menu, int position)
{
    MenuCacheDir *dir;

    if (G_UNLIKELY (sys_menu_item_quark == 0))
        sys_menu_item_quark = g_quark_from_static_string ("SysMenuItem");

    dir = menu_cache_dup_root_dir (m->menu_cache);

    if (dir)
    {
        sys_menu_load_submenu (m, dir, GTK_WIDGET (menu), position);
        menu_cache_item_unref (MENU_CACHE_ITEM (dir));
    }
    else
    {
        /* menu content is empty - add a place holder */
        GtkWidget* mi = gtk_menu_item_new ();
        g_object_set_qdata (G_OBJECT (mi), sys_menu_item_quark, GINT_TO_POINTER (1));
        gtk_menu_shell_insert (GTK_MENU_SHELL (menu), mi, position);
    }
}

static void reload_system_menu (MenuPlugin *m, GtkMenu *menu)
{
    GList *children, *child;
    GtkMenuItem* item;
    GtkWidget* sub_menu;
    gint idx;

    children = gtk_container_get_children (GTK_CONTAINER (menu));
    for (child = children, idx = 0; child; child = child->next, ++idx)
    {
        item = GTK_MENU_ITEM (child->data);
        if (g_object_get_qdata (G_OBJECT (item), sys_menu_item_quark) != NULL)
        {
            do
            {
                item = GTK_MENU_ITEM (child->data);
                child = child->next;
                gtk_widget_destroy (GTK_WIDGET (item));
            }
            while (child && g_object_get_qdata (G_OBJECT (child->data), sys_menu_item_quark) != NULL);

            sys_menu_insert_items (m, menu, idx);
            if (!child) break;
        }
        else if ((sub_menu = gtk_menu_item_get_submenu (item)))
        {
            reload_system_menu (m, GTK_MENU (sub_menu));
        }
    }
    g_list_free (children);
}

static void handle_reload_menu (MenuCache *, gpointer user_data)
{
    MenuPlugin *m = (MenuPlugin *) user_data;

    gtk_list_store_clear (m->applist);
    reload_system_menu (m, GTK_MENU (m->menu));
}

static void read_system_menu (GtkMenu *menu, MenuPlugin *m)
{
    if (m->menu_cache == NULL)
    {
        gboolean need_prefix = (g_getenv ("XDG_MENU_PREFIX") == NULL);
        m->menu_cache = menu_cache_lookup (need_prefix ? "lxde-applications.menu+hidden" : "applications.menu+hidden");
        if (m->menu_cache == NULL)
        {
            g_warning ("error loading applications menu");
            return;
        }
        m->reload_notify = menu_cache_add_reload_notify (m->menu_cache, handle_reload_menu, m);
        sys_menu_insert_items (m, menu, -1);
    }
}

/* Functions to create individual menu items from panel config */

static void handle_run_command (GtkWidget *, gpointer data)
{
    void (*cmd) (void) = (void *) data;
    cmd ();
}

static GtkWidget *read_menu_item (MenuPlugin *m, char *disp_name, char *icon, void (*cmd)(void))
{
    GtkWidget *item, *box, *label, *img;
    GdkPixbuf *pixbuf;

    item = gtk_menu_item_new ();
    box = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, MENU_ICON_SPACE);
    gtk_container_add (GTK_CONTAINER (item), box);
    gtk_container_set_border_width (GTK_CONTAINER (item), 0);

    label = gtk_label_new (disp_name);
    g_signal_connect (G_OBJECT (item), "activate", (GCallback) handle_run_command, cmd);

    img = gtk_image_new ();
    pixbuf = gtk_icon_theme_load_icon (gtk_icon_theme_get_default (), icon, wrap_icon_size (m), GTK_ICON_LOOKUP_FORCE_SIZE, NULL);
    if (pixbuf)
    {
        gtk_image_set_from_pixbuf (GTK_IMAGE (img), pixbuf);
        g_object_unref (pixbuf);
        gtk_container_add (GTK_CONTAINER (box), img);
    }
    gtk_container_add (GTK_CONTAINER (box), label);
    gtk_widget_show_all (box);

    return item;
}

#ifdef LXPLUG
static void handle_menu_hidden (GtkWidget *, gpointer user_data)
{
    MenuPlugin *m = (MenuPlugin *) user_data;
    if (m->swin && !gtk_widget_is_visible (m->swin)) m->swin = NULL;
}
#else
static void handle_popped_up (GtkMenu *menu, gpointer, gpointer, gboolean, gboolean, MenuPlugin *)
{
    GdkRectangle rect;
    GtkWidget *win = gtk_widget_get_toplevel (GTK_WIDGET (menu));
    GdkWindow *gwin = gtk_widget_get_window (win);
    GdkMonitor *mon = gdk_display_get_monitor_at_window (gdk_display_get_default (), gwin);
    gdk_monitor_get_workarea (mon, &rect);
    int height = gdk_window_get_height (gwin);
    int max_height = rect.height / gdk_window_get_scale_factor (gwin);
    if (height > max_height)
    {
        height = max_height;
        gdk_window_resize (gwin, gdk_window_get_width (gwin), height);
    }
}
#endif

static void mlogout (void)
{
    fm_launch_command_simple (NULL, NULL, 0, "lxde-pi-shutdown-helper", NULL);
}

/* Top level function to read in menu data from panel configuration */
static gboolean create_menu (MenuPlugin *m)
{
    GtkWidget *mi;

    m->menu = gtk_menu_new ();
    gtk_menu_set_reserve_toggle_size (GTK_MENU (m->menu), FALSE);
    gtk_container_set_border_width (GTK_CONTAINER (m->menu), 0);
    g_signal_connect (m->menu, "key-press-event", G_CALLBACK (handle_key_presses), m);
#ifdef LXPLUG
    g_signal_connect (m->menu, "hide", G_CALLBACK (handle_menu_hidden), m);
#else
    g_signal_connect (m->menu, "popped-up", G_CALLBACK (handle_popped_up), m);
#endif
    read_system_menu (GTK_MENU (m->menu), m);

    mi = gtk_separator_menu_item_new ();
    gtk_widget_set_name (mi, "sysmenu");
    gtk_widget_show (mi);
    gtk_menu_shell_append (GTK_MENU_SHELL (m->menu), mi);

    mi = read_menu_item (m, _("Run"), "system-run", gtk_run);
    gtk_widget_set_name (mi, "sysmenu");
    gtk_widget_show (mi);
    gtk_menu_shell_append (GTK_MENU_SHELL (m->menu), mi);

    mi = read_menu_item (m, _("Logout"), "system-shutdown", mlogout);
    gtk_widget_set_name (mi, "sysmenu");
    gtk_widget_show (mi);
    gtk_menu_shell_append (GTK_MENU_SHELL (m->menu), mi);

    return TRUE;
}

/*----------------------------------------------------------------------------*/
/* wf-panel plugin functions                                                  */
/*----------------------------------------------------------------------------*/

/* Handler for button click */
static void menu_button_clicked (GtkWidget *, MenuPlugin *m)
{
    CHECK_LONGPRESS
    wrap_show_menu (m->plugin, m->menu);
}

/* Handler for system config changed message from panel */
void menu_update_display (MenuPlugin *m)
{
    GdkPixbuf *pixbuf = gtk_icon_theme_load_icon (gtk_icon_theme_get_default (), m->icon, wrap_icon_size (m), GTK_ICON_LOOKUP_FORCE_SIZE, NULL);
    if (pixbuf)
    {
        gtk_image_set_from_pixbuf (GTK_IMAGE (m->img), pixbuf);
        g_object_unref (pixbuf);
    }
    if (m->img) gtk_widget_set_size_request (m->img, wrap_icon_size (m) + 2 * m->padding, -1);

    if (m->applist) gtk_list_store_clear (m->applist);
    if (m->menu) gtk_widget_destroy (m->menu);
    if (m->swin) destroy_search (m);
    if (m->menu_cache)
    {
        menu_cache_remove_reload_notify (m->menu_cache, m->reload_notify);
        menu_cache_unref (m->menu_cache);
        m->menu_cache = NULL;
    }
    create_menu (m);
}

/* Handler for control message */
void menu_show_menu (MenuPlugin *m)
{
    if (gtk_widget_is_visible (m->menu)) gtk_menu_popdown (GTK_MENU (m->menu));
    else if (m->swin && gtk_widget_is_visible (m->swin)) destroy_search (m);
    else wrap_show_menu (m->plugin, m->menu);
}

/* Handler for padding update from variable watcher */
void menu_set_padding (MenuPlugin *m)
{
    gtk_widget_set_size_request (m->img, wrap_icon_size (m) + 2 * m->padding, -1);
}

void menu_init (MenuPlugin *m)
{
    setlocale (LC_ALL, "");
    bindtextdomain (GETTEXT_PACKAGE, PACKAGE_LOCALE_DIR);
    bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");

#ifndef LXPLUG
    fm_gtk_init (NULL);
    fm_init (NULL);
#endif

    /* Allocate icon as a child of top level */
    m->img = gtk_image_new ();
    gtk_container_add (GTK_CONTAINER (m->plugin), m->img);
    gtk_widget_set_tooltip_text (m->img, _("Click here to open applications menu"));

    /* Set up button */
    gtk_button_set_relief (GTK_BUTTON (m->plugin), GTK_RELIEF_NONE);
#ifndef LXPLUG
    g_signal_connect (m->plugin, "clicked", G_CALLBACK (menu_button_clicked), m);
#endif

    /* Set up variables */
    m->icon = g_strdup ("start-here");
    m->applist = gtk_list_store_new (3, GDK_TYPE_PIXBUF, G_TYPE_STRING, G_TYPE_STRING);
    m->ds = fm_dnd_src_new (NULL);
    m->swin = NULL;
    m->menu_cache = NULL;

    /* Load the menu configuration */
    create_menu (m);

    /* Watch the icon theme and reload the menu if it changes */
    g_signal_connect (gtk_icon_theme_get_default (), "changed", G_CALLBACK (handle_reload_menu), m);

    /* Show the widget and return */
    gtk_widget_show_all (m->plugin);
}

void menu_destructor (gpointer user_data)
{
    MenuPlugin *m = (MenuPlugin *) user_data;

    g_signal_handlers_disconnect_matched (m->ds, G_SIGNAL_MATCH_FUNC, 0, 0, NULL, handle_menu_item_data_get, NULL);
    g_object_unref (G_OBJECT (m->ds));

    if (m->menu) gtk_widget_destroy (m->menu);
#ifdef LXPLUG
    if (m->swin) destroy_search (m);
#else
    close_popup ();
#endif
    if (m->menu_cache)
    {
        menu_cache_remove_reload_notify (m->menu_cache, m->reload_notify);
        menu_cache_unref (m->menu_cache);
    }
    g_free (m->icon);

#ifndef LXPLUG
    if (m->migesture) g_object_unref (m->migesture);
#endif

    g_free (m);
}

/*----------------------------------------------------------------------------*/
/* LXPanel plugin functions                                                   */
/*----------------------------------------------------------------------------*/
#ifdef LXPLUG

/* Constructor */
static GtkWidget *menu_constructor (LXPanel *panel, config_setting_t *settings)
{
    /* Allocate and initialize plugin context */
    MenuPlugin *m = g_new0 (MenuPlugin, 1);

    /* Allocate top level widget and set into plugin widget pointer. */
    m->panel = panel;
    m->settings = settings;
    m->plugin = gtk_button_new ();
    lxpanel_plugin_set_data (m->plugin, m, menu_destructor);

    /* Read config */
    if (!config_setting_lookup_int (m->settings, "padding", &m->padding)) m->padding = 4;
    if (!config_setting_lookup_int (m->settings, "fixed", &m->fixed)) m->fixed = FALSE;
    if (!config_setting_lookup_int (m->settings, "height", &m->height)) m->height = 300;

    menu_init (m);

    return m->plugin;
}

/* Handler for button press */
static gboolean menu_button_press_event (GtkWidget *plugin, GdkEventButton *event, LXPanel *)
{
    MenuPlugin *m = lxpanel_plugin_get_data (plugin);
    if (event->button == 1)
    {
        menu_button_clicked (plugin, m);
        return TRUE;
    }
    return FALSE;
}

/* Handler for system config changed message from panel */
static void menu_panel_configuration_changed (LXPanel *, GtkWidget *plugin)
{
    MenuPlugin *m = lxpanel_plugin_get_data (plugin);
    menu_update_display (m);
}

/* Handler for control message */
static void menu_control (GtkWidget *plugin)
{
    MenuPlugin *m = lxpanel_plugin_get_data (plugin);
    menu_show_menu (m);
}

/* Apply changes from config dialog */
static gboolean menu_apply_config (gpointer user_data)
{
    MenuPlugin *m = lxpanel_plugin_get_data (GTK_WIDGET (user_data));

    config_group_set_int (m->settings, "padding", m->padding);
    config_group_set_int (m->settings, "fixed", m->fixed);
    config_group_set_int (m->settings, "height", m->height);

    menu_set_padding (m);
    return FALSE;
}

/* Display configuration dialog */
static GtkWidget *menu_configure (LXPanel *panel, GtkWidget *plugin)
{
    MenuPlugin *m = lxpanel_plugin_get_data (plugin);

    return lxpanel_generic_config_dlg (_("Menu"), panel, menu_apply_config, plugin,
                                       _("Icon horizontal padding"), &m->padding, CONF_TYPE_INT,
                                       _("Fix height of search window"), &m->fixed, CONF_TYPE_BOOL,
                                       _("Search window height"), &m->height, CONF_TYPE_INT,
                                       NULL);
}

FM_DEFINE_MODULE (lxpanel_gtk, smenu)

/* Plugin descriptor */
LXPanelPluginInit fm_module_init_lxpanel_gtk = {
    .name = N_("Menu with Search"),
    .description = N_("Searchable Application Menu"),
    .new_instance = menu_constructor,
    .reconfigure = menu_panel_configuration_changed,
    .config = menu_configure,
    .button_press_event = menu_button_press_event,
    .show_system_menu = menu_control,
    .gettext_package = GETTEXT_PACKAGE
};
#endif

/* End of file */
/*----------------------------------------------------------------------------*/
