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

static const Ecore_Getopt optdesc = {
   "smman",
   "%prog",
   PACKAGE_VERSION,
   "(C) Guillaume Friloux <kuri@efl.so>",
   "GPL v2.",
   "  _______                                   \n"
   " |   _   |.--------..--------..---.-..-----.\n"
   " |   1___||        ||        ||  _  ||     |\n"
   " |____   ||__|__|__||__|__|__||___._||__|__|\n"
   " |:  1   |                                  \n"
   " |::.. . |                                  \n"
   " `-------'                                  \n",
   0,
   {
      ECORE_GETOPT_STORE_TRUE('d', "debug", "Runs smman in debug mode."),
      ECORE_GETOPT_LICENSE('L', "license"),
      ECORE_GETOPT_COPYRIGHT('C', "copyright"),
      ECORE_GETOPT_VERSION('V', "version"),
      ECORE_GETOPT_HELP('h', "help"),
      ECORE_GETOPT_SENTINEL
   }
}; /* ASCII ART : http://patorjk.com/software/taag/ - Font Cricket */


Smman *
init(void)
{
   Smman *smman;

   smman_log_dom_global = eina_log_domain_register("smman", EINA_COLOR_CYAN);
   if (smman_log_dom_global < 0)
     {
        EINA_LOG_ERR("Smman can not create a general log domain");
        return NULL;
     }

   smman = calloc(1, sizeof(Smman));
   if (!smman)
     {
        ERR("Failed to allocate smman structure");
        return NULL;
     }

   smman->rules = rules_new("/etc/smman/rules.d/");
   if (!smman->rules)
     {
        ERR("Failed to allocate new Rules structure");
        return NULL;
     }

   smman->spy = spy_new();
   if (!smman->spy)
     {
        ERR("Failed to create new spy");
        return NULL;
     }

   smman->ev.sl = ecore_event_handler_add(SPY_EVENT_LINE,log_line_event, smman);
   return smman;
}

int main(int argc, char **argv)
{
   Smman *smman;
   Eina_Bool opt_quit = EINA_FALSE,
             opt_debug = EINA_FALSE;
   int opt_ind;

   eina_init();
   ecore_init();

   Ecore_Getopt_Value values[] = {
     ECORE_GETOPT_VALUE_BOOL(opt_debug),
     ECORE_GETOPT_VALUE_BOOL(opt_quit),
     ECORE_GETOPT_VALUE_BOOL(opt_quit),
     ECORE_GETOPT_VALUE_BOOL(opt_quit),
     ECORE_GETOPT_VALUE_BOOL(opt_quit),
     ECORE_GETOPT_VALUE_NONE
   };

   opt_ind = ecore_getopt_parse(&optdesc, values, argc, argv);
   if (opt_ind<0)
     {
        EINA_LOG_ERR("Failed to parse args");
        return 1;
     }

   if (opt_quit)
     return 0;

   conf_init();
   rules_init();
   spy_init();
   store_init();

   smman = init();
   if (!smman)
     return 1;

   conf_load("/etc/smman/smman.conf", config_done, config_error, smman);
   rules_load(smman->rules, filter_load, filter_load_done,
              filter_load_error, smman);

   ecore_main_loop_begin();

   store_shutdown();
   spy_shutdown();
   rules_shutdown();
   conf_shutdown();
   ecore_shutdown();
   eina_log_domain_unregister(smman_log_dom_global);
   smman_log_dom_global = -1;
   eina_shutdown();

   return 0;
}
