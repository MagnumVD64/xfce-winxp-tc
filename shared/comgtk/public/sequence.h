#ifndef __COMGTK_SEQUENCE_H__
#define __COMGTK_SEQUENCE_H__

#include <glib.h>

//
// PUBLIC FUNCTIONS
//

/**
 * Locates the iterator within a sequence that contains the specified data.
 *
 * @param sequence      The sequence.
 * @param gconstpointer The data to look for.
 * @return The iterator that contains the data, NULL if it was not found.
 */
GSequenceIter* wintc_sequence_find(
    GSequence*    sequence,
    gconstpointer data
);

#endif
