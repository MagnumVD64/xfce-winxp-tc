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

static gboolean on_list_view_button_press_event(
    GtkWidget* widget,
    GdkEvent*  event,
    gpointer   user_data
);
static gboolean on_list_view_button_release_event(
    GtkWidget* widget,
    GdkEvent*  event,
    gpointer   user_data
);
static gboolean on_list_view_motion_notify_event(
    GtkWidget* widget,
    GdkEvent*  event,
    gpointer   user_data
);

//
// GTK OOP CLASS/INSTANCE DEFINITIONS
//
struct _WinTCCtlListViewClass
{
    GtkDrawingAreaClass __parent__;
};

struct _WinTCCtlListView
{
    GtkDrawingArea __parent__;

    GList* list_icons;

    // UI State
    //
    WinTCCtlListViewIcon* hit_icon;
    gint                  hit_x;
    gint                  hit_y;
};

//
// GTK TYPE DEFINITIONS & CTORS
//
G_DEFINE_TYPE(
    WinTCCtlListView,
    wintc_ctl_list_view,
    GTK_TYPE_DRAWING_AREA
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
    gtk_widget_add_events(
        GTK_WIDGET(self),
        GDK_BUTTON_PRESS_MASK  |
        GDK_BUTTON_MOTION_MASK |
        GDK_BUTTON_RELEASE_MASK
    );

    g_signal_connect(
        self,
        "button-press-event",
        G_CALLBACK(on_list_view_button_press_event),
        NULL
    );
    g_signal_connect(
        self,
        "button-release-event",
        G_CALLBACK(on_list_view_button_release_event),
        NULL
    );
    g_signal_connect(
        self,
        "motion-notify-event",
        G_CALLBACK(on_list_view_motion_notify_event),
        NULL
    );

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
        cairo_clip(cr);
        gdk_cairo_set_source_pixbuf(
            cr,
            large_icon->icon,
            (gdouble) large_icon->x,
            (gdouble) large_icon->y
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

//
// CALLBACKS
//
static gboolean on_list_view_button_press_event(
    GtkWidget* widget,
    GdkEvent*  event,
    WINTC_UNUSED(gpointer user_data)
)
{
    GdkEventButton*   e         = (GdkEventButton*) event;
    WinTCCtlListView* list_view = WINTC_CTL_LIST_VIEW(widget);

    WINTC_LOG_DEBUG("Test");

    // Crude hit box search
    //
    for (GList* iter = list_view->list_icons; iter; iter = iter->next)
    {
        WinTCCtlListViewIcon* large_icon =
            (WinTCCtlListViewIcon*) iter->data;

        WINTC_LOG_DEBUG(
            "Hit test x%d y%d against icon x%d y%d",
            (gint) e->x,
            (gint) e->y,
            large_icon->x,
            large_icon->y
        );

        if (
            (
                e->x >= large_icon->x &&
                e->x  < large_icon->x + HITBOX_LARGE_ICON
            ) &&
            (
                e->y >= large_icon->y &&
                e->y <  large_icon->y + HITBOX_LARGE_ICON
            )
        )
        {
            WINTC_LOG_DEBUG("Hit!");

            list_view->hit_icon = large_icon;
            list_view->hit_x    = e->x - large_icon->x;
            list_view->hit_y    = e->y - large_icon->y;

            break;
        }
    }

    return TRUE;
}

static gboolean on_list_view_button_release_event(
    GtkWidget* widget,
    WINTC_UNUSED(GdkEvent*  event),
    WINTC_UNUSED(gpointer   user_data)
)
{
    WinTCCtlListView* list_view = WINTC_CTL_LIST_VIEW(widget);

    list_view->hit_icon = NULL;

    return TRUE;
}

static gboolean on_list_view_motion_notify_event(
    GtkWidget* widget,
    GdkEvent*  event,
    WINTC_UNUSED(gpointer user_data)
)
{
    GdkEventMotion*  e         = (GdkEventMotion*) event;
    WinTCCtlListView* list_view = WINTC_CTL_LIST_VIEW(widget);

    if (!(list_view->hit_icon))
    {
        return TRUE;
    }

    list_view->hit_icon->x = e->x - list_view->hit_x;
    list_view->hit_icon->y = e->y - list_view->hit_y;

    gtk_widget_queue_draw(widget);

    return TRUE;
}
