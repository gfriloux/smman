#include "spy_private.h"

/**
 * @addtogroup Lib-Spy-Functions
 * @{
 */

const char *
spy_line_get(Spy_Line *sl)
{
   return sl->line;
}

Spy_File *
spy_line_spyfile_get(Spy_Line *sl)
{
   return sl->sf;
}

/**
 * @}
 */
