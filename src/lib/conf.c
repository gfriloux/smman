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
 * @file libconf.c
 * @brief Allows reading of a configuration file
 * @author Guillaume Friloux <kuri@efl.so>
 * @version 1.0
 *
 * Allows reading of a configuration file.
 * Every valid line of the configuration will be stored in an
 * Eina list, and can be retrieved using callbacks
 * @todo Being able to specify separation character between variable and value
 *
 */

/**
 * @page tutorial_libconf Libconf Documentation
 * Libconf will allow you to parse basic configuration files of type :
 *
 * variable = value
 *
 *
 * All you need to do is initialise the library, give fullpath to configuration
 * file and read it.
 *
 * Example code :
 * @code
 * #include <libconf.h>
 *
 * int show_conf(char *variable, char *value)
 * {
 * 	printf("Variable=%s\tValue=%s\n", variable, value);
 * 	return(0);
 * }
 *
 * int main(int argc, char **argv)
 * {
 * 	if( argc < 2 ) return(1);
 * 	struct libconfig myconf;
 * 	eina_init();
 * 	libconfig_init(argv[1], &myconf);
 * 	libconfig_load(&myconf);
 * 	libconfig_list(&myconf, show_conf);
 * 	libconfig_free(&myconf);
 * 	return(0);
 * }
 * @endcode
 *
 * @verbatim  gcc -o main main.c `pkg-config --cflags libconf` `pkg-config --libs libconf`  `pkg-config --cflags eina` @endverbatim
 * @verbatim  EINA_LOG_LEVEL=5 ./main /etc/docupack.conf@endverbatim
 */
#include <Conf.h>

/**
 * \brief Inits lib.
 *
 * \param file full path to configuration file
 * \param myconf Pointer to a libconfig structure
 * \return 0 if no error, 1 otherwise
 */
int libconfig_init(char *file, struct libconfig *myconf)
{
	strcpy(myconf->file, file);
	myconf->lconfig = NULL;

	ligconfig_einadom = eina_log_domain_register("Libconf", EINA_COLOR_ORANGE);

	EINA_LOG_DOM_DBG(ligconfig_einadom, "Using configuration file %s", myconf->file);

	return(0);
}


/**
 * \brief Loads configuration file to store every variable into our Eina list
 *
 * \param myconf Pointer to a libconfig structure
 * \return 0
 */
int libconfig_load(struct libconfig *myconf)
{
	FILE *fd;
	char line[1024],
	     *ptr;
	int nb;

	EINA_LOG_DOM_DBG(ligconfig_einadom, "Loading configuration file");

	fd = fopen(myconf->file, "r");
	if( !fd )
	{
		EINA_LOG_DOM_ERR(ligconfig_einadom, "Can't open conf file \"%s\" : %s", myconf->file, strerror(errno));
		return(-1);
	}

	while( 1 )
	{
		struct libconfig_entry *entrytmp;
		int size;

		nb = fscanf(fd, "%1024[^\n]\n", line);

		// Did not got anything
		if( nb != 1 )
 			break;

		// Check if last char is '\r', if so we delete it
		ptr = &line[strlen(line)-1];
		if( ptr[0] == '\r' ) ptr[0] = 0;

		// If this line is a comment, we go to next line
		if( line[0] == '#' ) continue;

		// We check that line has a '='
		ptr = strchr(line, '=');
		if( !ptr )
		{
			EINA_LOG_DOM_INFO(ligconfig_einadom, "Config entry is invalid : %s", line);
			continue;
		}

		ptr[0] = 0;

		ptr = &ptr[1];

		// We delete undesired spaces
		while( ( ptr[0] == 32 ) || ( ptr[0] == 9 ) )
			ptr = &ptr[1];

		size = strlen(ptr);
		while( ( ptr[size-1] == 32 ) || ( ptr[size-1] == 9 ) )
		{
			ptr[size-1] = 0;
			size = strlen(ptr);
		}

		size = strlen(line);
		while( ( line[size-1] == 32 ) || ( line[size-1] == 9 ) )
		{
			line[size-1] = 0;
			size = strlen(line);
		}


		entrytmp = malloc( sizeof(struct libconfig_entry) );

		entrytmp->var = malloc( sizeof(char) * ( strlen(line) + 1 ) );
		strcpy(entrytmp->var, line);

		entrytmp->value = malloc( sizeof(char) * ( strlen(ptr) + 1 ) );
		strcpy(entrytmp->value, ptr);

		EINA_LOG_DOM_DBG(ligconfig_einadom, "Adding %s<->%s", entrytmp->var, entrytmp->value);
		myconf->lconfig = eina_list_prepend(myconf->lconfig, entrytmp);
	}

	fclose(fd);

	return(0);
}

/**
 * \brief Will send to a callback every entry of the Eina list
 *
 * \param myconf Pointer to a libconfig structure
 * \param callback_function Callback to call for each entry
 * \return 0
 */
int libconfig_list(struct libconfig *myconf, int (*callback_function)(char *variable, char *value))
{
	Eina_List *l;
	struct libconfig_entry *myentry;

	EINA_LIST_FOREACH(myconf->lconfig, l, myentry)
		callback_function(myentry->var, myentry->value);

	return(0);
}

/**
 * \brief Will free our eina list
 *
 * \param myconf Pointer to a libconfig structure
 * \return 0
 */
int libconfig_free(struct libconfig *myconf)
{
	Eina_List *l;
	struct libconfig_entry *myentry;

	EINA_LIST_FOREACH(myconf->lconfig, l, myentry)
	{
		free(myentry->var);
		free(myentry->value);
		free(myentry);
	}
	eina_list_free(myconf->lconfig);
	return(0);
}
