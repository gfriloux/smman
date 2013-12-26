#include "smman.h"

void
config_done(void *data,
            Conf *conf)
{
   Smman *smman;
   Eina_Iterator *it;

   smman = data;

   it = eina_hash_iterator_tuple_new(conf_variables_get(conf));
   while (eina_iterator_next(it, &data))
     {
        const char *variable,
                   *value;
        Eina_Hash_Tuple *t = data;

        variable = t->key;
        value = t->data;

        if (!strcmp("server", variable))
          smman->cfg.server = strdup(value);
        else if (!strcmp("host", variable))
          smman->cfg.host = strdup(value);
     }

   DBG("Server = %s", smman->cfg.server);
   DBG("Host = %s", smman->cfg.host);
   eina_iterator_free(it);

   smman->store = store_new(smman->cfg.server);
}

void
config_error(void *data EINA_UNUSED,
             Conf *conf EINA_UNUSED,
             const char *errstr)
{
   ERR("Failed to load /etc/smman/smman.conf : %s", errstr);
   ecore_main_loop_quit();
}
