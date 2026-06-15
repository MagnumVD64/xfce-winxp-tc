#ifndef __APPLICATION_H__
#define __APPLICATION_H__

#include <glib.h>
#include <gtk/gtk.h>

//
// GTK OOP BOILERPLATE
//
#define WINTC_TYPE_LIST_VIEW_TEST_APPLICATION (wintc_list_view_test_application_get_type())

G_DECLARE_FINAL_TYPE(
    WinTCListViewTestApplication,
    wintc_list_view_test_application,
    WINTC,
    LIST_VIEW_TEST_APPLICATION,
    GtkApplication
)

//
// PUBLIC FUNCTIONS
//
WinTCListViewTestApplication* wintc_list_view_test_application_new(void);

#endif
