#include "rules_private.h"

Eina_Bool
rules_load_ls_filter(void *data EINA_UNUSED,
                     Eio_File *handler EINA_UNUSED,
                     const Eina_File_Direct_Info *info)
{
   if (info->type == EINA_FILE_DIR)
     return EINA_FALSE;
   return EINA_TRUE;
}

void
rules_load_ls(void *data,
              Eio_File *handler EINA_UNUSED,
              const Eina_File_Direct_Info *info)
{
   Rules_Load *rl;
   Rule *rule;

   DBG("Rule file : %s", info->path + info->name_start);

   rl = data;

   /* We load file */

   /* If file is OK, we register it */
   rule = calloc(1, sizeof(Rule));
   rule->name = strdup(info->path + info->name_start);

   rl->cb.progress((void *)rl->cb.data, rl->rules, rule);
}

void
rules_load_ls_done(void *data,
                   Eio_File *handler EINA_UNUSED)
{
   Rules_Load *rl;

   DBG("End of rules listing.");
   rl = data;
   rl->cb.done((void *)rl->cb.data, rl->rules);
   free(rl);
}


void
rules_load_ls_error(void *data,
                    Eio_File *handler EINA_UNUSED,
                    int error)
{
   Rules_Load *rl;

   rl = data;

   ERR("Failed to list files in %s : %s",
       rl->rules->directory, strerror(error));
   rl->cb.error((void *)rl->cb.data, rl->rules, "Failed to list rules directory");
   free(rl);
}
