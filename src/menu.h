typedef struct {
#ifdef LXPLUG
    LXPanel *panel;
    config_setting_t *settings;
#else
    int icon_size;                      /* Variables used under wf-panel */
    gboolean bottom;
    GtkGesture *gesture;
    GtkGesture *migesture;
#endif
    GtkWidget *plugin, *img, *menu;
    GtkWidget *swin, *srch, *stv, *scr;
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


