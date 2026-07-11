#include <glib.h>
#include <gtk/gtk.h>
#include <wintc/comctl.h>
#include <wintc/comgtk.h>

#include "application.h"
#include "window.h"

//
// PRIVATE ENUMS
//
enum
{
    TARGET_TEXT
};

//
// FORWARD DECLARATIONS
//
static void on_list_view_drag_data_get(
    GtkWidget*        widget,
    GdkDragContext*   context,
    GtkSelectionData* data,
    guint             info,
    guint             time,
    gpointer          user_data
);
static void on_list_view_drag_data_received(
    GtkWidget*        widget,
    GdkDragContext*   context,
    gint              x,
    gint              y,
    GtkSelectionData* data,
    guint             info,
    guint             time,
    gpointer          user_data
);
static gboolean on_list_view_drag_drop(
    GtkWidget*      widget,
    GdkDragContext* context,
    int             x,
    int             y,
    guint           time,
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

//
// STATIC DATA
//
static GtkTargetEntry S_DRAG_TARGETS[] = {
    {
        "text/plain",
        0,
        TARGET_TEXT
    }
};

static GdkAtom S_ATOM_TEXT_PLAIN;

//
// GTK OOP CLASS/INSTANCE DEFINITIONS
//
struct _WinTCListViewTestWindowClass
{
    GtkApplicationWindowClass __parent__;
};

struct _WinTCListViewTestWindow
{
    GtkApplicationWindow __parent__;
};

//
// GTK TYPE DEFINITION & CTORS
//
G_DEFINE_TYPE(
    WinTCListViewTestWindow,
    wintc_list_view_test_window,
    GTK_TYPE_APPLICATION_WINDOW
)

static void wintc_list_view_test_window_class_init(
    WINTC_UNUSED(WinTCListViewTestWindowClass* klass)
)
{
    S_ATOM_TEXT_PLAIN =
        gdk_atom_intern_static_string("text/plain");
}

static void wintc_list_view_test_window_init(
    WinTCListViewTestWindow* self
)
{
    GtkWidget* list_view = wintc_ctl_list_view_new();

    gtk_window_set_default_size(
        GTK_WINDOW(self),
        320,
        200
    );

    gtk_container_add(
        GTK_CONTAINER(self),
        list_view
    );

    wintc_ctl_list_view_enable_drag_dest(
        WINTC_CTL_LIST_VIEW(list_view),
        S_DRAG_TARGETS,
        G_N_ELEMENTS(S_DRAG_TARGETS),
        GDK_ACTION_COPY
    );
    wintc_ctl_list_view_enable_drag_source(
        WINTC_CTL_LIST_VIEW(list_view),
        S_DRAG_TARGETS,
        G_N_ELEMENTS(S_DRAG_TARGETS),
        GDK_ACTION_COPY
    );

    g_signal_connect(
        list_view,
        "drag-data-get",
        G_CALLBACK(on_list_view_drag_data_get),
        NULL
    );
    g_signal_connect(
        list_view,
        "drag-data-received",
        G_CALLBACK(on_list_view_drag_data_received),
        NULL
    );
    g_signal_connect(
        list_view,
        "drag-drop",
        G_CALLBACK(on_list_view_drag_drop),
        NULL
    );
    g_signal_connect(
        list_view,
        "drag-motion",
        G_CALLBACK(on_list_view_drag_motion),
        NULL
    );
}

//
// PUBLIC FUNCTIONS
//
GtkWidget* wintc_list_view_test_window_new(
    WinTCListViewTestApplication* app
)
{
    return GTK_WIDGET(
        g_object_new(
            WINTC_TYPE_LIST_VIEW_TEST_WINDOW,
            "application", GTK_APPLICATION(app),
            "title",       "List View Test",
            NULL
        )
    );
}

//
// CALLBACKS
//
static void on_list_view_drag_data_get(
    WINTC_UNUSED(GtkWidget* widget),
    WINTC_UNUSED(GdkDragContext* context),
    GtkSelectionData* data,
    WINTC_UNUSED(guint info),
    WINTC_UNUSED(guint time),
    WINTC_UNUSED(gpointer user_data)
)
{
    gtk_selection_data_set_text(
        data,
        "The drag worked!",
        -1
    );
}

static void on_list_view_drag_data_received(
    WINTC_UNUSED(GtkWidget* widget),
    GdkDragContext*   context,
    WINTC_UNUSED(gint x),
    WINTC_UNUSED(gint y),
    GtkSelectionData* data,
    WINTC_UNUSED(guint info),
    guint             time,
    WINTC_UNUSED(gpointer user_data)
)
{
    gchar* str = (gchar*) gtk_selection_data_get_text(data);

    WINTC_LOG_DEBUG("%s", str);

    g_free(str);

    gtk_drag_finish(
        context,
        TRUE,
        FALSE,
        time
    );
}

static gboolean on_list_view_drag_drop(
    GtkWidget*      widget,
    GdkDragContext* context,
    WINTC_UNUSED(int x),
    WINTC_UNUSED(int y),
    guint           time,
    WINTC_UNUSED(gpointer user_data)
)
{
    gtk_drag_get_data(
        widget,
        context,
        S_ATOM_TEXT_PLAIN,
        time
    );

    return TRUE;
}

static gboolean on_list_view_drag_motion(
    WINTC_UNUSED(GtkWidget* widget),
    GdkDragContext* context,
    WINTC_UNUSED(int x),
    WINTC_UNUSED(int y),
    guint           time,
    WINTC_UNUSED(gpointer user_data)
)
{
    gdk_drag_status(
        context,
        GDK_ACTION_COPY,
        time
    );

    return TRUE;
}
