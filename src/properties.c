/*============================================================================
Copyright (c) 2025 Raspberry Pi
All rights reserved.

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

#include <gtk/gtk.h>
#include <glib/gstdio.h>
#include <glib/gi18n.h>

#include "plugin.h"

/*----------------------------------------------------------------------------*/
/* Typedefs and macros                                                        */
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/* Global data                                                                */
/*----------------------------------------------------------------------------*/

static GtkWidget *dlg, *img_icon, *lbl_loc, *lbl_id;

/*----------------------------------------------------------------------------*/
/* Prototypes                                                                 */
/*----------------------------------------------------------------------------*/

static void show_icon (const char *name);
static void edit_item (GtkButton *, gpointer data);
static void dialog_cancel (GtkButton *, gpointer);

/*----------------------------------------------------------------------------*/
/* Function definitions                                                       */
/*----------------------------------------------------------------------------*/

/* File properties dialog */

void show_properties_dialog (MenuCacheItem *item)
{
    GtkBuilder *builder;
    char *path, *icon_name;

    textdomain (GETTEXT_PACKAGE);
    builder = gtk_builder_new_from_file (PACKAGE_DATA_DIR "/ui/properties.ui");
    dlg = (GtkWidget *) gtk_builder_get_object (builder, "wd_properties");
    img_icon = (GtkWidget *) gtk_builder_get_object (builder, "img_icon");
    lbl_loc = (GtkWidget *) gtk_builder_get_object (builder, "lbl_loc");
    lbl_id = (GtkWidget *) gtk_builder_get_object (builder, "lbl_file");

    icon_name = g_strdup (menu_cache_item_get_icon (item));
    show_icon (icon_name);
    g_free (icon_name);

    gtk_label_set_text (GTK_LABEL (lbl_id), menu_cache_item_get_file_basename (item));
    gtk_label_set_text (GTK_LABEL (gtk_builder_get_object (builder, "lbl_name")), menu_cache_item_get_name (item));
    gtk_label_set_text (GTK_LABEL (gtk_builder_get_object (builder, "lbl_cmd")), menu_cache_app_get_exec (MENU_CACHE_APP (item)));
    if (menu_cache_item_get_comment (item))
        gtk_label_set_text (GTK_LABEL (gtk_builder_get_object (builder, "lbl_desc")), menu_cache_item_get_comment (item));

    path = menu_cache_item_get_file_path (item);
    gtk_label_set_text (GTK_LABEL (gtk_builder_get_object (builder, "lbl_target")), path);
    g_free (path);

    MenuCacheDir *parent = menu_cache_item_dup_parent (item);
    gtk_label_set_text (GTK_LABEL (lbl_loc), menu_cache_item_get_name (MENU_CACHE_ITEM (parent)));
    menu_cache_item_unref (MENU_CACHE_ITEM (parent));

    g_signal_connect (gtk_builder_get_object (builder, "btn_ok"), "clicked", G_CALLBACK (dialog_cancel), NULL);
    g_signal_connect (gtk_builder_get_object (builder, "btn_edit"), "clicked", G_CALLBACK (edit_item), item);

    gtk_window_set_default_size (GTK_WINDOW (dlg), 500, -1);
    gtk_widget_show (dlg);
    g_object_unref (builder);
}

static void show_icon (const char *name)
{
    GdkPixbuf *pixbuf;
    int scale;

    scale = gtk_widget_get_scale_factor (dlg);
    if (strchr (name, '/'))
        pixbuf = gdk_pixbuf_new_from_file_at_scale (name, scale * 32, scale * 32, TRUE, NULL);
    else
        pixbuf = gtk_icon_theme_load_icon_for_scale (gtk_icon_theme_get_default (), name, 32,
            scale, GTK_ICON_LOOKUP_FORCE_SIZE, NULL);
    set_image_from_pixbuf (img_icon, pixbuf);
    g_object_unref (pixbuf);
}

static void edit_item (GtkButton *, gpointer data)
{
    if (fork () == 0)
    {
        MenuCacheItem *item = (MenuCacheItem *) data;

        char *args[] = { "rpcc", "main_menu", "on_menu_edit", (char *) menu_cache_item_get_file_basename (item), NULL };
        execvp ("rpcc", args);
    }
    gtk_widget_destroy (dlg);
}

static void dialog_cancel (GtkButton *, gpointer)
{
    gtk_widget_destroy (dlg);
}

/* End of file */
/*----------------------------------------------------------------------------*/
