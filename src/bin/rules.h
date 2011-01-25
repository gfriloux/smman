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
 * @file rules.h
 * @brief Contains all includes and prototypes for rules.c
 * @author Guillaume Friloux <guillaume.friloux@asp64.com>
 * @version 1.0
 *
 * Contains all includes and prototypes for rules.c
 */

#include <dirent.h>
#include <errno.h>
#include <Eina.h>
#include <fnmatch.h>
#include <sys/types.h>
#include <regex.h>

#include "libconf.h"
#include "global.h"
#include "logmessages.h"

struct rule *rules_temp;		/**< Pointer to a rule, used so rules_load_rule_loadspec() knowns which rule rules_load_rule() is processing                                */

int rules_load(void);
int rules_load_rule(char *rule_name);
int rules_load_rule_loadspec(char *variable, char *value);
int rules_print(void);
int rules_list(int (*callback)(struct rule *foundrule));
int rules_filtermessage(struct logmessage *new_logmessage);

