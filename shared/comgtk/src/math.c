#include <gdk/gdk.h>
#include <glib.h>

#include "../public/math.h"

//
// PUBLIC FUNCTIONS
//
gboolean wintc_point_in_rect(
    GdkPoint*     point,
    GdkRectangle* rect
)
{
    return wintc_point_xy_in_rect(point->x, point->y, rect);
}

gboolean wintc_point_xy_in_rect(
    gint          x,
    gint          y,
    GdkRectangle* rect
)
{
    return
        x >= rect->x && x < rect->x + rect->width &&
        y >= rect->y && y < rect->y + rect->height;
}
