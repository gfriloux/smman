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

void _usage(char *progname)
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

int main(int argc, char **argv)
{
   int c;
   eina_init();
   ecore_init();
   ecore_file_init();

   send_connected = EINA_FALSE;

   while (1)
     {
        int option_index = 0;

        static struct option long_options[] = {
           {"help", 0, 0, 'h'},
           {0, 0, 0, 0}
        };

        c = getopt_long(argc, argv, "h", long_options, &option_index);
        if (c == -1) break;

        switch (c)
          {
           case 'h':
              _usage(argv[0]);
              exit(0);
           default:
              break;
          }
     }

   conf_load();
   rules_load();
   rules_print();
   spy_init();
   logfiles_print();

   ecore_main_loop_begin();

   return 0;
}
