#define _GNU_SOURCE
#include <stdio.h>

#include "store_private.h"


/**
 * @addtogroup Lib-Store-Functions
 * @{
 */

/**
 * @cond IGNORE
 */


/**
 * @brief Creates a new string from a format string.
 * @param s the NULL-terminated format string to retrieve an instance of.
 * @return A pointer to the allocated buffer. NULL on failure.
 */
char *
store_utils_dupf(const char *s, ...)
{
   va_list args;
   int len;
   char *str;

   va_start(args, s);
   len = vasprintf(&str, s, args);
   va_end(args);

   if (len == -1)
     return NULL;

   return str;
}

/**
 * @endcond
 */

/**
 * @}
 */
