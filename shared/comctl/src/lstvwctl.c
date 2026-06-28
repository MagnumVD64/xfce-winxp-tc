#include <gdk/gdk.h>
#include <gdk-pixbuf/gdk-pixbuf.h>
#include <glib.h>
#include <gtk/gtk.h>
#include <wintc/comgtk.h>

#include "../public/lstvwctl.h"

#define LABEL_TEXT_SHADOW_INTENSITY 3
#define LABEL_TEXT_SHADOW_OFFSET    (LABEL_TEXT_SHADOW_RADIUS * 2)
#define LABEL_TEXT_SHADOW_RADIUS    2

#define HITBOX_LARGE_ICON 32

//
// PRIVATE STRUCTURES
//
typedef struct _WinTCCtlListViewIcon
{
    // Basic state
    //
    GdkPixbuf* icon;
    gchar*     text;
    gint       x;
    gint       y;

    // Drawing cache
    //
    cairo_surface_t* surface_text_shadow;
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
    const gchar*      icon_name,
    const gchar*      text
);
static void wintc_ctl_list_view_update_icon_text_shadow(
    WinTCCtlListView*     list_view,
    WinTCCtlListViewIcon* icon
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

    wintc_ctl_list_view_create_large_icon(self, "folder", "My Stuff");
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

        // Icon itself
        //
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

        // Icon label
        //
        PangoContext*  ctx    = gtk_widget_get_pango_context(widget);
        PangoRectangle extents;
        gdouble        label_x;
        PangoLayout*   layout = pango_layout_new(ctx);

        pango_layout_set_text(layout, large_icon->text, -1);

        pango_layout_get_pixel_extents(
            layout,
            NULL,
            &extents
        );

        cairo_save(cr);
        cairo_set_source_rgba(cr, 0.0f, 0.0f, 0.0f, 1.0f);

        label_x =
            (gdouble) (large_icon->x + 16) - (extents.width / 2);

        for (gint i = 0; i < LABEL_TEXT_SHADOW_INTENSITY; i++)
        {
            cairo_mask_surface(
                cr,
                large_icon->surface_text_shadow,
                label_x - LABEL_TEXT_SHADOW_OFFSET,
                (gdouble) large_icon->y + 35 - LABEL_TEXT_SHADOW_OFFSET
            );
        }

        cairo_restore(cr);

        cairo_set_source_rgb(cr, 1.0f, 1.0f, 1.0f);
        cairo_move_to(
            cr,
            label_x,
            (gdouble) large_icon->y + 35
        );

        pango_cairo_show_layout(cr, layout);

        g_object_unref(layout);
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
    const gchar*      icon_name,
    const gchar*      text
)
{
    WinTCCtlListViewIcon* large_icon = g_new(WinTCCtlListViewIcon, 1);

    large_icon->text = g_strdup(text);
    large_icon->x    = 0;
    large_icon->y    = 0;

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

    large_icon->surface_text_shadow = NULL;
    wintc_ctl_list_view_update_icon_text_shadow(list_view, large_icon);

    gtk_widget_queue_draw(GTK_WIDGET(list_view));
}

static void wintc_ctl_list_view_update_icon_text_shadow(
    WinTCCtlListView*     list_view,
    WinTCCtlListViewIcon* icon
)
{
    PangoContext* ctx = gtk_widget_get_pango_context(GTK_WIDGET(list_view));

    if (icon->surface_text_shadow)
    {
        cairo_surface_destroy(icon->surface_text_shadow);
    }

    // Work out space required for the shadow
    //
    PangoRectangle extents;
    PangoLayout*   layout = pango_layout_new(ctx);

    pango_layout_set_text(layout, icon->text, -1);

    pango_layout_get_pixel_extents(
        layout,
        &extents,
        NULL
    );

    // Render to a backing image surface
    //
    cairo_t* cr;
    gint     surface_height;
    gint     surface_stride;
    gint     surface_width;

    icon->surface_text_shadow =
        cairo_image_surface_create(
            CAIRO_FORMAT_ARGB32,
            extents.width  + (LABEL_TEXT_SHADOW_RADIUS * 4),
            extents.height + (LABEL_TEXT_SHADOW_RADIUS * 4)
        );

    surface_height = cairo_image_surface_get_height(icon->surface_text_shadow);
    surface_stride = cairo_image_surface_get_stride(icon->surface_text_shadow);
    surface_width  = cairo_image_surface_get_width(icon->surface_text_shadow);

    cr = cairo_create(icon->surface_text_shadow);

    cairo_move_to(cr, LABEL_TEXT_SHADOW_RADIUS * 2, LABEL_TEXT_SHADOW_RADIUS * 2);
    cairo_set_source_rgba(cr, 0.0f, 0.0f, 0.0f, 1.0f);

    pango_cairo_show_layout(cr, layout);

    cairo_surface_flush(icon->surface_text_shadow);

    // Perform the blur
    //
    unsigned char* buf = cairo_image_surface_get_data(icon->surface_text_shadow);
    unsigned char* tmp = g_malloc(surface_stride * surface_height);

    for (gint y = 0; y < surface_height; y++) // Horz pass
    {
        unsigned char* src = buf + (y * surface_stride);
        unsigned char* dst = tmp + (y * surface_stride);

        for (gint x = 0; x < surface_width; x++)
        {
            gint len = 0;
            gint sum = 0;

            for (
                gint offset  = -LABEL_TEXT_SHADOW_RADIUS;
                offset      <=  LABEL_TEXT_SHADOW_RADIUS;
                offset++
            )
            {
                gint dx = x + offset;

                if (dx >= 0 && dx < surface_width)
                {
                    sum += src[dx * 4 + 3];
                    len++;
                }
            }

            dst[x * 4 + 3] = sum / len;
        }
    }

    for (gint x = 0; x < surface_width; x++) // Vert pass
    {
        for (gint y = 0; y < surface_height; y++)
        {
            gint len = 0;
            gint sum = 0;

            for (
                gint offset = -LABEL_TEXT_SHADOW_RADIUS;
                offset      <  LABEL_TEXT_SHADOW_RADIUS;
                offset++
            )
            {
                gint dy = y + offset;

                if (dy >= 0 && dy < surface_height)
                {
                    sum += tmp[(dy * surface_stride) + (x * 4) + 3];
                    len++;
                }
            }

            buf[(y * surface_stride) + (x * 4) + 3] = sum / len;
        }
    }

    g_free(tmp);
    cairo_destroy(cr);
    g_object_unref(layout);

    cairo_surface_mark_dirty(icon->surface_text_shadow);
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
