#include "smman.h"

void
filter_load(void *data,
            Rules *rules,
            Rule *rule)
{
   Smman *smman;

   smman = data;
   if (smman->rules != rules)
     return;

   DBG("smman[%p] rules[%p] rule[%p][%s]", smman, rules, rule, rule->name);

   DBG("rule : %s\n\tfilename[%s]\n\tsource_host[%s]\n\t"
       "source_path[%s]\n\ttags[%s]\n\ttodel[%s]",
       rule->name, rule->spec.filename, rule->spec.source_host,
       rule->spec.source_path, rule->spec.tags,
       (rule->spec.todel) ? "EINA_TRUE" : "EINA_FALSE");
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
