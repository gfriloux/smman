#include "rules_private.h"

void
rules_load_rule(void *data,
                Conf *conf)
{
   Rules_Load *rl;
   Rule *rule;
   Eina_Iterator *it;
   const char *file,
              *s;

   rl = data;
   file = conf_file_get(conf);

   DBG("Loaded rule %s", file);

   s = strrchr(file, '/');
   if (!s) s = file;
   else s += 1;

   rule = calloc(1, sizeof(Rule));
   rule->name = strdup(s);

   it =  eina_hash_iterator_tuple_new(conf_variables_get(conf));
   while (eina_iterator_next(it, &data))
     {
        const char *variable,
                   *value;
        Eina_Hash_Tuple *t = data;

        variable = t->key;
        value = t->data;
        if (!strcmp(variable, "filename"))
          rule->spec.filename = strdup(value);
        else if (!strcmp(variable, "tags"))
          rule->spec.tags = strdup(value);
        else if (!strcmp(variable, "source_host"))
          rule->spec.source_host = strdup(value);
        else if (!strcmp(variable, "source_path"))
          rule->spec.source_path = strdup(value);
        else if (!strcmp(variable, "todel"))
          rule->spec.todel = !!atoi(value);

        else if (!strncmp(variable, "message", 7))
        {
           Rule_Regex *rr;
           int r;

           rr = calloc(1, sizeof(Rule_Regex));
           rule->spec.regex = eina_inlist_append(rule->spec.regex,
                                                 EINA_INLIST_GET(rr));

           r = regcomp(&(rr->preg), value, REG_EXTENDED);
           if (r)
             {
                ERR("Failed to compile regex \"%s\", dropping rule.", value);
                eina_iterator_free(it);
                rules_rule_free(rule);
                return;
             }
           rr->regex = strdup(value);

           if (!strstr(variable + 7, "unmatch"))
             rr->must_match = EINA_TRUE;
        }
     }
   eina_iterator_free(it);
   rl->cb.progress((void *)rl->cb.data, rl->rules, rule);
}

void
rules_load_rule_error(void *data,
                      Conf *conf,
                      const char *errstr)
{
   Rules_Load *rl;

   rl = data;

   DBG("Failed to load rule %s", conf_file_get(conf));
   rl->cb.error((void *)rl->cb.data, rl->rules, errstr);
   free(rl);
}

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

   rl = data;
   DBG("rl[%p] Rule file : %s", rl, info->path + info->name_start);

   /* We load file */
   conf_load((char *)info->path,
             rules_load_rule,
             rules_load_rule_error,
             rl);
}

void
rules_load_ls_done(void *data EINA_UNUSED,
                   Eio_File *handler EINA_UNUSED)
{
   DBG("End of rules listing.");
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