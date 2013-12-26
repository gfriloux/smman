#define _GNU_SOURCE
#include <stdio.h>

#include "smman.h"

char *
sdupf(const char *s, ...)
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

char *
date_es(void)
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

