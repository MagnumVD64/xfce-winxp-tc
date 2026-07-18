#include <glib.h>

#include "../public/sequence.h"

//
// PUBLIC FUNCTIONS
//
GSequenceIter* wintc_sequence_find(
    GSequence*    sequence,
    gconstpointer data
)
{
    GSequenceIter* iter = g_sequence_get_begin_iter(sequence);

    while (!g_sequence_iter_is_end(iter))
    {
        if (g_sequence_get(iter) == data)
        {
            return iter;
        }

        iter = g_sequence_iter_next(iter);
    }

    return NULL;
}
