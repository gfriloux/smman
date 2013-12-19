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
