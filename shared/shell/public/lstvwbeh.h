/** @file */

#ifndef __SHELL_LSTVWBEH_H__
#define __SHELL_LSTVWBEH_H__

#include <glib.h>
#include <gtk/gtk.h>
#include <wintc/comctl.h>

#include "browser.h"

//
// GTK OOP BOILERPLATE
//
#define WINTC_TYPE_SH_LIST_VIEW_BEHAVIOUR (wintc_sh_list_view_behaviour_get_type())

G_DECLARE_FINAL_TYPE(
    WinTCShListViewBehaviour,
    wintc_sh_list_view_behaviour,
    WINTC,
    SH_LIST_VIEW_BEHAVIOUR,
    GObject
)

//
// PUBLIC FUNCTIONS
//
WinTCShListViewBehaviour* wintc_sh_list_view_behaviour_new(
    WinTCCtlListView* list_view,
    WinTCShBrowser*   browser
);

GList* wintc_sh_list_view_behaviour_get_selected_items(
    WinTCShListViewBehaviour* behaviour
);

#endif
