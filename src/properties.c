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

#include <gtk/gtk.h>
#include <menu-cache.h>


#define     GET_WIDGET(transform,name) name = transform(gtk_builder_get_object(builder, #name))


GtkDialog* fm_file_properties_widget_new (void)
{
    GtkBuilder* builder=gtk_builder_new();
    GtkDialog* dlg;
    //FmFilePropData* data;
    //FmPathList* paths;

    gtk_builder_set_translation_domain(builder, GETTEXT_PACKAGE);
    //data = g_slice_new0(FmFilePropData);

    //data->files = fm_file_info_list_ref(files);
    //data->single_type = fm_file_info_list_is_same_type(files);
    //data->single_file = (fm_file_info_list_get_length(files) == 1);
    //data->fi = fm_file_info_list_peek_head(files);
    //if(data->single_type)
    //    data->mime_type = fm_mime_type_ref(fm_file_info_get_mime_type(data->fi));
    //paths = fm_path_list_new_from_file_info_list(files);
    //data->dc_job = fm_deep_count_job_new(paths, FM_DC_JOB_DEFAULT);
    //fm_path_list_unref(paths);
    //data->ext = NULL; /* no extension by default */
    //data->extdata = NULL;

    gtk_builder_add_from_file (builder, PACKAGE_DATA_DIR "/ui/file-prop.ui", NULL);
    GET_WIDGET (GTK_DIALOG, dlg);
#if 0
    gtk_dialog_set_alternative_button_order (GTK_DIALOG (data->dlg), GTK_RESPONSE_OK, GTK_RESPONSE_CANCEL, -1);
    dlg = data->dlg;

    GET_WIDGET(GTK_TABLE,general_table);
    GET_WIDGET(GTK_IMAGE,icon);
    GET_WIDGET(GTK_WIDGET,icon_eventbox);
    GET_WIDGET(GTK_ENTRY,name);
    GET_WIDGET(GTK_LABEL,file);
    GET_WIDGET(GTK_LABEL,file_label);
    GET_WIDGET(GTK_LABEL,dir);
    GET_WIDGET(GTK_LABEL,target);
    GET_WIDGET(GTK_WIDGET,target_label);
    GET_WIDGET(GTK_LABEL,type);
    GET_WIDGET(GTK_WIDGET,open_with_label);
    GET_WIDGET(GTK_COMBO_BOX,open_with);
    GET_WIDGET(GTK_LABEL,total_files);
    GET_WIDGET(GTK_WIDGET,total_files_label);
    GET_WIDGET(GTK_LABEL,total_size);
    GET_WIDGET(GTK_LABEL,size_on_disk);
    GET_WIDGET(GTK_LABEL,mtime);
    GET_WIDGET(GTK_WIDGET,mtime_label);
    GET_WIDGET(GTK_LABEL,atime);
    GET_WIDGET(GTK_WIDGET,atime_label);
    GET_WIDGET(GTK_LABEL,ctime);
    GET_WIDGET(GTK_WIDGET,ctime_label);

    GET_WIDGET(GTK_WIDGET,permissions_tab);
    GET_WIDGET(GTK_ENTRY,owner);
    GET_WIDGET(GTK_ENTRY,group);

    GET_WIDGET(GTK_COMBO_BOX,read_perm);
    GET_WIDGET(GTK_COMBO_BOX,write_perm);
    GET_WIDGET(GTK_LABEL,exec_label);
    GET_WIDGET(GTK_COMBO_BOX,exec_perm);
    GET_WIDGET(GTK_LABEL,flags_label);
    GET_WIDGET(GTK_COMBO_BOX,flags_set_file);
    GET_WIDGET(GTK_COMBO_BOX,flags_set_dir);
    GET_WIDGET(GTK_WIDGET,hidden);

    init_application_list(data);

    data->timeout = gdk_threads_add_timeout(600, on_timeout, data);
    g_signal_connect(dlg, "response", G_CALLBACK(on_response), data);
    g_signal_connect_swapped(dlg, "destroy", G_CALLBACK(fm_file_prop_data_free), data);
    g_signal_connect(data->dc_job, "finished", G_CALLBACK(on_finished), data);

    g_signal_connect(data->icon_eventbox, "button-press-event",
                     G_CALLBACK(_icon_click_event), data);
    g_signal_connect(data->icon_eventbox, "key-press-event",
                     G_CALLBACK(_icon_press_event), data);

    if (!fm_job_run_async(FM_JOB(data->dc_job)))
    {
        g_object_unref(data->dc_job);
        data->dc_job = NULL;
        g_critical("failed to run scanning job for file properties dialog");
    }

    update_ui(data);

    /* if we got some extension then activate it updating dialog window */
    if(data->ext != NULL)
    {
        GSList *l, *l2;
        for (l = data->ext, l2 = data->extdata; l; l = l->next, l2 = l2->next)
            l2->data = ((FmFilePropExt*)l->data)->cb.init(builder, data, data->files);
    }
    /* add this after all updates from extensions was made */
    if (gtk_widget_get_can_focus(data->icon_eventbox))
    {
        /* the dialog isn't realized yet so set cursor in callback */
        g_signal_connect(data->icon_eventbox, "enter-notify-event",
                         G_CALLBACK(on_icon_enter_notify), data);
    }
#endif
    g_object_unref(builder);

    return dlg;
}


void show_properties_dialog (MenuCacheItem *item)
{
    GtkDialog* dlg = fm_file_properties_widget_new ();
    //if (parent) gtk_window_set_transient_for (GTK_WINDOW (dlg), parent);
    gtk_widget_show (GTK_WIDGET (dlg));
    g_signal_connect_after (dlg, "response", G_CALLBACK (gtk_widget_destroy), NULL);
}

/* End of file */
/*----------------------------------------------------------------------------*/
