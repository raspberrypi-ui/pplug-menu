/*============================================================================
Copyright (c) 2024 Raspberry Pi Holdings Ltd.
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

#include <glibmm.h>
#include "smenu.hpp"

extern "C" {
    WayfireWidget *create () { return new WayfireSmenu; }
    void destroy (WayfireWidget *w) { delete w; }

    static constexpr conf_table_t conf_table[4] = {
        {CONF_INT,  "padding",          N_("Icon horizontal padding")},
        {CONF_BOOL, "search_fixed",     N_("Fix height of search window")},
        {CONF_INT,  "search_height",    N_("Search window height")},
        {CONF_NONE, NULL,               NULL}
    };
    const conf_table_t *config_params (void) { return conf_table; };
    const char *display_name (void) { return N_("Menu"); };
    const char *package_name (void) { return GETTEXT_PACKAGE; };
}

void WayfireSmenu::bar_pos_changed_cb (void)
{
    if ((std::string) bar_pos == "bottom") m->bottom = TRUE;
    else m->bottom = FALSE;
}

void WayfireSmenu::icon_size_changed_cb (void)
{
    m->icon_size = icon_size;
    menu_update_display (m);
}

void WayfireSmenu::search_param_changed_cb (void)
{
    m->height = search_height;
    m->fixed = search_fixed;
}

void WayfireSmenu::padding_changed_cb (void)
{
    m->padding = padding;
    menu_update_display (m);
}

void WayfireSmenu::command (const char *cmd)
{
    if (!g_strcmp0 (cmd, "menu")) menu_show_menu (m);
}

bool WayfireSmenu::set_icon (void)
{
    menu_update_display (m);
    return false;
}

void WayfireSmenu::init (Gtk::HBox *container)
{
    /* Create the button */
    plugin = std::make_unique <Gtk::Button> ();
    plugin->set_name (PLUGIN_NAME);
    container->pack_start (*plugin, false, false);

    /* Setup structure */
    m = g_new0 (MenuPlugin, 1);
    m->plugin = (GtkWidget *)((*plugin).gobj());
    m->icon_size = icon_size;
    m->height = search_height;
    m->fixed = search_fixed;
    m->padding = padding;
    icon_timer = Glib::signal_idle().connect (sigc::mem_fun (*this, &WayfireSmenu::set_icon));
    bar_pos_changed_cb ();

    /* Initialise the plugin */
    menu_init (m);

    /* Setup callbacks */
    icon_size.set_callback (sigc::mem_fun (*this, &WayfireSmenu::icon_size_changed_cb));
    bar_pos.set_callback (sigc::mem_fun (*this, &WayfireSmenu::bar_pos_changed_cb));
    search_height.set_callback (sigc::mem_fun (*this, &WayfireSmenu::search_param_changed_cb));
    search_fixed.set_callback (sigc::mem_fun (*this, &WayfireSmenu::search_param_changed_cb));
    padding.set_callback (sigc::mem_fun (*this, &WayfireSmenu::padding_changed_cb));
}

WayfireSmenu::~WayfireSmenu()
{
    icon_timer.disconnect ();
    menu_destructor (m);
}

/* End of file */
/*----------------------------------------------------------------------------*/
