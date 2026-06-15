#ifndef __WINDOW_H__
#define __WINDOW_H__

#include <glib.h>
#include <gtk/gtk.h>

#include "application.h"

//
// GTK OOP BOILERPLATE
//
#define WINTC_TYPE_LIST_VIEW_TEST_WINDOW (wintc_list_view_test_window_get_type())

G_DECLARE_FINAL_TYPE(
    WinTCListViewTestWindow,
    wintc_list_view_test_window,
    WINTC,
    LIST_VIEW_TEST_WINDOW,
    GtkApplicationWindow
)

//
// PUBLIC FUNCTIONS
//
GtkWidget* wintc_list_view_test_window_new(
    WinTCListViewTestApplication* app
);

#endif
