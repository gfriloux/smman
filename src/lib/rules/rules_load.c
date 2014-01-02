#include "rules_private.h"

/**
 * @addtogroup Lib-Rules-Functions
 * @{
 */

/**
 * @cond IGNORE
 */

/**
 * @brief Create a Rule structure from tuple given by @ref Lib-Conf
 *
 * @param data Rules_Load structure.
 * @param conf Conf structure.
 *
 * This function is called by conf_load() when the loading of the rule
 * is over.<br />
 * Once the rule is allocated, it is added to the rules's list and
 * given to the progress callback.
 */
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
          {
             char **tags = eina_str_split(value, ",", 0);
             int i;

             for (i = 0; tags[i]; i++)
               {
                  Eina_Strbuf *buf = eina_strbuf_new();

                  eina_strbuf_append(buf, tags[i]);
                  eina_strbuf_trim(buf);
                  rule->spec.tags = eina_list_append(rule->spec.tags,
                                                 eina_strbuf_string_steal(buf));
                  eina_strbuf_free(buf);
               }
             free(tags[0]);
             free(tags);
          }
        else if (!strcmp(variable, "source_host"))
          rule->spec.source_host = strdup(value);
        else if (!strcmp(variable, "source_path"))
          rule->spec.source_path = strdup(value);
        else if (!strcmp(variable, "delete"))
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
   rl->rules->rules = eina_inlist_append(rl->rules->rules,
                                         EINA_INLIST_GET(rule));
}

/**
 * @brief Reports an error to the error Callback defined by app.
 *
 * @param data Rules_Load structure.
 * @param conf Conf structure.
 * @param errstr Error string given by @ref Lib-Conf
 *
 * This function is called by conf_load() if it failed to read and parse
 * a configuration file.
 */
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

/**
 * @brief Filter called from eio's thread, we filter directories.
 *
 * @param data UNUSED
 * @param handler UNUSED
 * @param info Eina_File_Direct_Info structure of file.
 *
 * @return EINA_FALSE if passed file is a directory, EINA_TRUE otherwise.
 */
Eina_Bool
rules_load_ls_filter(void *data EINA_UNUSED,
                     Eio_File *handler EINA_UNUSED,
                     const Eina_File_Direct_Info *info)
{
   if (info->type == EINA_FILE_DIR)
     return EINA_FALSE;
   return EINA_TRUE;
}

/**
 * @brief Load one found rule.
 *
 * @param data Rules_Load structure.
 * @param handler UNUSED.
 * @param info Eina_File_Direct_Info structure of file.
 *
 * This function is called by eio_file_direct_ls() for every file listed.
 */
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

/**
 * @brief Reports the end of rules loading.
 *
 * @param data Rules_Load structure.
 * @param handler UNUSED.
 *
 * This function gets called by eio_file_direct_ls() (from rules_load()) when
 * listing of files is over.
 */
void
rules_load_ls_done(void *data,
                   Eio_File *handler EINA_UNUSED)
{
   Rules_Load *rl = data;
   DBG("End of rules listing.");
   rl->cb.done((void *)rl->cb.data, rl->rules);
   free(rl);
}

/**
 * @brief Reports an error to the error Callback defined by app.
 *
 * @param data Rules_Load structure.
 * @param handler UNUSED.
 * @param error errno from eio.
 *
 * This function gets called by eio_file_direct_ls() when an error occurs
 * while listing files.
 */
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

/**
 * @endcond
 */

/**
 * @}
 */
