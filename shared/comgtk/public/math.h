/** @file */

#ifndef __COMGTK_MATH_H__
#define __COMGTK_MATH_H__

#include <gdk/gdk.h>
#include <glib.h>

//
// PUBLIC FUNCTIONS
//

/**
 * Determines if the point is in the specified rectangle.
 *
 * @param point The point.
 * @param rect  The rectangle.
 * @return True if the point is within the rectangle.
 */
gboolean wintc_point_in_rect(
    GdkPoint*     point,
    GdkRectangle* rect
);

/**
 * Determines if the point is in the specified rectangle.
 *
 * @param x    The x-coordinate of the point.
 * @param y    The y-coordinate of the point.
 * @param rect The rectangle.
 * @return True if the point is within the rectangle.
 */
gboolean wintc_point_xy_in_rect(
    gint          x,
    gint          y,
    GdkRectangle* rect
);

#endif
