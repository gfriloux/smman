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
 * @file conf.h
 * @brief Contains all includes and prototypes for conf.c
 * @author Guillaume Friloux <kuri@efl.so>
 * @version 1.0
 *
 * Contains all includes and prototypes for conf.c
 */
#include <unistd.h>

#include "libconf.h"
#include "global.h"

int conf_load(void);
int conf_load_var(char *variable, char *value);
