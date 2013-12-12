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
 * @file libconf.h
 * @brief Contains structs and prototypes of libconf
 * @author Guillaume Friloux <kuri@efl.so>
 * @version 1.0
 *
 * Contains structs and prototypes of libconf
 *
 */

#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <Eina.h>

#ifndef LIBCONFVARS
#define LIBCONFVARS
/**
 * \struct libconfig
 * \brief Main structure
 */
struct libconfig
{
	char file[512];				/**< Name of config file */
	Eina_List *lconfig;			/**< List of variables */
};

/**
 * \struct libconfig_entry
 * \brief Structure containing information about one config entry
 */
struct libconfig_entry
{
	char *var,					/**< Variable name */
	     *value;				/**< Value for this variable */
};

int ligconfig_einadom;			/**< Eina DOM to use for eina logs */
#endif

int libconfig_init(char *file, struct libconfig *myconf);
int libconfig_load(struct libconfig *myconf);
int libconfig_list(struct libconfig *myconf, int (*callback_function)(char *variable, char *value));
int libconfig_free(struct libconfig *myconf);
void libconfig_version(char *version);
