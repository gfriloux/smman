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

/**
 * @file date.c
 * @brief Gets us the current date, like logstash does
 * @author Guillaume Friloux <kuri@efl.so>
 * @version 1.0
 *
 * Gets us the current date, like logstash does
 */

#include "date.h"

/**
 * @fn char * timestamp_XML(char * date)
 * @brief Gets us the current date, like logstash does
 * @bug it doesnt get us date in UTC format, it gets us date on the system
 *
 * @param date (char *) Buffer where we will store the date
 *
 * @return 0
 */
char * timestamp_XML(char * date)
{
	time_t temps;
	struct tm temp;
	temps=time(NULL);
	localtime_r(&temps,&temp);
	sprintf(date,"%04d-%02d-%02dT%02d:%02d:%02d.000000Z",(temp.tm_year)+1900,(temp.tm_mon)+1,temp.tm_mday,temp.tm_hour,temp.tm_min,temp.tm_sec);
	return(date);
}
