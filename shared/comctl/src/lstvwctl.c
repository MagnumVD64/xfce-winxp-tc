#include <gdk/gdk.h>
#include <gdk-pixbuf/gdk-pixbuf.h>
#include <glib.h>
#include <gtk/gtk.h>
#include <wintc/comgtk.h>

#include "../public/lstvwctl.h"

#define LABEL_TEXT_SHADOW_INTENSITY 3
#define LABEL_TEXT_SHADOW_OFFSET    (LABEL_TEXT_SHADOW_RADIUS * 2)
#define LABEL_TEXT_SHADOW_RADIUS    2

#define DRAG_THRESHOLD    5
#define HITBOX_LARGE_ICON 32

#define ICON_IS_COMMITTED(i) (i->icon && i->text)

//
// PRIVATE ENUMS
//
typedef enum _WinTCCtlListViewIconStyle
{
    WINTC_CTL_LIST_VIEW_ICON_STYLE_REGULAR,
    WINTC_CTL_LIST_VIEW_ICON_STYLE_SELECTED,
    WINTC_CTL_LIST_VIEW_ICON_STYLE_GHOSTED
} WinTCCtlListViewIconStyle;

//
// PRIVATE STRUCTURES
//
typedef struct _WinTCCtlListViewIcon
{
    // Basic state
    //
    GdkPixbuf* icon;
    gchar*     text;

    GdkRectangle hitbox_icon;
    GdkRectangle hitbox_label;

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

static void wintc_ctl_list_view_commit_icon_drag(
    WinTCCtlListView* list_view
);
static WinTCCtlListViewIcon* wintc_ctl_list_view_create_large_icon(
    WinTCCtlListView* list_view
);
static WinTCCtlListViewIcon* wintc_ctl_list_view_hit_test(
    WinTCCtlListView* list_view,
    gint              x,
    gint              y,
    gboolean*         hit_label
);
static void wintc_ctl_list_view_render_large_icon(
    WinTCCtlListView*         list_view,
    WinTCCtlListViewIcon*     large_icon,
    cairo_t*                  cr,
    WinTCCtlListViewIconStyle style,
    gint                      offset_x,
    gint                      offset_y
);
static void wintc_ctl_list_view_reset_hit_state(
    WinTCCtlListView* list_view
);
static void wintc_ctl_list_view_set_icon_pixbuf(
    WinTCCtlListView*     list_view,
    WinTCCtlListViewIcon* icon,
    GdkPixbuf*            pixbuf
);
static void wintc_ctl_list_view_set_icon_text(
    WinTCCtlListView*     list_view,
    WinTCCtlListViewIcon* icon,
    gchar*                text
);
static void wintc_ctl_list_view_update_dnd_state(
    WinTCCtlListView* list_view
);
static void wintc_ctl_list_view_update_icon(
    WinTCCtlListView*     list_view,
    WinTCCtlListViewIcon* icon,
    GtkTreeIter*          iter
);

static void on_list_view_drag_end(
    GtkWidget*      widget,
    GdkDragContext* context,
    gpointer        user_data
);
static gboolean on_list_view_drag_motion(
    GtkWidget*      widget,
    GdkDragContext* context,
    int             x,
    int             y,
    guint           time,
    gpointer        user_data
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
static gboolean on_list_view_enter_notify_event(
    GtkWidget* widget,
    GdkEvent*  event,
    gpointer   user_data
);
static gboolean on_list_view_leave_notify_event(
    GtkWidget* widget,
    GdkEvent*  event,
    gpointer   user_data
);
static gboolean on_list_view_motion_notify_event(
    GtkWidget* widget,
    GdkEvent*  event,
    gpointer   user_data
);

static void on_model_row_changed(
    GtkTreeModel* tree_model,
    GtkTreePath*  path,
    GtkTreeIter*  iter,
    gpointer      user_data
);
static void on_model_row_deleted(
    GtkTreeModel* tree_model,
    GtkTreePath*  path,
    gpointer      user_data
);
static void on_model_row_inserted(
    GtkTreeModel* tree_model,
    GtkTreePath*  path,
    GtkTreeIter*  iter,
    gpointer      user_data
);
static void on_model_rows_reordered(
    GtkTreeModel* tree_model,
    GtkTreePath*  path,
    GtkTreeIter*  iter,
    gpointer      new_order,
    gpointer      user_data
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
    GList* list_selected;

    // Model related
    //
    GtkTreeModel* model;
    gint          col_pixbuf;
    gint          col_text;

    GSequence* seq_icons;

    gulong sigid_row_changed;
    gulong sigid_row_deleted;
    gulong sigid_row_inserted;

    // UI State
    //
    GdkRectangle motion_rect;

    WinTCCtlListViewIcon* hit_icon;
    gboolean              hit_started;
    gboolean              hit_dragging;
    gboolean              hit_in_widget;

    // DND stuff
    //
    GtkTargetList*  dnd_dest_targets;
    GdkDragAction   dnd_dest_actions;
    GtkTargetList*  dnd_src_targets;
    GdkDragAction   dnd_src_actions;
    GdkDragContext* dnd_ctx;

    WinTCCtlListViewIcon* dnd_icon_target;
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
    self->col_pixbuf = -1;
    self->col_text   = -1;
    self->seq_icons = g_sequence_new(NULL);

    gtk_widget_add_events(
        GTK_WIDGET(self),
        GDK_BUTTON_PRESS_MASK   |
        GDK_BUTTON_MOTION_MASK  |
        GDK_BUTTON_RELEASE_MASK |
        GDK_ENTER_NOTIFY_MASK   |
        GDK_LEAVE_NOTIFY_MASK
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
        "drag-end",
        G_CALLBACK(on_list_view_drag_end),
        NULL
    );
    g_signal_connect(
        self,
        "drag-motion",
        G_CALLBACK(on_list_view_drag_motion),
        NULL
    );
    g_signal_connect(
        self,
        "motion-notify-event",
        G_CALLBACK(on_list_view_motion_notify_event),
        NULL
    );
    g_signal_connect(
        self,
        "enter-notify-event",
        G_CALLBACK(on_list_view_enter_notify_event),
        NULL
    );
    g_signal_connect(
        self,
        "leave-notify-event",
        G_CALLBACK(on_list_view_leave_notify_event),
        NULL
    );
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

        if (!ICON_IS_COMMITTED(large_icon))
        {
            continue;
        }

        gboolean render_selected =
            large_icon == list_view->dnd_icon_target ||
            g_list_find(list_view->list_selected, large_icon);

        wintc_ctl_list_view_render_large_icon(
            list_view,
            large_icon,
            cr,
            render_selected ?
                WINTC_CTL_LIST_VIEW_ICON_STYLE_SELECTED :
                WINTC_CTL_LIST_VIEW_ICON_STYLE_REGULAR,
            0,
            0
        );
    }

    // Handle dragging ops...
    //
    if (list_view->hit_started)
    {
        if (list_view->hit_dragging) // Dragging icons
        {
            for (
                GList* iter = list_view->list_selected;
                iter;
                iter = iter->next
            )
            {
                WinTCCtlListViewIcon* large_icon =
                    (WinTCCtlListViewIcon*) iter->data;

                wintc_ctl_list_view_render_large_icon(
                    list_view,
                    large_icon,
                    cr,
                    WINTC_CTL_LIST_VIEW_ICON_STYLE_GHOSTED,
                    list_view->motion_rect.width,
                    list_view->motion_rect.height
                );
            }
        }
        else // Selection box drag
        {
            const gdouble s_dashes[] = { 1.0f };

            cairo_rectangle(
                cr,
                (gdouble) list_view->motion_rect.x,
                (gdouble) list_view->motion_rect.y,
                (gdouble) list_view->motion_rect.width,
                (gdouble) list_view->motion_rect.height
            );

            cairo_set_dash(
                cr,
                s_dashes,
                1,
                0.0f
            );
            cairo_set_operator(cr, CAIRO_OPERATOR_XOR);
            cairo_set_source_rgb(cr, 0.0f, 0.0f, 0.0f);

            cairo_stroke(cr);
        }
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

void wintc_ctl_list_view_enable_drag_dest(
    WinTCCtlListView*     list_view,
    const GtkTargetEntry* targets,
    gint                  n_targets,
    GdkDragAction         actions
)
{
    if (list_view->dnd_dest_targets)
    {
        wintc_ctl_list_view_unset_drag_dest(list_view);
    }

    list_view->dnd_dest_targets = gtk_target_list_new(targets, n_targets);
    list_view->dnd_dest_actions = actions;

    wintc_ctl_list_view_update_dnd_state(list_view);
}

void wintc_ctl_list_view_enable_drag_source(
    WinTCCtlListView*     list_view,
    const GtkTargetEntry* targets,
    gint                  n_targets,
    GdkDragAction         actions
)
{
    if (list_view->dnd_src_targets)
    {
        wintc_ctl_list_view_unset_drag_source(list_view);
    }

    list_view->dnd_src_targets = gtk_target_list_new(targets, n_targets);
    list_view->dnd_src_actions = actions;

    // Must set ourselves up as a drag destination to be able to receive
    // drag motion events
    //
    gtk_drag_dest_set(
        GTK_WIDGET(list_view),
        0,
        NULL,
        0,
        GDK_ACTION_COPY
    );

    wintc_ctl_list_view_update_dnd_state(list_view);
}

GtkTreeModel* wintc_ctl_list_view_get_model(
    WinTCCtlListView* list_view
)
{
    return list_view->model;
}

gint wintc_ctl_list_view_get_pixbuf_column(
    WinTCCtlListView* list_view
)
{
    return list_view->col_pixbuf;
}

gint wintc_ctl_list_view_get_text_column(
    WinTCCtlListView* list_view
)
{
    return list_view->col_text;
}

void wintc_ctl_list_view_set_model(
    WinTCCtlListView* list_view,
    GtkTreeModel*     model
)
{
    //
    // FIXME: Unset existing model
    //

    list_view->model = model;

    list_view->sigid_row_changed =
        g_signal_connect_object(
            list_view->model,
            "row-changed",
            G_CALLBACK(on_model_row_changed),
            list_view,
            G_CONNECT_DEFAULT
        );
    list_view->sigid_row_deleted =
        g_signal_connect_object(
            list_view->model,
            "row-deleted",
            G_CALLBACK(on_model_row_deleted),
            list_view,
            G_CONNECT_DEFAULT
        );
    list_view->sigid_row_inserted =
        g_signal_connect_object(
            list_view->model,
            "row-inserted",
            G_CALLBACK(on_model_row_inserted),
            list_view,
            G_CONNECT_DEFAULT
        );
    list_view->sigid_row_inserted =
        g_signal_connect_object(
            list_view->model,
            "rows-reordered",
            G_CALLBACK(on_model_rows_reordered),
            list_view,
            G_CONNECT_DEFAULT
        );

    //
    // FIXME: Iterate over model
    //
}

void wintc_ctl_list_view_set_pixbuf_column(
    WinTCCtlListView* list_view,
    gint              column
)
{
    list_view->col_pixbuf = column;

    //
    // FIXME: Update existing icons now
    //
}

void wintc_ctl_list_view_set_text_column(
    WinTCCtlListView* list_view,
    gint              column
)
{
    list_view->col_text = column;

    //
    // FIXME: Update existing icons now
    //
}

void wintc_ctl_list_view_unset_drag_dest(
    WinTCCtlListView* list_view
)
{
    if (list_view->dnd_dest_targets)
    {
        gtk_target_list_unref(list_view->dnd_dest_targets);
    }

    list_view->dnd_dest_targets   = NULL;
    list_view->dnd_dest_actions   = 0;

    wintc_ctl_list_view_update_dnd_state(list_view);
}

void wintc_ctl_list_view_unset_drag_source(
    WinTCCtlListView* list_view
)
{
    if (list_view->dnd_ctx)
    {
        gtk_drag_cancel(list_view->dnd_ctx);
        g_clear_object(&(list_view->dnd_ctx));
    }

    if (list_view->dnd_src_targets)
    {
        gtk_target_list_unref(list_view->dnd_src_targets);
    }

    list_view->dnd_src_targets = NULL;
    list_view->dnd_src_actions = 0;

    wintc_ctl_list_view_update_dnd_state(list_view);
}

//
// PRIVATE FUNCTIONS
//
static void wintc_ctl_list_view_commit_icon_drag(
    WinTCCtlListView* list_view
)
{
    for (GList* iter = list_view->list_selected; iter; iter = iter->next)
    {
        WinTCCtlListViewIcon* large_icon =
            (WinTCCtlListViewIcon*) iter->data;

        large_icon->hitbox_icon.x  += list_view->motion_rect.width;
        large_icon->hitbox_icon.y  += list_view->motion_rect.height;
        large_icon->hitbox_label.x += list_view->motion_rect.width;
        large_icon->hitbox_label.y += list_view->motion_rect.height;
    }
}

static WinTCCtlListViewIcon* wintc_ctl_list_view_create_large_icon(
    WINTC_UNUSED(WinTCCtlListView* list_view)
)
{
    WinTCCtlListViewIcon* large_icon = g_new0(WinTCCtlListViewIcon, 1);

    large_icon->hitbox_icon.width  = HITBOX_LARGE_ICON;
    large_icon->hitbox_icon.height = HITBOX_LARGE_ICON;

    return large_icon;
}

static WinTCCtlListViewIcon* wintc_ctl_list_view_hit_test(
    WinTCCtlListView* list_view,
    gint              x,
    gint              y,
    gboolean*         hit_label
)
{
    // Crude hit box search
    //
    for (GList* iter = list_view->list_icons; iter; iter = iter->next)
    {
        WinTCCtlListViewIcon* large_icon =
            (WinTCCtlListViewIcon*) iter->data;

        if (
            wintc_point_xy_in_rect(
                x,
                y,
                &(large_icon->hitbox_icon)
            )
        )
        {
            return large_icon;
        }

        if (
            wintc_point_xy_in_rect(
                x,
                y,
                &(large_icon->hitbox_label)
            )
        )
        {
            WINTC_SAFE_REF_SET(hit_label, TRUE);
            return large_icon;
        }
    }

    return NULL;
}

static void wintc_ctl_list_view_render_large_icon(
    WinTCCtlListView*         list_view,
    WinTCCtlListViewIcon*     large_icon,
    cairo_t*                  cr,
    WinTCCtlListViewIconStyle style,
    gint                      offset_x,
    gint                      offset_y
)
{
    gdouble          alpha        = 1.0f;
    cairo_surface_t* surface_icon = NULL;

    if (style == WINTC_CTL_LIST_VIEW_ICON_STYLE_GHOSTED)
    {
        alpha = 0.5f;
    }

    // Icon itself
    //
    gdk_cairo_set_source_pixbuf(
        cr,
        large_icon->icon,
        (gdouble) large_icon->hitbox_icon.x + offset_x,
        (gdouble) large_icon->hitbox_icon.y + offset_y
    );

    cairo_pattern_get_surface(
        cairo_get_source(cr),
        &surface_icon
    );

    cairo_paint_with_alpha(cr, alpha);

    // If the icon is selected, it needs the highlight
    //
    if (style == WINTC_CTL_LIST_VIEW_ICON_STYLE_SELECTED)
    {
        cairo_save(cr);
        cairo_set_source_rgba(cr, 0.0f, 0.0f, 0.8f, 0.5f);
        cairo_mask_surface(
            cr,
            surface_icon,
            (gdouble) large_icon->hitbox_icon.x + offset_x,
            (gdouble) large_icon->hitbox_icon.y + offset_y
        );
        cairo_restore(cr);
    }

    // Icon label
    //
    PangoContext* ctx    = gtk_widget_get_pango_context(GTK_WIDGET(list_view));
    PangoLayout*  layout = pango_layout_new(ctx);

    pango_layout_set_text(layout, large_icon->text, -1);

    if (style == WINTC_CTL_LIST_VIEW_ICON_STYLE_REGULAR)
    {
        cairo_save(cr);
        cairo_set_source_rgba(cr, 0.0f, 0.0f, 0.0f, 1.0f);

        for (gint i = 0; i < LABEL_TEXT_SHADOW_INTENSITY; i++)
        {
            cairo_mask_surface(
                cr,
                large_icon->surface_text_shadow,
                (gdouble) large_icon->hitbox_label.x +
                offset_x - LABEL_TEXT_SHADOW_OFFSET,
                (gdouble) large_icon->hitbox_label.y +
                offset_y - LABEL_TEXT_SHADOW_OFFSET
            );
        }

        cairo_restore(cr);
    }
    else
    {
        cairo_save(cr);
        cairo_rectangle(
            cr,
            (gdouble) large_icon->hitbox_label.x + offset_x,
            (gdouble) large_icon->hitbox_label.y + offset_y,
            (gdouble) large_icon->hitbox_label.width,
            (gdouble) large_icon->hitbox_label.height
        );
        cairo_clip(cr);
        cairo_set_source_rgba(cr, 0.0f, 0.0f, 0.4f, alpha);
        cairo_paint(cr);

        cairo_set_source_rgba(cr, 1.0f, 1.0f, 1.0f, alpha);
        cairo_stroke(cr);
        cairo_restore(cr);
    }

    cairo_set_source_rgb(cr, 1.0f, 1.0f, 1.0f);
    cairo_move_to(
        cr,
        large_icon->hitbox_label.x + offset_x,
        large_icon->hitbox_label.y + offset_y
    );

    pango_cairo_show_layout(cr, layout);

    g_object_unref(layout);
}

static void wintc_ctl_list_view_reset_hit_state(
    WinTCCtlListView* list_view
)
{
    memset(&(list_view->motion_rect), 0, sizeof(GdkRectangle));
    list_view->hit_icon        = NULL;
    list_view->hit_started     = FALSE;
    list_view->hit_dragging    = FALSE;
    list_view->dnd_icon_target = NULL;
}

static void wintc_ctl_list_view_set_icon_pixbuf(
    WINTC_UNUSED(WinTCCtlListView* list_view),
    WinTCCtlListViewIcon* icon,
    GdkPixbuf*            pixbuf
)
{
    if (icon->icon)
    {
        g_clear_object(&(icon->icon));
    }

    if (!pixbuf)
    {
        return;
    }

    icon->icon = pixbuf; // Ref should've already been taken from model_get
}

static void wintc_ctl_list_view_set_icon_text(
    WinTCCtlListView*     list_view,
    WinTCCtlListViewIcon* icon,
    gchar*                text
)
{
    PangoContext* ctx = gtk_widget_get_pango_context(GTK_WIDGET(list_view));

    g_free(icon->text);
    icon->text = text; // Ref should've already been taken from model_get

    if (icon->surface_text_shadow)
    {
        cairo_surface_destroy(icon->surface_text_shadow);
    }

    if (!text)
    {
        return;
    }

    // Retrieve the extents for the text used for the shadow (ink extents) and
    // hitbox (logical extents)
    //
    PangoRectangle extents;
    PangoLayout*   layout = pango_layout_new(ctx);

    pango_layout_set_text(layout, icon->text, -1);

    pango_layout_get_pixel_extents(
        layout,
        &extents,
        (PangoRectangle*) &(icon->hitbox_label)
    );

    icon->hitbox_label.x =
        icon->hitbox_icon.x +
        (icon->hitbox_icon.width / 2) - (icon->hitbox_label.width / 2);
    icon->hitbox_label.y =
        icon->hitbox_icon.y + 35;

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

static void wintc_ctl_list_view_update_dnd_state(
    WinTCCtlListView* list_view
)
{
    gtk_drag_dest_unset(GTK_WIDGET(list_view));

    if (
        list_view->dnd_src_targets ||
        list_view->dnd_dest_targets
    )
    {
        gtk_drag_dest_set(
            GTK_WIDGET(list_view),
            0,
            NULL,
            0,
            list_view->dnd_dest_actions ?
                list_view->dnd_dest_actions :
                GDK_ACTION_COPY
        );

        if (list_view->dnd_dest_targets)
        {
            gtk_drag_dest_set_target_list(
                GTK_WIDGET(list_view),
                list_view->dnd_dest_targets
            );
        }
    }
}

static void wintc_ctl_list_view_update_icon(
    WinTCCtlListView*     list_view,
    WinTCCtlListViewIcon* icon,
    GtkTreeIter*          iter
)
{
    if (list_view->col_pixbuf > -1)
    {
        GdkPixbuf* pixbuf = NULL;

        gtk_tree_model_get(
            list_view->model,
            iter,
            list_view->col_pixbuf, &pixbuf,
            -1
        );

        wintc_ctl_list_view_set_icon_pixbuf(
            list_view,
            icon,
            pixbuf
        );
    }

    if (list_view->col_text > -1)
    {
        gchar* text = NULL;

        gtk_tree_model_get(
            list_view->model,
            iter,
            list_view->col_text, &text,
            -1
        );

        wintc_ctl_list_view_set_icon_text(
            list_view,
            icon,
            text
        );
    }
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

    if (e->button & GDK_BUTTON1_MASK)
    {
        return FALSE;
    }

    list_view->hit_started = TRUE;

    list_view->motion_rect.x = e->x;
    list_view->motion_rect.y = e->y;

    // Hit check
    //
    list_view->hit_icon =
        wintc_ctl_list_view_hit_test(
            list_view,
            e->x,
            e->y,
            NULL
        );

    // Update the selection state for whether we hit the icon or not
    //
    if (list_view->hit_icon)
    {
        if (
            !list_view->list_selected ||
            !g_list_find(
                list_view->list_selected,
                list_view->hit_icon
            )
        )
        {
            g_clear_list(&(list_view->list_selected), NULL);

            list_view->list_selected =
                g_list_append(
                    list_view->list_selected,
                    list_view->hit_icon
                );
        }
    }
    else
    {
        g_clear_list(&(list_view->list_selected), NULL);
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

    if (list_view->hit_dragging)
    {
        wintc_ctl_list_view_commit_icon_drag(list_view);
    }

    wintc_ctl_list_view_reset_hit_state(list_view);

    gtk_widget_queue_draw(widget);

    return TRUE;
}

static void on_list_view_drag_end(
    GtkWidget* widget,
    WINTC_UNUSED(GdkDragContext* context),
    WINTC_UNUSED(gpointer user_data)
)
{
    WinTCCtlListView* list_view = WINTC_CTL_LIST_VIEW(widget);

    WINTC_LOG_DEBUG("Ended drag");

    // If we were dragging icons then commit the drag now
    //
    if (list_view->dnd_ctx)
    {
        wintc_ctl_list_view_commit_icon_drag(list_view);
        wintc_ctl_list_view_reset_hit_state(list_view);
    }

    list_view->dnd_ctx = NULL;

    gtk_widget_queue_draw(widget);
}

static gboolean on_list_view_drag_motion(
    GtkWidget* widget,
    WINTC_UNUSED(GdkDragContext* context),
    int        x,
    int        y,
    WINTC_UNUSED(guint time),
    WINTC_UNUSED(gpointer user_data)
)
{
    WinTCCtlListView* list_view = WINTC_CTL_LIST_VIEW(widget);

    // If it's our drag, update the dragging position
    //
    if (list_view->dnd_ctx)
    {
        list_view->motion_rect.width =
            x - list_view->motion_rect.x;
        list_view->motion_rect.height =
            y - list_view->motion_rect.y;
    }

    list_view->dnd_icon_target =
        wintc_ctl_list_view_hit_test(
            list_view,
            x,
            y,
            NULL
        );

    gtk_widget_queue_draw(widget);

    return FALSE;
}

static gboolean on_list_view_enter_notify_event(
    GtkWidget* widget,
    WINTC_UNUSED(GdkEvent*  event),
    WINTC_UNUSED(gpointer   user_data)
)
{
    WinTCCtlListView* list_view = WINTC_CTL_LIST_VIEW(widget);

    list_view->hit_in_widget = TRUE;

    return FALSE;
}

static gboolean on_list_view_leave_notify_event(
    GtkWidget* widget,
    WINTC_UNUSED(GdkEvent*  event),
    WINTC_UNUSED(gpointer   user_data)
)
{
    WinTCCtlListView* list_view = WINTC_CTL_LIST_VIEW(widget);

    list_view->hit_in_widget = FALSE;

    return FALSE;
}

static gboolean on_list_view_motion_notify_event(
    GtkWidget* widget,
    GdkEvent*  event,
    WINTC_UNUSED(gpointer user_data)
)
{
    GdkEventMotion*   e         = (GdkEventMotion*) event;
    WinTCCtlListView* list_view = WINTC_CTL_LIST_VIEW(widget);

    // Do not update any position information if we're not in bounds
    //
    if (!(list_view->hit_in_widget))
    {
        return FALSE;
    }

    list_view->motion_rect.width =
        e->x - list_view->motion_rect.x;
    list_view->motion_rect.height =
        e->y - list_view->motion_rect.y;

    // If no icon hit-tested, then this is a selection box dragging op
    //
    if (list_view->hit_icon)
    {
        if (
            !(list_view->hit_dragging) &&
            (
                abs(list_view->motion_rect.width)  > DRAG_THRESHOLD ||
                abs(list_view->motion_rect.height) > DRAG_THRESHOLD
            )
        )
        {
            list_view->hit_dragging = TRUE;

            if (list_view->dnd_src_targets)
            {
                WINTC_LOG_DEBUG("Initiate drag");

                list_view->dnd_ctx =
                    gtk_drag_begin_with_coordinates(
                        widget,
                        list_view->dnd_src_targets,
                        list_view->dnd_src_actions,
                        GDK_BUTTON1_MASK,
                        event,
                        -1,
                        -1
                    );
            }
        }
    }
    else
    {
        g_clear_list(&(list_view->list_selected), NULL);

        for (GList* iter = list_view->list_icons; iter; iter = iter->next)
        {
            WinTCCtlListViewIcon* large_icon =
                (WinTCCtlListViewIcon*) iter->data;

            GdkRectangle rect = list_view->motion_rect;

            wintc_rectangle_normalize(&rect);

            if (
                gdk_rectangle_intersect(
                    &rect,
                    &(large_icon->hitbox_icon),
                    NULL
                ) ||
                gdk_rectangle_intersect(
                    &rect,
                    &(large_icon->hitbox_label),
                    NULL
                )
            )
            {
                list_view->list_selected =
                    g_list_prepend(
                        list_view->list_selected,
                        large_icon
                    );
            }
        }

        list_view->list_selected =
            g_list_reverse(list_view->list_selected);
    }

    gtk_widget_queue_draw(widget);

    return TRUE;
}

static void on_model_row_changed(
    WINTC_UNUSED(GtkTreeModel* tree_model),
    GtkTreePath* path,
    GtkTreeIter* iter,
    gpointer     user_data
)
{
    WinTCCtlListView* list_view = WINTC_CTL_LIST_VIEW(user_data);

    if (gtk_tree_path_get_depth(path) > 1)
    {
        return;
    }

    gint idx = gtk_tree_path_get_indices(path)[0];

    GSequenceIter* iter_seq =
        g_sequence_get_iter_at_pos(
            list_view->seq_icons,
            idx
        );

    WinTCCtlListViewIcon* large_icon = g_sequence_get(iter_seq);

    wintc_ctl_list_view_update_icon(list_view, large_icon, iter);

    gtk_widget_queue_draw(GTK_WIDGET(list_view));

    /**
    large_icon->icon =
        gtk_icon_theme_load_icon(
            gtk_icon_theme_get_default(),
            icon_name,
            32,
            GTK_ICON_LOOKUP_FORCE_SIZE,
            NULL
        );

    wintc_ctl_list_view_set_icon_text(
        list_view,
        large_icon,
        text
    );
    */
}

static void on_model_row_deleted(
    WINTC_UNUSED(GtkTreeModel* tree_model),
    GtkTreePath* path,
    gpointer     user_data
)
{
    WinTCCtlListView* list_view = WINTC_CTL_LIST_VIEW(user_data);

    if (gtk_tree_path_get_depth(path) > 1)
    {
        return;
    }

    gint idx = gtk_tree_path_get_indices(path)[0];

    GSequenceIter* iter =
        g_sequence_get_iter_at_pos(
            list_view->seq_icons,
            idx
        );

    WinTCCtlListViewIcon* large_icon = g_sequence_get(iter);

    g_sequence_remove(iter);

    list_view->list_icons =
        g_list_delete_link(
            list_view->list_icons,
            g_list_find(
                list_view->list_icons,
                large_icon
            )
        );

    //
    // FIXME: Free large icon
    //

    gtk_widget_queue_draw(GTK_WIDGET(list_view));
}

static void on_model_row_inserted(
    WINTC_UNUSED(GtkTreeModel* tree_model),
    GtkTreePath*  path,
    WINTC_UNUSED(GtkTreeIter* iter),
    gpointer      user_data
)
{
    WinTCCtlListView* list_view = WINTC_CTL_LIST_VIEW(user_data);

    if (gtk_tree_path_get_depth(path) > 1)
    {
        return;
    }

    gint idx = gtk_tree_path_get_indices(path)[0];

    WinTCCtlListViewIcon* large_icon =
        wintc_ctl_list_view_create_large_icon(list_view);

    GSequenceIter* iter_seq =
        g_sequence_get_iter_at_pos(list_view->seq_icons, idx);

    g_sequence_insert_before(iter_seq, large_icon);

    list_view->list_icons =
        g_list_prepend(list_view->list_icons, large_icon);

    gtk_widget_queue_draw(GTK_WIDGET(list_view));
}

static void on_model_rows_reordered(
    WINTC_UNUSED(GtkTreeModel* tree_model),
    GtkTreePath* path,
    GtkTreeIter* iter,
    gpointer     new_order,
    gpointer     user_data
)
{
    WinTCCtlListView* list_view = WINTC_CTL_LIST_VIEW(user_data);

    if (gtk_tree_path_get_depth(path) > 0)
    {
        return;
    }

    // Create a new GSequence to replace the existing one
    //
    gint       len = gtk_tree_model_iter_n_children(
                         list_view->model,
                         iter
                     );
    GSequence* seq = g_sequence_new(NULL);

    for (gint i = 0; i < len; i++)
    {
        gint old_idx = ((gint*) new_order)[i];

        GSequenceIter* iter = g_sequence_get_iter_at_pos(list_view->seq_icons, old_idx);

        WinTCCtlListViewIcon* large_icon = g_sequence_get(iter);

        g_sequence_append(seq, large_icon);
    }

    g_sequence_free(list_view->seq_icons);
    list_view->seq_icons = seq;
}
