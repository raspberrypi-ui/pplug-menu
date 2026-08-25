/*============================================================================
Copyright (c) 2022-2025 Raspberry Pi
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

#include "plugin.h"

#include "smenu.h"

/*----------------------------------------------------------------------------*/
/* LXPanel plugin functions                                                   */
/*----------------------------------------------------------------------------*/

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
    menu_set_values (m);
    lxplug_read_settings (m->settings, conf_table);

    menu_init (m);

    return m->plugin;
}

/* Handler for system config changed message from panel */
static void menu_panel_configuration_changed (LXPanel *, GtkWidget *plugin)
{
    MenuPlugin *m = lxpanel_plugin_get_data (plugin);
    menu_update_display (m);
}

/* Handler for control message */
static gboolean menu_control (GtkWidget *plugin, const char *cmd)
{
    MenuPlugin *m = lxpanel_plugin_get_data (plugin);
    return menu_control_msg (m, cmd);
}

/* Apply changes from config dialog */
static gboolean menu_apply_config (gpointer user_data)
{
    MenuPlugin *m = lxpanel_plugin_get_data (GTK_WIDGET (user_data));
    lxplug_write_settings (m->settings, conf_table);
    menu_update_display (m);
    return FALSE;
}

/* Display configuration dialog */
static GtkWidget *menu_configure (LXPanel *panel, GtkWidget *plugin)
{
    return lxpanel_generic_config_dlg_new (_(PLUGIN_TITLE), panel,
        menu_apply_config, plugin,
        conf_table);
}

int module_lxpanel_gtk_version = 1;
char module_name[] = PLUGIN_NAME;

/* Plugin descriptor */
LXPanelPluginInit fm_module_init_lxpanel_gtk = {
    .name = PLUGIN_TITLE,
    .description = N_("Searchable Application Menu"),
    .new_instance = menu_constructor,
    .reconfigure = menu_panel_configuration_changed,
    .config = menu_configure,
    .control = menu_control,
    .gettext_package = GETTEXT_PACKAGE
};

/* End of file */
/*----------------------------------------------------------------------------*/
