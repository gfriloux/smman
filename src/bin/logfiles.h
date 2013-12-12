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
 * @file logfiles.h
 * @brief Contains all includes and prototypes for logfiles.c
 * @author Guillaume Friloux <kuri@efl.so>
 * @version 1.0
 *
 * Contains all includes and prototypes for logfiles.c
 */
#include <errno.h>
#include <sys/stat.h>

#include "global.h"

int logfiles_exist(char *logfile);
int logfiles_add(struct logfile *new_logfile);
int logfiles_print(void);
int logfiles_new(struct logfile **new_logfile, char *filename, fpos_t cursor, unsigned long long int filesize);
int logfiles_del(struct logfile **old_logfile);
int logfiles_getend(char *logfile, fpos_t *pos_end);
int logfiles_getbegin(char *logfile, fpos_t *pos_begin);
int logfiles_getsize(char *logfile, unsigned long long int *filesize);

