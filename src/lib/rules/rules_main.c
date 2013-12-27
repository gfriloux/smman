#include "rules_private.h"

static int _rules_init_count = 0;
int _rules_log_dom_global = -1;


void
rules_rule_free(Rule *rule)
{
   char *s;
   EINA_SAFETY_ON_NULL_RETURN(rule);

   free((char *)rule->name);
   free((char *)rule->spec.filename);
   free((char *)rule->spec.source_host);
   free((char *)rule->spec.source_path);

   EINA_LIST_FREE(rule->spec.tags, s)
     free(s);

   while (rule->spec.regex)
     {
        Rule_Regex *rr = EINA_INLIST_CONTAINER_GET(rule->spec.regex, Rule_Regex);
        rule->spec.regex = eina_inlist_remove(rule->spec.regex,rule->spec.regex);
        regfree(&(rr->preg));
        free((char *)rr->regex);
        free(rr);
     }
   free(rule);
}

Rules *
rules_new(const char *directory)
{
   Rules *rules;

   rules = calloc(1, sizeof(Rules));
   rules->directory = strdup(directory);
   return rules;
}

void
rules_purge(Rules *rules)
{
   while (rules->rules)
     {
        Rule *rule = EINA_INLIST_CONTAINER_GET(rules->rules, Rule);
        rules->rules = eina_inlist_remove(rules->rules, rules->rules);
        rules_rule_free(rule);
     }
}

Eina_Bool
rules_load(Rules *rules,
           Rules_Progress_Cb progress_cb,
           Rules_Done_Cb done_cb,
           Rules_Error_Cb error_cb,
           void *data)
{
   Rules_Load *rl;

   rl = calloc(1, sizeof(Rules_Load));
   rl->rules = rules;

   rl->cb.progress = progress_cb;
   rl->cb.done = done_cb;
   rl->cb.error = error_cb;
   rl->cb.data = data;

   eio_file_direct_ls(rl->rules->directory,
                      rules_load_ls_filter,
                      rules_load_ls,
                      rules_load_ls_done,
                      rules_load_ls_error,
                      rl);
   return EINA_TRUE;
}

int
rules_init(void)
{
   if (++_rules_init_count != 1)
     return _rules_init_count;

   if (!eina_init())
     {
        fprintf(stderr, "Rules can not initialize Eina\n");
        return --_rules_init_count;
     }

   _rules_log_dom_global = eina_log_domain_register("rules", EINA_COLOR_RED);
   if (_rules_log_dom_global < 0)
     {
        EINA_LOG_ERR("Rules can not create a general log domain");
        goto shutdown_eina;
     }

   if (!ecore_init())
     {
        ERR("Can not initialize Ecore");
        goto unregister_log_domain;
     }

   if (!eio_init())
     {
        ERR("Can not initialize Eio");
        goto shutdown_ecore;
     }

   if (!conf_init())
     {
        ERR("Can not initialize Conf");
        goto shutdown_eio;
     }

   return _rules_init_count;

shutdown_eio:
   eio_shutdown();
shutdown_ecore:
   ecore_shutdown();
unregister_log_domain:
   eina_log_domain_unregister(_rules_log_dom_global);
   _rules_log_dom_global = -1;
shutdown_eina:
   eina_shutdown();
   return --_rules_init_count;
}

int
rules_shutdown(void)
{
   if (_rules_init_count <= 0)
     {
        fprintf(stderr, "Rules init count not greater than 0 in shutdown.");
        return 0;
     }

   if (--_rules_init_count != 0)
     return _rules_init_count;

   conf_shutdown();
   eio_shutdown();
   ecore_shutdown();
   eina_log_domain_unregister(_rules_log_dom_global);
   _rules_log_dom_global = -1;
   eina_shutdown();
   return _rules_init_count;
}
