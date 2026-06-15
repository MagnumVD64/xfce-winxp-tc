#include <glib.h>
#include <gtk/gtk.h>
#include <wintc/comgtk.h>

#include "application.h"
#include "window.h"

//
// GTK OOP CLASS/INSTANCE DEFINITIONS
//
struct _WinTCListViewTestApplicationClass
{
    GtkApplicationClass __parent__;
};

struct _WinTCListViewTestApplication
{
    GtkApplication __parent__;
};

//
// FORWARD DECLARATIONS
//
static void wintc_list_view_test_application_activate(
    GApplication* application
);

//
// GTK TYPE DEFINITIONS & CTORS
//
G_DEFINE_TYPE(
    WinTCListViewTestApplication,
    wintc_list_view_test_application,
    GTK_TYPE_APPLICATION
)

static void wintc_list_view_test_application_class_init(
    WinTCListViewTestApplicationClass* klass
)
{
    GApplicationClass* application_class = G_APPLICATION_CLASS(klass);

    application_class->activate = wintc_list_view_test_application_activate;
}

static void wintc_list_view_test_application_init(
    WINTC_UNUSED(WinTCListViewTestApplication* self)
) { }

//
// CLASS VIRTUAL METHODS
//
static void wintc_list_view_test_application_activate(
    GApplication* application
)
{
    GtkWidget* new_window =
        wintc_list_view_test_window_new(
            WINTC_LIST_VIEW_TEST_APPLICATION(application)
        );

    gtk_widget_show_all(new_window);
}

//
// PUBLIC FUNCTIONS
//
WinTCListViewTestApplication* wintc_list_view_test_application_new(void)
{
    return WINTC_LIST_VIEW_TEST_APPLICATION(
        g_object_new(
            wintc_list_view_test_application_get_type(),
            "application-id", "uk.co.oddmatics.wintc.play.listview-test",
            NULL
        )
    );
}
