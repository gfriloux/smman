/*
 * Copyright © 2011 ASP64 <guillaume.friloux@asp64.com>
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
 * @file spy.c
 * @brief Contains functions that monitors logfiles
 * @author Guillaume Friloux <guillaume.friloux@asp64.com>
 * @version 1.0
 *
 * @bug : In rules, use wildcards only for filenames, not directories
 *
 * Contains functions that monitors logfiles
 */
#include "spy.h"

/**
 * @fn int spy_init(void)
 * @brief Will ask rules_list() to list all rules and tell spy_addwatcher()
 *  of every found rule so we can process it and monitor matching log files
 *
 * @return 0
 */
int spy_init(void)
{
	rules_list(spy_addwatcher);
	return(0);
}

/**
 * @fn int spy_addwatcher(struct rule *foundrule)
 * @brief Gets a rule in param, and will check for every matching
 *  files on the system
 *
 * @param foundrule (struct rule *) Rule found by rules_list()
 *
 * @return 0 if no error, -1 if any error
 */
int spy_addwatcher(struct rule *foundrule)
{
	wordexp_t p;
	int ret;
	unsigned int i;
	char *ptr,
	      tmp[512];

	if( !foundrule->filename )
		return(0);

	// As filenames can contain wildcards, we have to test
	// Globbing
	ret = wordexp(foundrule->filename, &p, 0);
	if( ret )
	{
		EINA_LOG_DOM_ERR(einadom_spy, "l utilisation de wordexp() a echoue, il a renvoye %d", ret);
		return(-1);
	}

	EINA_LOG_DOM_DBG(einadom_spy, "Found %d results for %s", p.we_wordc, foundrule->filename);

	// We now have a list of files (or a unique file) and we want to add them
	// To our file list
	for( i = 0; i < p.we_wordc; i++)
	{
		int exist = 0;

		exist = logfiles_exist(p.we_wordv[i]);

		if( !exist )
		{
			fpos_t pos_tmp;
			struct logfile *new_logfile;
			unsigned long long int filesize = 0;

			// We get our cursor pos
			logfiles_getend(p.we_wordv[i], &pos_tmp);
			logfiles_getsize(p.we_wordv[i], &filesize);
			logfiles_new(&new_logfile, p.we_wordv[i], pos_tmp, filesize);

			logfiles_add(new_logfile);
			efm = ecore_file_monitor_add(p.we_wordv[i],
                       (Ecore_File_Monitor_Cb)spy_event,
                       (void *)new_logfile);
		}
	}
	wordfree(&p);

	// We will check parent directory to see if a new file is created
	// and matches one of the rules when a CREATE event is raised.
	// This part sucks cause we only watch the last directory while
	// The wildcard can englob multiple directories! (and so our code gets totally buggy)
	strncpy(tmp, foundrule->filename, 512);
	ptr = dirname(tmp);
	efm = ecore_file_monitor_add(ptr,
                       (Ecore_File_Monitor_Cb)spy_event,
                       NULL);

	return(0);
}

/**
 * @fn int spy_event(void *data, Ecore_File_Monitor *em, Ecore_File_Event event, const char *path)
 * @brief ecore_main_loop() will call this function for every notification he gets from logfiles
 *  activity.
 *
 * @param data (void *) Data associated to the logfile in spy_addwatcher()
 * @param em (Ecore_File_Monitor *) Not really used here.
 * @param event (Ecore_File_Event) Event type
 * @param path (const char *) Name of the file that raised the event
 * 
 * @return ECORE_CALLBACK_RENEW
 */
int spy_event(void *data, Ecore_File_Monitor *em, Ecore_File_Event event, const char *path)
{
	struct logmessage *new_logmessage;
	char message[4096];

	EINA_LOG_DOM_DBG(einadom_spy, "Event on %s ! %p - %p", path, data, em);
	message[0] = 0;

	if( event == ECORE_FILE_EVENT_MODIFIED )
	{
		struct logfile *log = data;
		fpos_t new_pos;
		unsigned long long int filesize;
		Eina_Counter *counter;
		char *counterresult;
		if( !log )
			return(ECORE_CALLBACK_RENEW);
		counter = eina_counter_new("MessageProcessingTime");
		eina_counter_start(counter);
		// If logfile got trunc, than size is different
		logfiles_getsize((char *)path, &filesize);

		if( filesize < log->filesize )
		{
			// If event is that file has been truncated, then there is
			// nothing to read, we just set cursor to beginning
			logfiles_getbegin((char *)path, &(log->cursor));
			log->filesize = filesize;
			return(ECORE_CALLBACK_RENEW);
		}
		log->filesize = filesize;

		EINA_LOG_DOM_DBG(einadom_spy, "Event on %s - %d !", path, log->cursor);

		// We must take attention to the fact that the file might
		// Have been truncated
		spy_extract_new_lines(log->name, log->cursor, &new_pos, message);
		log->cursor = new_pos;

		// If nothing has been read, then file has been trunc and is empty
		// So we do nothing
		if( message[0] == 0 )
			return(ECORE_CALLBACK_RENEW);

		logmessages_new(&new_logmessage, message, log->name);

		rules_filtermessage(new_logmessage);

		// Now we only have to send this filtered message to ES
		if( !new_logmessage->todel )
			send_logmessage(new_logmessage);

		// Message is sent, we can free our structure
		logmessages_free(&new_logmessage);
		eina_counter_stop(counter, 1);

		counterresult = eina_counter_dump(counter);
		EINA_LOG_DOM_DBG(einadom_spy, "Message processing time :\n%s", counterresult);
		eina_counter_free(counter);
	}
	else if( event == ECORE_FILE_EVENT_DELETED_SELF
	      || event == ECORE_FILE_EVENT_DELETED_DIRECTORY
	      || event == ECORE_FILE_EVENT_DELETED_FILE
	       )
	{
		struct logfile *log = data,
		               *foundlog;
		Eina_List *l;
		EINA_LOG_DOM_DBG(einadom_spy, "Logfile %s has been deleted!", path);

		// If this file was one of the files we used to monitor,
		// We must delete it from our list
		EINA_LIST_FOREACH(list_logfiles, l, foundlog)
		{
			if( !strcmp(foundlog->name, path) )
			{
				list_logfiles = eina_list_remove(list_logfiles, log);
				logfiles_del(&log);
				break;
			}
		}
	}
	else if( event == ECORE_FILE_EVENT_CREATED_FILE
	      || event == ECORE_FILE_EVENT_CREATED_DIRECTORY
	       )
	{
		// A new log file has been made, we have to check if this file already is in our filelog
		// And if a rule affects it.
		// If so, we add it to our monitor
		Eina_List *l;
		struct rule *foundrule = NULL;
		struct logfile *new_logfile = NULL;
		fpos_t pos_tmp;
		int ret,
		    got_it = 0;

		EINA_LOG_DOM_DBG(einadom_spy, "Logfile %s has been created!", path);

		// If we already monitor this file, then we ignore it
		// Which might be a problem btw. This thing shouldnt happen
		EINA_LIST_FOREACH(list_logfiles, l, new_logfile)
		{
			if( !strcmp(new_logfile->name, path) )
				return(ECORE_CALLBACK_RENEW);
				
		}

		EINA_LIST_FOREACH(list_rules, l, foundrule)
		{
			if( foundrule->filename )
			{
				unsigned long long int filesize;
				ret = fnmatch(foundrule->filename, path, FNM_NOESCAPE);
				if( ret )
					continue;

				// We get our cursor pos
				logfiles_getbegin((char *)path, &pos_tmp);
				logfiles_getsize((char *)path, &filesize);
				logfiles_new(&new_logfile, (char *)path, pos_tmp, filesize);

				logfiles_add(new_logfile);
				efm = ecore_file_monitor_add(path,
                       (Ecore_File_Monitor_Cb)spy_event,
                       (void *)new_logfile);

				got_it = 1;
				break;
			}
		}

		if( got_it )
		{
			// We must take attention to the fact that the file might
			// Have been truncated
			spy_extract_new_lines(new_logfile->name, new_logfile->cursor, &new_logfile->cursor, message);

			// If nothing has been read, then file has been trunc and is empty
			// So we do nothing
			if( message[0] == 0 )
				return(ECORE_CALLBACK_RENEW);

			logmessages_new(&new_logmessage, message, new_logfile->name);
			rules_filtermessage(new_logmessage);

			// Now we only have to send this filtered message to ES
			if( !new_logmessage->todel )
				send_logmessage(new_logmessage);

			// Message is sent, we can free our structure
			logmessages_free(&new_logmessage);
		}
	}
	else
	{
		EINA_LOG_DOM_DBG(einadom_spy, "WTF event on %s !", path);

	}
	return(ECORE_CALLBACK_RENEW);
}



/**
 * @fn int spy_extract_new_lines(char *filename, fpos_t pos_cur, fpos_t *pos_new, char *message)
 * @brief Gets the new log message that has been inserted. This function kind of sucks and has to
 *  be improved to really extract all the messages and send them to a callback that will
 *  process them intead of taking only one (which can cause a problem if we don't get
 *  one inotify per message logged).
 *
 * @param filename (char *) File that raised an event and should be read
 * @param pos_cur (fpos_t) current cursor position for this file
 * @param pos_new (fpos_t *) New position of the cursor after extracting the new message
 * @param message (char *) message extracted from the logfile
 * 
 * @return 0 or -1 if there is an error
 */
int spy_extract_new_lines(char *filename, fpos_t pos_cur, fpos_t *pos_new, char *message)
{
	FILE *fp;
	int ret;
	fpos_t pos_begin;

	fp = fopen(filename, "r");
	if( !fp )
	{
		EINA_LOG_DOM_ERR(einadom_spy, "We haven't been able to open %s : %s", filename, strerror(errno));
		return(-1);
	}

	fgetpos(fp, &pos_begin);


	// If we can't set cursor, file has been truncated!
	ret = fsetpos(fp, &pos_cur);
	if( ret )
	{
		fsetpos(fp, &pos_begin);
		*pos_new = pos_begin;
	}
	else *pos_new = pos_cur;

	// This while is only here to avoid empty lines
	while( 1 )
	{
		ret = fscanf(fp,"%4096[^\n]\n", message);

		if( ret == EOF )
			break;

		// Didnt read anything
		if( ret == 0)
		{
			char tmp[2];
			EINA_LOG_DOM_DBG(einadom_spy, "Empty line hack");
			ret = fread(tmp, 1, 1, fp);
			if( ret != 1 )
				EINA_LOG_DOM_DBG(einadom_spy, "WTF?!");
			continue;
		}
		if( ret != 1 )
		{
			fgetpos(fp, pos_new);
			EINA_LOG_DOM_DBG(einadom_spy, "Break! new_pos = %d", *pos_new);
			break;
		}
		else
			fgetpos(fp, pos_new);

		// We have a message to log
	}

	fclose(fp);
	return(0);
}
