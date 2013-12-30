#include "conf_private.h"

/**
 * @addtogroup Lib-Conf-Functions
 * @{
 */


/**
 * @cond IGNORE
 */
static int _conf_init_count = 0;
int _conf_log_dom_global = -1;
/**
 * @endcond
 */

/**
 * @brief Returns a pointer to the filename of the conf.
 *
 * @param conf Conf structure.
 * @return Pointer to the filename. Do not free it as it is a pointer to the
 *         internal's buffer.
 */
const char *
conf_file_get(Conf *conf)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(conf, NULL);
   return conf->file;
}

/**
 * @brief Returns an hash table having the keys/values of the conf.
 *
 * @param conf Conf structure.
 * @return Pointer to the internal Hash table. Do not free it!
 */
Eina_Hash *
conf_variables_get(Conf *conf)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(conf, NULL);
   return conf->variables;
}

/**
 * @brief Frees an allocated Conf structure.
 *
 * @param conf Conf structure.
 */
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
   eina_hash_free(conf->variables);
   free(conf);
}

/**
 * @brief Loads the configuration file.
 *
 * @param file Configuration file to load.
 * @param done_cb Callback called in the main loop when the file has been
 *                successfully loaded.
 * @param error_cb Callback called in the main loop when the file couldn't
 *                 be loaded.
 * @param data Unmodified user data passed to callbacks.
 * @return EINA_TRUE if successfull, EINA_FALSE otherwise.
 */
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

/**
 * @brief Initialize conf and all it's required submodule.
 * @return 1 or greater on success, 0 otherwise.
 */
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

/**
 * @brief Shutdown conf and all it's submodule if possible.
 *
 * @return 0 if conf shuts down, greater than 0 otherwise.
 *         This function shuts down all things set up in conf_init()
 *         and cleans up its memory.
 */
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

   eio_shutdown();
   ecore_shutdown();
   eina_log_domain_unregister(_conf_log_dom_global);
   _conf_log_dom_global = -1;
   eina_shutdown();
   return _conf_init_count;
}

/**
 * @}
 */
