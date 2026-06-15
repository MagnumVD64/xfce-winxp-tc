/** @file */

#ifndef __COMCTL_LSTVWCTL_H__
#define __COMCTL_LSTVWCTL_H__

#include <glib.h>
#include <gtk/gtk.h>

//
// PUBLIC ENUMS
//
typedef enum _WinTCCtlListViewMode
{
    WINTC_CTL_LIST_VIEW_MODE_LARGE_ICON,
    WINTC_CTL_LIST_VIEW_MODE_DETAILS,
    WINTC_CTL_LIST_VIEW_MODE_SMALL_ICON,
    WINTC_CTL_LIST_VIEW_MODE_LIST,
    WINTC_CTL_LIST_VIEW_MODE_TILE
} WinTCCtlListViewMode;

//
// GTK OOP BOILERPLATE
//
#define WINTC_TYPE_CTL_LIST_VIEW (wintc_ctl_list_view_get_type())

G_DECLARE_FINAL_TYPE(
    WinTCCtlListView,
    wintc_ctl_list_view,
    WINTC,
    CTL_LIST_VIEW,
    GtkWidget
)

//
// PUBLIC FUNCTIONS
//

/**
 * Creates a new instance of WinTCCtlListView.
 *
 * @return The new WinTCCtlListView instance cast to GtkWidget.
 */
GtkWidget* wintc_ctl_list_view_new(void);

#endif
