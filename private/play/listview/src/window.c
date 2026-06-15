#include <glib.h>
#include <gtk/gtk.h>
#include <wintc/comctl.h>
#include <wintc/comgtk.h>

#include "application.h"
#include "window.h"

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
) {}

static void wintc_list_view_test_window_init(
    WinTCListViewTestWindow* self
)
{
    gtk_window_set_default_size(
        GTK_WINDOW(self),
        320,
        200
    );

    gtk_container_add(
        GTK_CONTAINER(self),
        wintc_ctl_list_view_new()
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
