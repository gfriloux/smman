#include "conf_private.h"

static int _conf_init_count = 0;
int _conf_log_dom_global = -1;

Eina_Hash *
conf_variables_get(Conf *conf)
{
   return conf->variables;
}

void
conf_free(Conf *conf)
{
   EINA_SAFETY_ON_NULL_RETURN(conf);

   if (conf->ef)
     {
        eina_file_close(conf->ef);
        conf->ef = NULL;
     }

   if (conf->file)
     {
        free((char *)conf->file);
        conf->file = NULL;
     }

   free(conf);
}

Eina_Bool
conf_load(char *file,
          Conf_Done_Cb done_cb,
          Conf_Error_Cb error_cb,
          const void *data)
{
   Conf *conf;
   Conf_Load *cl;

   conf = calloc(1, sizeof(Conf));
   if (!conf)
     {
        ERR("Failed to allocate Conf structure");
        return EINA_FALSE;
     }

   conf->ef = eina_file_open(file, EINA_FALSE);
   if (!conf->ef)
     {
        ERR("Failed to open file %s : %s", file, strerror(errno));
        goto free_conf;
     }

   conf->file = strdup(file);
   if (!conf->file)
     {
        ERR("Failed to allocate buffer");
        goto close_file;
     }

   cl = calloc(1, sizeof(Conf_Load));
   if (!cl)
     {
        ERR("Failed to allocate Conf_Load structure");
        goto free_file;
     }

   cl->cb.done = done_cb;
   cl->cb.error = error_cb;
   cl->cb.data = data;
   cl->conf = conf;

   conf->variables = eina_hash_string_superfast_new(conf_load_line_free);

   eio_file_map_all(conf->ef,
                    EINA_FILE_SEQUENTIAL,
                    conf_load_map_filter,
                    conf_load_map_main,
                    conf_load_map_error,
                    cl);

   return EINA_TRUE;

free_file:
   free((char *)conf->file);
close_file:
   eina_file_close(conf->ef);
   conf->ef = NULL;
free_conf:
   free(conf);
   return EINA_FALSE;
}

int conf_init(void)
{
   if (++_conf_init_count != 1)
     return _conf_init_count;

   if (!eina_init())
     {
        fprintf(stderr, "Conf can not initialize Eina\n");
        return --_conf_init_count;
     }

   _conf_log_dom_global = eina_log_domain_register("conf", EINA_COLOR_RED);
   if (_conf_log_dom_global < 0)
     {
        EINA_LOG_ERR("Conf can not create a general log domain");
        goto shutdown_eina;
     }

   if (!ecore_init())
     {
        ERR("Can not initialize Ecore");
        goto unregister_log_domain;
     }

   if (!eio_init())
     {
        ERR("Can not initialize Conf");
        goto shutdown_ecore;
     }

   eina_log_threads_enable();
   return _conf_init_count;

shutdown_ecore:
   ecore_shutdown();
unregister_log_domain:
   eina_log_domain_unregister(_conf_log_dom_global);
   _conf_log_dom_global = -1;
shutdown_eina:
   eina_shutdown();
   return --_conf_init_count;
}

int
conf_shutdown(void)
{
   if (_conf_init_count <= 0)
     {
        fprintf(stderr, "Conf init count not greater than 0 in shutdown.");
        return 0;
     }

   if (--_conf_init_count != 0)
     return _conf_init_count;

   ecore_shutdown();
   eina_log_domain_unregister(_conf_log_dom_global);
   _conf_log_dom_global = -1;
   eina_shutdown();
   return _conf_init_count;
}
