#include "smman.h"
#include <glob.h>

static void
_filter_rules_free(void *data)
{
   rules_rule_free((Rule *)data);
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
       "source_path[%s]\n\ttags[%s]\n\ttodel[%s]",
       rule->name, rule->spec.filename, rule->spec.source_host,
       rule->spec.source_path, rule->spec.tags,
       (rule->spec.todel) ? "EINA_TRUE" : "EINA_FALSE");

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
}

void
filter_load_done(void *data,
                 Rules *rules)
{
   Smman *smman;

   smman = data;
   if (smman->rules != rules)
     return;

   DBG("smman[%p] rules[%p]", smman, rules);
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
