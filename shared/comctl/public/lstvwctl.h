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
    GtkDrawingArea
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

void wintc_ctl_list_view_enable_drag_dest(
    WinTCCtlListView*     list_view,
    const GtkTargetEntry* targets,
    gint                  n_targets,
    GdkDragAction         actions
);
void wintc_ctl_list_view_enable_drag_source(
    WinTCCtlListView*     list_view,
    const GtkTargetEntry* targets,
    gint                  n_targets,
    GdkDragAction         actions
);

GtkTreePath* wintc_ctl_list_view_get_drop_target(
    WinTCCtlListView* list_view
);
GtkTreeModel* wintc_ctl_list_view_get_model(
    WinTCCtlListView* list_view
);
GtkOrientation wintc_ctl_list_view_get_orientation(
    WinTCCtlListView* list_view
);
gint wintc_ctl_list_view_get_pixbuf_column(
    WinTCCtlListView* list_view
);
gint wintc_ctl_list_view_get_text_column(
    WinTCCtlListView* list_view
);

void wintc_ctl_list_view_set_model(
    WinTCCtlListView* list_view,
    GtkTreeModel*     model
);
void wintc_ctl_list_view_set_orientation(
    WinTCCtlListView* list_view,
    GtkOrientation    orientation
);
void wintc_ctl_list_view_set_pixbuf_column(
    WinTCCtlListView* list_view,
    gint              column
);
void wintc_ctl_list_view_set_text_column(
    WinTCCtlListView* list_view,
    gint              column
);

void wintc_ctl_list_view_unset_drag_dest(
    WinTCCtlListView* list_view
);
void wintc_ctl_list_view_unset_drag_source(
    WinTCCtlListView* list_view
);

#endif
