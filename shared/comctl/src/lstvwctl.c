#include <gdk/gdk.h>
#include <gdk-pixbuf/gdk-pixbuf.h>
#include <glib.h>
#include <gtk/gtk.h>
#include <wintc/comgtk.h>

#include "../public/lstvwctl.h"

#define HITBOX_LARGE_ICON 32

//
// PRIVATE STRUCTURES
//
typedef struct _WinTCCtlListViewIcon
{
    GdkPixbuf* icon;
    gint       x;
    gint       y;
} WinTCCtlListViewIcon;

//
// FORWARD DECLARATIONS
//
static gboolean wintc_ctl_list_view_draw(
    GtkWidget* widget,
    cairo_t*   cr
);

static void wintc_ctl_list_view_create_large_icon(
    WinTCCtlListView* list_view,
    const gchar*      icon_name
);

//
// GTK OOP CLASS/INSTANCE DEFINITIONS
//
struct _WinTCCtlListViewClass
{
    GtkWidgetClass __parent__;
};

struct _WinTCCtlListView
{
    GtkWidget __parent__;

    GList* list_icons;
};

//
// GTK TYPE DEFINITIONS & CTORS
//
G_DEFINE_TYPE(
    WinTCCtlListView,
    wintc_ctl_list_view,
    GTK_TYPE_WIDGET
)

static void wintc_ctl_list_view_class_init(
    WinTCCtlListViewClass* klass
)
{
    GtkWidgetClass* widget_class = GTK_WIDGET_CLASS(klass);

    widget_class->draw = wintc_ctl_list_view_draw;
}

static void wintc_ctl_list_view_init(
    WinTCCtlListView* self
)
{
    gtk_widget_set_has_window(GTK_WIDGET(self), FALSE);

    wintc_ctl_list_view_create_large_icon(self, "folder");
}

//
// CLASS VIRTUAL METHODS
//
static gboolean wintc_ctl_list_view_draw(
    GtkWidget* widget,
    cairo_t*   cr
)
{
    WinTCCtlListView* list_view = WINTC_CTL_LIST_VIEW(widget);

    cairo_save(cr);
    cairo_set_source_rgb(cr, 1.0f, 1.0f, 1.0f);
    cairo_paint(cr);
    cairo_restore(cr);

    // Paint icons
    //
    for (GList* iter = list_view->list_icons; iter; iter = iter->next)
    {
        WinTCCtlListViewIcon* large_icon =
            (WinTCCtlListViewIcon*) iter->data;

        cairo_save(cr);
        cairo_rectangle(
            cr,
            (gdouble) large_icon->x,
            (gdouble) large_icon->y,
            32.0f,
            32.0f
        );
        gdk_cairo_set_source_pixbuf(
            cr,
            large_icon->icon,
            0.0f,
            0.0f
        );
        cairo_paint(cr);
        cairo_restore(cr);
    }

    return FALSE;
}

//
// PUBLIC FUNCTIONS
//
GtkWidget* wintc_ctl_list_view_new(void)
{
    return GTK_WIDGET(
        g_object_new(
            WINTC_TYPE_CTL_LIST_VIEW,
            NULL
        )
    );
}

//
// PRIVATE FUNCTIONS
//
static void wintc_ctl_list_view_create_large_icon(
    WinTCCtlListView* list_view,
    const gchar*      icon_name
)
{
    WinTCCtlListViewIcon* large_icon = g_new(WinTCCtlListViewIcon, 1);

    large_icon->x = 0;
    large_icon->y = 0;

    large_icon->icon =
        gtk_icon_theme_load_icon(
            gtk_icon_theme_get_default(),
            icon_name,
            32,
            GTK_ICON_LOOKUP_FORCE_SIZE,
            NULL
        );

    list_view->list_icons =
        g_list_append(list_view->list_icons, large_icon);

    gtk_widget_queue_draw(GTK_WIDGET(list_view));
}
