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
#include <Conf.h>

/**
 * @brief This function will init libconf and ask to load our configuration
 *  file
 *
 * @return 0
 */
int conf_load(void)
{
	struct libconfig myconf;

	strcpy(global_ESserver, "http://192.168.2.84:9200/logstash/events/");
	strcpy(global_rulesdir, "/etc/smman/rules.d/");
	strcpy(global_conf, "/etc/smman/smman.conf");
	strcpy(global_type, "syslog");
	gethostname(global_host, sizeof(global_host));

	einadom_rules    = eina_log_domain_register("smman_rules"   , EINA_COLOR_LIGHTRED);
	einadom_spy      = eina_log_domain_register("smman_spy"     , EINA_COLOR_LIGHTRED);
	einadom_logfiles = eina_log_domain_register("smman_logfiles", EINA_COLOR_LIGHTRED);
	einadom_send     = eina_log_domain_register("smman_send"    , EINA_COLOR_LIGHTRED);
	einadom_conf     = eina_log_domain_register("smman_conf"    , EINA_COLOR_LIGHTRED);

	libconfig_init(global_conf, &myconf);
	libconfig_load(&myconf);

	libconfig_list(&myconf, conf_load_var);

	libconfig_free(&myconf);
	return(0);
}

/**
 * @brief This function is a callback for libconf, and receives a variable +
 *  its value found in a configuration file
 *
 * @param variable (char *) Variable's name
 * @param value (char *) Variable's value
 *
 * @return 0
 */
int conf_load_var(char *variable, char *value)
{
	if( !strcmp(variable, "server") )
	{
		EINA_LOG_DOM_DBG(einadom_conf, "Setting ES server to %s", variable);
		strcpy(global_ESserver, value);
	}
	else if( !strcmp(variable, "host") )
	{
		EINA_LOG_DOM_DBG(einadom_conf, "Setting default hostname to %s", variable);
		strcpy(global_host, value);
	}
	else if( !strcmp(variable, "type") )
	{
		EINA_LOG_DOM_DBG(einadom_conf, "Setting default type to %s", variable);
		strcpy(global_type, value);
	}
	else
	{
		EINA_LOG_DOM_ERR(einadom_conf, "Unknown config variable %s", variable);
	}

	return(0);
}
