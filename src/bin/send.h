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
 * @file send.h
 * @brief Contains all includes and prototypes for send.c
 * @author Guillaume Friloux <guillaume.friloux@asp64.com>
 * @version 1.0
 *
 * Contains all includes and prototypes for send.c
 */

#include <Ecore_Con.h>
#include <curl/curl.h>

#include "global.h"

CURL *curl;
CURLcode res;
unsigned char send_connected;

int send_logmessage(struct logmessage *new_logmessage);
int send_convJSON(char *source_host, char *source_path, char *type, char *message, char *tags, char *timestamp, char **jsondata);
char *send_escape(char *src, char **dst);
int send_toES(char *jsondata);
size_t send_fromES(void *ptr, size_t size, size_t nmemb, void *data);
int send_init(void);
int send_destroy(void);
