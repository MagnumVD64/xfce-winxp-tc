#include <gdk/gdk.h>
#include <glib.h>
#include <gtk/gtk.h>
#include <wintc/comgtk.h>

#include "../public/lstvwctl.h"

//
// FORWARD DECLARATIONS
//
static gboolean wintc_ctl_list_view_draw(
    GtkWidget* widget,
    cairo_t*   cr
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
}

//
// CLASS VIRTUAL METHODS
//
static gboolean wintc_ctl_list_view_draw(
    WINTC_UNUSED(GtkWidget* widget),
    cairo_t* cr
)
{
    cairo_set_source_rgb(cr, 1.0f, 0.0f, 0.0f);
    cairo_paint(cr);
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
