#include "smman.h"
#include <glob.h>

static void
_filter_rules_free(void *data EINA_UNUSED)
{
}

Eina_Bool
filter_reload(void *data,
              int type EINA_UNUSED,
              void *ev EINA_UNUSED)
{
   Smman *smman = data;
   Filter *filter;

   DBG("smman[%p]", smman);

   EINA_INLIST_FOREACH(smman->filters, filter)
     {
        spy_file_pause(filter->sf);
        eina_hash_free(filter->rules);
        filter->rules = eina_hash_string_superfast_new(_filter_rules_free);
     }

   rules_purge(smman->rules);

   rules_load(smman->rules, filter_load, filter_load_done,
              filter_load_error, smman);
   return EINA_TRUE;
}

Filter *
_filter_find(Smman *smman,
             const char *filename)
{
   Filter *filter;

   EINA_INLIST_FOREACH(smman->filters, filter)
     {
        if (!strcmp(filter->filename, filename))
          return filter;
     }
   return NULL;
}

void
filter_load(void *data,
            Rules *rules,
            Rule *rule)
{
   Smman *smman;
   Filter *filter;
   int r;
   glob_t files;
   char **s;
   size_t i;

   smman = data;
   if (smman->rules != rules)
     return;

   DBG("smman[%p] rules[%p] rule[%p][%s]", smman, rules, rule, rule->name);

   DBG("rule : %s\n\tfilename[%s]\n\tsource_host[%s]\n\t"
       "source_path[%s]\ntodel[%s]",
       rule->name, rule->spec.filename, rule->spec.source_host,
       rule->spec.source_path, (rule->spec.todel) ? "EINA_TRUE" : "EINA_FALSE");

   /* Loop with globbing to write here! */
   r = glob(rule->spec.filename, GLOB_MARK, 0, &files);
   if (r)
     {
        ERR("Unable to find files matching \"%s\"", rule->spec.filename);
        return;
     }

   for (s = files.gl_pathv, i = files.gl_pathc; i; s++, i--)
     {
        DBG("Corresponding file %s", *s);

        /* We have to check the files affected by this rule */
        filter = _filter_find(smman,*s);
        if (filter)
          goto add_rule;

        DBG("No filter found for %s, creating new one", *s);
        filter = calloc(1, sizeof(Filter));
        filter->sf = spy_file_new(smman->spy, *s);
        if (!filter->sf)
          {
             ERR("Failed to put spy on %s", *s);
             free(filter);
             continue;
          }

        spy_file_data_set(filter->sf, filter);
        filter->filename = strdup(*s);
        filter->rules = eina_hash_string_superfast_new(_filter_rules_free);
        smman->filters = eina_inlist_append(smman->filters,
                                            EINA_INLIST_GET(filter));

        add_rule:
        DBG("Adding rule[%p][%s] to filter[%p][%s]",
            rule, rule->name, filter, filter->filename);
        eina_hash_add(filter->rules, rule->name, rule);
     }
   globfree(&files);
}

void
filter_load_done(void *data,
                 Rules *rules)
{
   Smman *smman;
   Filter *filter;
   Eina_Inlist *l;

   smman = data;
   if (smman->rules != rules)
     return;

   DBG("smman[%p] rules[%p]", smman, rules);

   EINA_INLIST_FOREACH_SAFE(smman->filters, l, filter)
     {
        if (eina_hash_population(filter->rules))
          {
             DBG("Resuming sf[%p]", filter->sf);
             spy_file_resume(filter->sf);
             continue;
          }

        DBG("Freeing filter %s", filter->filename);
        smman->filters = eina_inlist_remove(smman->filters, smman->filters);
        spy_file_free(filter->sf);
        free((char *)filter->filename);
        eina_hash_free(filter->rules);
        free(filter);
     }
}

void
filter_load_error(void *data,
                  Rules *rules,
                  const char *errstr)
{
   Smman *smman;

   smman = data;

   if (smman->rules != rules)
     return;

   ERR("Failed to load rules : %s", errstr);
   ecore_main_loop_quit();
}
