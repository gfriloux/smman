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
 * @file main.h
 * @brief Header file for main.c
 * @author Guillaume Friloux <kuri@efl.so>
 * @version 1.0
 *
 * Header file for main.c
 *
 */

#include <stdio.h>
#include <Ecore.h>
#include <Ecore_File.h>
#include <getopt.h>

#include "global.h"

#include "rules.h"
#include "spy.h"
#include "conf.h"

void usage(char *progname);
void version(char *progname);

