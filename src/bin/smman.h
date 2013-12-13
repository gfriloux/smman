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

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <Eina.h>
#include <Ecore.h>
#include <Ecore_File.h>
#include <curl/curl.h>

#include <dirent.h>
#include <errno.h>
#include <fnmatch.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <regex.h>
#include <unistd.h>
#include <libgen.h>
#include <getopt.h>

#ifndef GLOBALVARS
#define GLOBALVARS
char global_rulesdir[35], /**< Allows us to store the rules directory          */
     global_conf[35],     /**< Allows us to store the configuration directory  */
     global_host[35],     /**< Allows us to store the default hostname         */
     global_type[35],     /**< Allows us to store the default type             */
     global_ESserver[128];/**< Allows us to store ElasticSearch server adress  */

/**
 * @struct rule
 * @brief This structure can handle a rule described in the rule directory
 */
struct rule
{
   char *name, /**< Rule's name (Guessed using the filename of the rule inside the rules directory) */
        *filename,/**< It is the filename of the logfile(s) that match this rule (see this as a condition) */
        *type,/**< Type we will apply to the matched log message (see this as an affectation) */
        *source_host,/**< Host we will apply to the matched log message (see this as an affectation) */
        *source_path,/**< Log path we will apply to the matched log message (its like renaming the log, without mv it) (see this as an affectation)              */
        *tags;/**< tags we will apply to the matched log message (see this as an affectation) */
   int todel;/**< If set to something other than 0, every matched rule will not be indexed */
   Eina_List *list_regex;/**< Regexps to apply to message we get from syslog, then match them (see this as a condition) */
};

/**
 * @struct regex
 * @brief This structure can handle a regex that will be compared to log messages
 *  in order to see if they are affected by a rule or not
 */
struct regex
{
   char *message;
   int must_match;
   regex_t preg;
};

/**
 * @struct logfile
 * @brief This structure allows us to attach a cursor position to a logfile,
 *  which is necessary to find new log entries
 */
struct logfile
{
   char *name; /**< Name of the log file */
   fpos_t cursor; /**< Position of the cursor inside this logfile */
   unsigned long long int filesize; /**< File size, only help to see if a logfile is truncated, as far as i know */
};

/**
 * @struct logmessage
 * @brief This structure Contains needed information about a logfile that is going
 *  to be JSONed for ES.
 */
struct logmessage
{
   char *source_host, /**< Hostname */
        *source_path, /**< Name of the logfile from where this message comes from */
        *timestamp, /**< Date of the log */
        *type, /**< Type of message */
        *message; /**< Log message to index in ES */
   Eina_List *list_tags; /**< List of tags associated to this message */
   int todel; /**< If set to something other than 0, this message wont be indexed */
};

int einadom_rules, /**< Used for eina_log inside rules.c */
    einadom_spy, /**< Used for eina_log inside spy.c */
    einadom_logfiles, /**< Used for eina_log inside logfiles.c */
    einadom_send, /**< Used for eina_log inside send.c */
    einadom_conf; /**< Used for eina_log inside conf.c */

Eina_List *list_rules; /**< Used to store all the rules in memory, in an eina list */
Eina_List *list_logfiles; /**< Used to store all the logfiles watched, in an eina list */
Ecore_File_Monitor *efm; /**< Used for file monitoring by ecore */

unsigned char send_connected;
#endif

int conf_load(void);
int conf_load_var(char *variable, char *value);

int logfiles_exist(char *logfile);
int logfiles_add(struct logfile *new_logfile);
int logfiles_print(void);
int logfiles_new(struct logfile **new_logfile, char *filename, fpos_t cursor, unsigned long long int filesize);
int logfiles_del(struct logfile **old_logfile);
int logfiles_getend(char *logfile, fpos_t *pos_end);
int logfiles_getbegin(char *logfile, fpos_t *pos_begin);
int logfiles_getsize(char *logfile, unsigned long long int *filesize);

int logmessages_new(struct logmessage **new_logmessage, char *message, char *logname);
int logmessages_free(struct logmessage **old_logmessage);
int logmessages_set_type(struct logmessage *mylog, char *type);
int logmessages_set_sourcehost(struct logmessage *mylog, char *source_host);
int logmessages_add_tag(struct logmessage *mylog, char *tag);
int logmessages_set_todel(struct logmessage *mylog, int value);

int rules_load(void);
int rules_load_rule(char *rule_name);
int rules_load_rule_loadspec(char *variable, char *value);
int rules_print(void);
int rules_list(int (*callback)(struct rule *foundrule));
int rules_filtermessage(struct logmessage *new_logmessage);

int send_logmessage(struct logmessage *new_logmessage);
int send_convJSON(char *source_host, char *source_path, char *type, char *message, char *tags, char *timestamp, char **jsondata);
char *send_escape(char *src, char **dst);
int send_toES(char *jsondata);
size_t send_fromES(void *ptr, size_t size, size_t nmemb, void *data);
int send_init(void);
int send_destroy(void);


int spy_init(void);
int spy_addwatcher(struct rule *foundrule);
int spy_event(void *data, Ecore_File_Monitor *em, Ecore_File_Event event, const char *path);
int spy_extract_new_lines(char *filename, fpos_t pos_cur, fpos_t *pos_new, char *message);

char * utils_date(void);