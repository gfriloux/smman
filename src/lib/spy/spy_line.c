#include "spy_private.h"

/**
 * @addtogroup Lib-Spy-Functions
 * @{
 */

/**
 * @brief Returns the line parsed by spy.
 * @param sl Spy_Line structure.
 * @return Pointer to parsed line.
 *
 * The pointer returned is the internal pointer of the Spy_Line,
 * so, do not free it!
 */
const char *
spy_line_get(Spy_Line *sl)
{
   return sl->line;
}

/**
 * @brief Return the Spy_File of a Spy_Line.
 * @param sl Spy_Line structure.
 * @return Pointer to the Spy_File structure.
 */
Spy_File *
spy_line_spyfile_get(Spy_Line *sl)
{
   return sl->sf;
}

/**
 * @}
 */
