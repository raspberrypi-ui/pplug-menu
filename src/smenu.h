/*============================================================================
Copyright (c) 2022-2025 Raspberry Pi Holdings Ltd.
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

/*----------------------------------------------------------------------------*/
/* Typedefs and macros                                                        */
/*----------------------------------------------------------------------------*/

typedef struct 
{
#ifdef LXPLUG
    LXPanel *panel;                 /* Back pointer to panel */
    config_setting_t *settings;     /* Plugin settings */
#else
    int icon_size;                  /* Variables used under wf-panel */
    gboolean bottom;
    GtkGesture *gesture;
    GtkGesture *migesture;
#endif
    GtkWidget *plugin;              /* Back pointer to the widget */
    GtkWidget *img;                 /* Taskbar icon */
    GtkWidget *menu;                /* Menu */
    GtkWidget *swin;                /* Search window popup */
    GtkWidget *srch;                /* Search window search bar */
    GtkWidget *stv;                 /* Search window tree view */
    GtkWidget *scr;                 /* Search window scrolled window */
    GtkListStore *applist;
    char *icon;
    int padding;
    int height;
    int rheight;
    gboolean fixed;

    MenuCache* menu_cache;
    gpointer reload_notify;
    FmDndSrc *ds;
} MenuPlugin;

/*----------------------------------------------------------------------------*/
/* Prototypes                                                                 */
/*----------------------------------------------------------------------------*/

extern void menu_init (MenuPlugin *m);
extern void menu_update_display (MenuPlugin *m);
extern void menu_set_padding (MenuPlugin *m);
extern void menu_show_menu (MenuPlugin *m);
extern void menu_destructor (gpointer user_data);

/* End of file */
/*----------------------------------------------------------------------------*/
