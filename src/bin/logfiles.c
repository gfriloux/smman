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
 * @file logfiles.c
 * @brief Manages log files list
 * @author Guillaume Friloux <kuri@efl.so>
 * @version 1.0
 *
 * Manages log files list
 * @see logfile
 * @see list_logfiles
 */
#include "logfiles.h"

/**
 * @fn int logfiles_exist(char *logfile)
 * @brief Checks in the list of log files we are watching if we
 *  already watch a given logfile
 *
 * @param logfile (char *) Name of the logfile to check
 *
 * @return 0 if not found, 1 if found
 */
int logfiles_exist(char *logfile)
{
	Eina_List *l;
	struct logfile *foundlogfile = NULL;
	EINA_LIST_FOREACH(list_logfiles, l, foundlogfile)
	{
		if( !strcmp(foundlogfile->name, logfile) )
		{
			return(1);
		}
	}

	return(0);
}

/**
 * @fn int logfiles_add(struct logfile *new_logfile)
 * @brief Adds a logfile struct inside the logfiles list
 *
 * @param new_logfile (struct logfile *) structure to add
 *
 * @return 0 if not found, 1 if found
 */
int logfiles_add(struct logfile *new_logfile)
{
	list_logfiles = eina_list_append(list_logfiles, new_logfile);
	return(0);
}

/**
 * @fn int logfiles_print(void)
 * @brief Prints on stdout the list of logfiles. Only usefull for a bit of
 *  monitoring of what is going on
 *
 * @return 0 if not found, 1 if found
 */
int logfiles_print(void)
{
	Eina_List *l;
	struct logfile *foundlogfile = NULL;
	EINA_LIST_FOREACH(list_logfiles, l, foundlogfile)
	{
		EINA_LOG_DOM_INFO(einadom_logfiles, "Name = %s", foundlogfile->name);
	}
	return(0);
}

/**
 * @fn int logfiles_new(struct logfile **new_logfile, char *filename, fpos_t cursor, unsigned long long int filesize)
 * @brief Will alloc a new logfile structure and set given values
 *
 * @param new_logfile (struct logfile *) structure to alloc
 * @param filename (char *) Name of the logfile
 * @param cursor (fpos_t) Position of the cursor
 * @param filesize (unsigned long long int) Size of the file
 *
 * @return 0
 */
int logfiles_new(struct logfile **new_logfile, char *filename, fpos_t cursor, unsigned long long int filesize)
{
	*new_logfile = malloc(sizeof(struct logfile));
	(*new_logfile)->name = malloc(sizeof(char) * ( strlen(filename) + 1 ) );
	strcpy( (*new_logfile)->name, filename);
	(*new_logfile)->cursor = cursor;
	(*new_logfile)->filesize = filesize;

	return(0);
}

/**
 * @fn int logfiles_del(struct logfile **old_logfile)
 * @brief This function will free an allocated logfile structure
 *
 * @param old_logfile (struct logfile **) structure to free
 *
 * @return 0
 */
int logfiles_del(struct logfile **old_logfile)
{
	if( !(*old_logfile) )
		return(0);

	if( (*old_logfile)->name )
		free((*old_logfile)->name);

	free((*old_logfile));
	return(0);
}

/**
 * @fn int logfiles_getend(char *logfile, fpos_t *pos_end)
 * @brief This function will get a cursor to the end of the file
 *
 * @param logfile (char *) logfile involved
 * @param pos_end (fpos_t *) pointer used to store the position
 *
 * @return 0 or -1 if an error occur
 */
int logfiles_getend(char *logfile, fpos_t *pos_end)
{
	FILE *fp;

	// We get our cursor pos
	fp = fopen(logfile, "r");
	if( !fp )
	{
		EINA_LOG_DOM_ERR(einadom_logfiles, "Can't open %s : %s", logfile, strerror(errno));
		return(-1);
	}
	fseeko(fp, 0L, SEEK_END);
	fgetpos(fp, pos_end);

	fclose(fp);
	return(0);
}

/**
 * @fn int logfiles_getbegin(char *logfile, fpos_t *pos_begin)
 * @brief This function will get a cursor to the begin of the file
 *
 * @param logfile (char *) logfile involved
 * @param pos_begin (fpos_t *) pointer used to store the position
 *
 * @return 0 or -1 if an error occur
 */
int logfiles_getbegin(char *logfile, fpos_t *pos_begin)
{
	FILE *fp;

	// We get our cursor pos
	fp = fopen(logfile, "r");
	if( !fp )
	{
		EINA_LOG_DOM_ERR(einadom_logfiles, "Can't open %s : %s", logfile, strerror(errno));
		return(-1);
	}
	fgetpos(fp, pos_begin);

	fclose(fp);
	return(0);
}

/**
 * @fn int logfiles_getsize(char *logfile, unsigned long long int *filesize)
 * @brief This function will get the size of a file
 *
 * @param logfile (char *) logfile involved
 * @param filesize (unsigned long long int) pointer used to store the size
 *
 * @return 0 or -1 if an error occur
 */
int logfiles_getsize(char *logfile, unsigned long long int *filesize)
{
	struct stat64 st_buffer;
	int retour;

	retour = stat64(logfile, &st_buffer);

	if( retour )
	{
		EINA_LOG_DOM_ERR(einadom_logfiles, "Can't get size of %s : %s", logfile, strerror(errno));
		return(-1);
	}

	*filesize = st_buffer.st_size;

	return(0);
}
