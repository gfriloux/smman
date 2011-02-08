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
 * @file rules.c
 * @brief Functions the manipulates rules
 * @author Guillaume Friloux <guillaume.friloux@asp64.com>
 * @version 1.0
 *
 * @bug : Better parsing of tags (we only escape double quotes right now)
 *
 * Functions the manipulates rules
 *
 */
#include "rules.h"

/**
 * @fn int rules_load(void)
 * @brief List the rules directory and loads every rule in it
 *
 * @return 0 if no error, -1 if can't open rules dir
 */
int rules_load(void)
{
	DIR *fd;
	struct dirent * entree;

	list_rules = NULL;

	// For every rule in rules directory, we will load it
	fd = opendir(global_rulesdir);
	if( !fd )
	{
		EINA_LOG_DOM_ERR(einadom_rules, "Can't open rules directory %s : %s", global_rulesdir, strerror(errno));
		return(-1);
	}
	while((entree = readdir(fd)) != NULL)
	{
		if( !strcmp(entree->d_name, ".") ) continue;
		if( !strcmp(entree->d_name, "..") ) continue;

		EINA_LOG_DOM_DBG(einadom_rules, "Found rule file %s", entree->d_name);

		rules_load_rule(entree->d_name);
	}

	closedir(fd);
	return(0);
}

/**
 * @fn int rules_load_rule(char *rule_name)
 * @brief This function receives the name of a rule file, and ask for libconf
 *  to analyze it
 *
 * @param rule_name (char *) name of the rule (filename)
 *
 * @return 0
 */
int rules_load_rule(char *rule_name)
{
	char full_path[512];
	struct libconfig myconf;

	sprintf(full_path, "%s%s", global_rulesdir, rule_name);

	libconfig_init(full_path, &myconf);
	libconfig_load(&myconf);

	// We have to make a new rule struct
	rules_temp = malloc(sizeof(struct rule));

	rules_temp->name = malloc(sizeof(char) * ( strlen(rule_name) + 1 ) );
	strcpy(rules_temp->name, rule_name);

	// We init default values for our struct
	rules_temp->filename = NULL;
	rules_temp->type = NULL;
	rules_temp->source_host = NULL;
	rules_temp->source_path = NULL;
	rules_temp->tags = NULL;
	rules_temp->todel = 0;

	libconfig_list(&myconf, rules_load_rule_loadspec);

	libconfig_free(&myconf);

	// We insert our new rule in our eina rule list
	list_rules = eina_list_append(list_rules, rules_temp);
	return(0);
}

/**
 * @fn int rules_load_rule_loadspec(char *variable, char *value)
 * @brief This function is a callback used by libconf to send us
 *  every variable+value that libconf found in a rulefile
 *
 * @param variable (char *) variable's name
 * @param value (char *) variable's value
 *
 * @return 0
 */
int rules_load_rule_loadspec(char *variable, char *value)
{
	if( !strcmp(variable, "filename") )
	{
		rules_temp->filename = malloc(sizeof(char) * ( strlen(value) + 1 ));
		strcpy(rules_temp->filename, value);
	}
	else if( ( !strcmp(variable, "message") )
	    || ( !strcmp(variable, "message_match") )
	    || ( !strcmp(variable, "message_unmatch") )
	       )
	{
		struct regex *tmp_regex;
		tmp_regex = malloc(sizeof(struct regex));

		tmp_regex->message = malloc(sizeof(char) * ( strlen(value) + 1 ));
		strcpy(tmp_regex->message, value);

		if( !strcmp(variable, "message_unmatch") )
			tmp_regex->must_match = 0;
		else
			tmp_regex->must_match = 1;

		rules_temp->list_regex = eina_list_append(rules_temp->list_regex, tmp_regex);
	}
	else if( !strcmp(variable, "type") )
	{
		rules_temp->type = malloc(sizeof(char) * ( strlen(value) + 1 ));
		strcpy(rules_temp->type, value);
	}
	else if( !strcmp(variable, "source_host") )
	{
		rules_temp->source_host = malloc(sizeof(char) * ( strlen(value) + 1 ));
		strcpy(rules_temp->source_host, value);
	}
	else if( !strcmp(variable, "source_path") )
	{
		rules_temp->source_path = malloc(sizeof(char) * ( strlen(value) + 1 ));
		strcpy(rules_temp->source_path, value);
	}
	else if( !strcmp(variable, "tags") )
	{
		rules_temp->tags = malloc(sizeof(char) * ( strlen(value) + 1 ));
		strcpy(rules_temp->tags, value);
	}
	else if( !strcmp(variable, "delete") )
	{
		rules_temp->todel = atoi(value);
	}
	else
	{
		EINA_LOG_DOM_ERR(einadom_rules, "Unknown variable name %s", variable);
	}
	return(0);
}


/**
 * @fn int rules_print(void)
 * @brief This function will print to stdout every rules that is in
 *  the rules list.
 *
 * @return 0
 */
int rules_print(void)
{
	Eina_List *l;
	struct rule *foundrule = NULL;
	
	EINA_LIST_FOREACH(list_rules, l, foundrule)
	{
		EINA_LOG_DOM_INFO(einadom_rules,
		                  "\n[%s]\n"
		                  "\tfilename\t= %s\n"
		                  "\ttype\t\t= %s\n"
		                  "\tsource_host\t= %s\n"
		                  "\tsource_path\t= %s\n"
		                  "\ttags\t\t= %s\n"
		                  "\tdelete\t= %d\n\n",
		                  foundrule->name,
		                  foundrule->filename,
		                  foundrule->type,
		                  foundrule->source_host,
		                  foundrule->source_path,
		                  foundrule->tags,
		                  foundrule->todel);
	}
	return(0);
}

/**
 * @fn int rules_list(int (*callback)(struct rule *foundrule))
 * @brief This function will list every rule in the list, to send each
 *  item to the given callback
 *
 * @param callback (int *) Callback that will receive all the items
 *
 * @return 0
 */
int rules_list(int (*callback)(struct rule *foundrule))
{
	Eina_List *l;
	struct rule *foundrule = NULL;
	
	EINA_LIST_FOREACH(list_rules, l, foundrule)
	{
		callback(foundrule);
	}
	return(0);
}


/**
 * @fn int rules_filtermessage(struct logmessage *new_logmessage)
 * @brief This function will get a log message, and apply all the rules
 *  on it before giving it back and filtered
 *
 * @param new_logmessage (struct logmessage *)
 *
 * @return 0
 */
int rules_filtermessage(struct logmessage *new_logmessage)
{
	Eina_List *l, *l2;
	struct rule *foundrule = NULL;
	struct regex *foundregex = NULL;
	int ret,
	    excluded = 0;

	// We check each rules to see what we have to do
	EINA_LIST_FOREACH(list_rules, l, foundrule)
	{
		// If filename is set, we check that our log is affected
		// By this rule's filename
		if( foundrule->filename )
		{
			ret = fnmatch(foundrule->filename, new_logmessage->source_path, FNM_NOESCAPE);
			if( ret )
			{
				EINA_LOG_DOM_DBG(einadom_rules, "Log \"%s\" from \"%s\" is not affected by rule %s (filename exclude)", new_logmessage->message, new_logmessage->source_path, foundrule->name);
				continue;
			}
		}

		// Now we check for message filtering
		EINA_LIST_FOREACH(foundrule->list_regex, l2, foundregex)
		{
			regex_t preg;
			size_t nmatch = 2;                                                      
			regmatch_t pmatch[2];

			ret = regcomp(&preg, foundregex->message, REG_EXTENDED);
			if( ret )
			{
				EINA_LOG_DOM_ERR(einadom_rules, "Regcomp failed to compile regexp %s", foundregex->message);
				regfree(&preg);
				continue;
			}

			ret = regexec(&preg, new_logmessage->message,nmatch, pmatch, 0);
			if( ret == foundregex->must_match )
			{
				EINA_LOG_DOM_INFO(einadom_rules, "Log \"%s\" from \"%s\" is not affected by rule %s (message exclude : %s / %d / %d)", new_logmessage->message, new_logmessage->source_path, foundrule->name, foundregex->message, foundregex->must_match, ret);
				regfree(&preg);
				excluded = 1;
				break;
			}
			regfree(&preg);
		}

		if( excluded )
			continue;

		// If we get here, then our log message have to be filtered

		// If type is specified, then we have to override it
		if( foundrule->type )
			logmessages_set_type(new_logmessage, foundrule->type);

		if( foundrule->source_host )
			logmessages_set_sourcehost(new_logmessage, foundrule->source_host);

		if( foundrule->tags )
			logmessages_add_tag(new_logmessage, foundrule->tags);

		if( foundrule->todel )
			logmessages_set_todel(new_logmessage, 1);
	}
	return(0);
}
