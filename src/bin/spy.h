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
 * @file spy.h
 * @brief Contains all includes and prototypes for spy.c
 * @author Guillaume Friloux <guillaume.friloux@asp64.com>
 * @version 1.0
 *
 * Contains all includes and prototypes for spy.c
 */
#include <wordexp.h>

#include "global.h"

#include "rules.h"
#include "logfiles.h"
#include "send.h"
#include "logmessages.h"

int spy_init(void);
int spy_addwatcher(struct rule *foundrule);
int spy_event(void *data, Ecore_File_Monitor *em, Ecore_File_Event event, const char *path);
int spy_extract_new_lines(char *filename, fpos_t pos_cur, fpos_t *pos_new, char *message);

