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
 * @file logmessages.c
 * @brief Contains functions that helps manipulating logmessage structure
 * @author Guillaume Friloux <kuri@efl.so>
 * @version 1.0
 *
 * Contains functions that helps manipulating logmessage structure
 */
#include "logmessages.h"

/**
 * @fn int logmessages_new(struct logmessage **new_logmessage, char *message, char *logname)
 * @brief This function will init a given logmessage structure with default values
 *
 * @param new_logmessage (struct logmessage **) struct to alloc
 * @param message (char *) Log message to store in this structure
 * @param logname (char *) Name of the log file
 *
 * @return 0
 */
int logmessages_new(struct logmessage **new_logmessage, char *message, char *logname)
{
	*new_logmessage = malloc(sizeof(struct logmessage));

	if( message )
	{
		(*new_logmessage)->message = malloc( sizeof(char) * ( strlen(message) + 1) );
		strcpy((*new_logmessage)->message, message);
	}
	else
	{
		(*new_logmessage)->message = malloc( sizeof(char) * (1) );
		(*new_logmessage)->message[0] = 0;
	}
	(*new_logmessage)->source_path = malloc( sizeof(char) * ( strlen(logname) + 1 ) );
	strcpy( (*new_logmessage)->source_path, logname);

	(*new_logmessage)->source_host = malloc( sizeof(char) * ( strlen(global_host) + 1 ) );
	strcpy( (*new_logmessage)->source_host, global_host);

	(*new_logmessage)->type = malloc( sizeof(char) * ( strlen(global_type) + 1 ) );
	strcpy( (*new_logmessage)->type, global_type);

	timestamp_XML( (*new_logmessage)->timestamp);

	(*new_logmessage)->list_tags   = NULL;
	(*new_logmessage)->todel = 0;
	return(0);
}

/**
 * @fn int logmessages_free(struct logmessage **old_logmessage)
 * @brief This function will free everything allocated in the structure
 *
 * @param old_logmessage (struct logmessage **) struct to free
 *
 * @return 0
 */
int logmessages_free(struct logmessage **old_logmessage)
{
	char *ptr;
	if( (*old_logmessage)->source_host )
		free( (*old_logmessage)->source_host);
	free( (*old_logmessage)->source_path);
	free( (*old_logmessage)->type);
	free( (*old_logmessage)->message);

	EINA_LIST_FREE( (*old_logmessage)->list_tags, ptr)
		free(ptr);
	free( (*old_logmessage));
	return(0);
}

/**
 * @fn int logmessages_set_type(struct logmessage *mylog, char *type)
 * @brief This function will set a given type to a given logmessage structure
 *
 * @param mylog (struct logmessage *) Structure to modify
 * @param type (char *) Type to set
 *
 * @return 0
 */
int logmessages_set_type(struct logmessage *mylog, char *type)
{
	if( mylog->type )
		free(mylog->type);

	mylog->type = malloc( sizeof(char) * ( strlen(type) + 1 ) );
	strcpy(mylog->type, type);
	return(0);
}

/**
 * @fn int logmessages_set_sourcehost(struct logmessage *mylog, char *source_host)
 * @brief This function will set a given host to a given logmessage structure
 *
 * @param mylog (struct logmessage *) Structure to modify
 * @param source_host (char *) Host to set
 *
 * @return 0
 */
int logmessages_set_sourcehost(struct logmessage *mylog, char *source_host)
{
	if( mylog->source_host )
		free(mylog->source_host);

	mylog->source_host = malloc( sizeof(char) * ( strlen(source_host) + 1 ) );
	strcpy(mylog->source_host, source_host);
	return(0);
}

/**
 * @fn int logmessages_add_tag(struct logmessage *mylog, char *tag)
 * @brief This function will add tags to the given logmessage structure.
 *  Tags has to be separated by a ","
 *
 * @param mylog (struct logmessage *) Structure to modify
 * @param tag (char *) tag to add
 *
 * @return 0
 */
int logmessages_add_tag(struct logmessage *mylog, char *tag)
{
	char *ptr,
	     *ptr_delim;
	int last = 0;
	ptr = &tag[0];

	while( 1 )
	{
		char *new_tag;
		ptr_delim = strchr(ptr, ',');
		if( !ptr_delim )
		{
			ptr_delim = &ptr[strlen(ptr)+1];
			last = 1;
		}
		else ptr_delim[0] = 0;

		new_tag = malloc( sizeof(char) * ( strlen(ptr) + 1 ) );
		strcpy(new_tag, ptr);

		mylog->list_tags = eina_list_append(mylog->list_tags, new_tag);

		if( last ) break;
		ptr_delim[0] = ',';
		ptr = &ptr_delim[1];
	}

	return(0);
}

/**
 * @fn int logmessages_set_todel(struct logmessage *mylog, int value)
 * @brief This function will set a given value to the delete field
 *
 * @param mylog (struct logmessage *) Structure to modify
 * @param value (int) value to set
 *
 * @return 0
 */
int logmessages_set_todel(struct logmessage *mylog, int value)
{
	mylog->todel = value;
	return(0);
}
