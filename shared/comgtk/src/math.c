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

gboolean wintc_rectangle_intersect(
    const GdkRectangle* src1,
    const GdkRectangle* src2,
    GdkRectangle*       dest
)
{
    GdkRectangle normal1;
    GdkRectangle normal2;

    normal1 = *src1;
    normal2 = *src2;

    wintc_rectangle_normalize(&normal1);
    wintc_rectangle_normalize(&normal2);

    return gdk_rectangle_intersect(&normal1, &normal2, dest);
}

void wintc_rectangle_normalize(
    GdkRectangle* rect
)
{
    if (rect->width < 0)
    {
        rect->x     = rect->x + rect->width;
        rect->width = -rect->width;
    }

    if (rect->height < 0)
    {
        rect->y      = rect->y + rect->height;
        rect->height = -rect->height;
    }
}
