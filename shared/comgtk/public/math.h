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

/**
 * Determines if rectangles intersect, outputting the intersection rectangle
 * if requested. Unlike gdk_rectangle_intersect, this handles negative sizes.
 *
 * @param src1 The first rectangle.
 * @param src2 The second rectangle.
 * @param dest (Optional) The output location for the intersection.
 * @return True if the rectangles intersect.
 */
gboolean wintc_rectangle_intersect(
    const GdkRectangle* src1,
    const GdkRectangle* src2,
    GdkRectangle*       dest
);

/**
 * Ensures a rectangle is always top-left origin with positive size.
 *
 * @param rect The rectangle.
 */
void wintc_rectangle_normalize(
    GdkRectangle* rect
);

#endif
