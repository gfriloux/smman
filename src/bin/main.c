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
 * @file main.c
 * @brief Main functions
 * @author Guillaume Friloux <guillaume.friloux@asp64.com>
 * @version 1.0
 *
 * @todo make an exit if receiving SIGUSR1 so we never kill -9 it
 * @todo Index all received messages in an EET file so we dont loose them
 *  if ES is down, if we have a network problem or if we get killed
 *
 * Main functions
 *
 */

/**
 * @mainpage Syslog Message MANager
 *
 * @author Guillaume Friloux <guillaume.friloux@asp64.com>
 *
 * @section INTRODUCTION Introduction
 * SMMan is gateway between syslog files and an <a href=http://www.elasticsearch.com>ElasticSearch</a> database.<br />
 * SMMan has a few more interesting stuff : He can tag every log he sees by using defined rules on them.<br />
 * SMMan uses a configuration file, and needs rules files that must be written by the user of SMMan.<br />
 * SMMan will then use these rules to monitor all the specified logfiles (using inotify), and extract every new entry
 * to filter it using the rules and then indexing it in the configured <a href=http://www.elasticsearch.com>ElasticSearch</a> database.
 * <img src=intro.png>
 *
 * <br />
 * @section CONFIGURATION Configuration
 * The configuration file has to be in <b>/etc/smman/smman.conf</b><br />
 * For now, there is only 3 configurable variables :
 * @li @b server : URL to <a href=http://www.elasticsearch.com>ElasticSearch</a> database. SMMan speaks to <a href=http://www.elasticsearch.com>ElasticSearch</a> using JSON.
 * @li @b host : Allows you to set a different host that the one returned by command hostname (optionnal).
 * @li @b type : Default type for all logs (optionnal).
 *
 *
 * Exemple of configuration file : <br />
 * @code
 * server = http://localhost:9200/logstash/logs/
 * host = BlackStar
 * type = syslog
 * @endcode
 *
 * <br />
 * @section RULES Writing rules
 * Writing rules is quite easy. SMMan search for rules in <b>/etc/smman/rules.d/</b><br />
 * Check the rules directory in the source code to see examples of rules.<br />
 * Basically, rules allows you to write matches about filenames or messages (using globbing/regexp), and set informations like :
 * @li source_host : Set a custom hostname
 * @li type : Set a custom type
 * @li tags : Add tags to the message
 * @li delete : Do not index the log, just drop it
 *
 * <br />
 * @section LOGSTASH Why not using logstash ?
 * @li Its written in ruby and i know nothing to ruby (so i cant modify anything).
 * @li I have been able to make it crash just by deleting a monitored file, or by
 * sending chars like éàè.
 * @li I seem to be too stupid to understand how to automatically tag messages (using Grok, which adds a dependancy seemed complicated to me).
 */
#include "main.h"

/**
 * @fn int main(int argc, char **argv)
 * @brief Main function, will launch all needed functions
 *
 * @param argc args count
 * @param argv args
 *
 * @return 0, exit should not happen
 */
int main(int argc, char **argv)
{
	int c;
	eina_init();
	ecore_init();
	ecore_file_init();
	//send_init();

	send_connected = EINA_FALSE;

	while( 1 )
	{
		int option_index = 0;

		static struct option long_options[] = {
		   {"version", 0, 0, 'v'},
		   {"help", 0, 0, 'h'},
		   {0, 0, 0, 0}
		};

		c = getopt_long(argc, argv, "vh", long_options, &option_index);
		if (c == -1) break;

		switch (c)
		{
			case 'v':
				version(argv[0]);
				exit(0);
			case 'h':
				usage(argv[0]);
				exit(0);
			default:
				break;
		}
	}

	// Read conf
	conf_load();

	// Load rules
	rules_load();

	// We show the list of loaded rules
	// Unecessary
	rules_print();

	// Begin to spy files
	spy_init();

	// We show the list of files we will watch
	// Unecessary
	logfiles_print();

	// We wait for events (new inserts in logfiles)
	ecore_main_loop_begin();

	return(0);
}

/**
 * @fn void usage(char *progname)
 * @brief displays basic usage
 * http://patorjk.com/software/taag/  Cricket font
 *
 * @param progname (char *) Name of program
 */
void usage(char *progname)
{
	printf("  _______                                   \n");
	printf(" |   _   |.--------..--------..---.-..-----.\n");
	printf(" |   1___||        ||        ||  _  ||     |\n");
	printf(" |____   ||__|__|__||__|__|__||___._||__|__|\n");
	printf(" |:  1   |                                  \n");
	printf(" |::.. . |    Usage for %s :                \n", progname);
	printf(" `-------'                                  \n");
	printf("\t--help\t\t-h :\tShow this help screen\n");
	printf("\t--version\t-v :\tShow revision version\n");
	printf("\n");
	printf("\tDebugging : \n");
	printf("\t\tEINA_LOG_LEVEL=5 %s\n", progname);
}

/**
 * @fn void version(char *progname)
 * @brief Shows program revision
 *
 * @param progname (char *) Name of program
 */
void version(char *progname)
{
	printf("%s r%s\n", progname, REVISION);
}
