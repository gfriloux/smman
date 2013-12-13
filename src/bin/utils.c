/*
 * Copyright © 2013 Guillaume Friloux <kuri@efl.so>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "smman.h"
#include <time.h>
#include <stdio.h>

/**
 * @brief Gets us the current date, in the same format as logstash.
 *
 * @return Pointer to the date's string, or NULL if an error
 *         occured.
 */
char *
utils_date(void)
{
   char *s;
   time_t temps;
   struct tm temp;

   temps = time(NULL);
   if (!localtime_r(&temps,&temp))
     return NULL;

   s = calloc(1, 28);
   EINA_SAFETY_ON_NULL_RETURN_VAL(s, NULL);

   sprintf(s,"%04d-%02d-%02dT%02d:%02d:%02d.000000Z",
           (temp.tm_year)+1900,(temp.tm_mon)+1,temp.tm_mday,
           temp.tm_hour,temp.tm_min,temp.tm_sec);
   return s;
}
