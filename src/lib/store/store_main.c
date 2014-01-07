#include "store_private.h"

/**
 * @addtogroup Lib-Store-Functions
 * @{
 */

/**
 * @cond IGNORE
 */

static int _store_init_count = 0;
int _store_log_dom_global = -1;

/**
 * @brief Frees a Store_Add structure.
 * @param sa Store_Add structure to free.
 */
void
store_add_free(Store_Add *sa)
{
   if (sa->ec) ecore_con_url_free(sa->ec);
   if (sa->data.buf) eina_strbuf_free(sa->data.buf);
   if (sa->ev.ed) ecore_event_handler_del(sa->ev.ed);
   if (sa->ev.ec) ecore_event_handler_del(sa->ev.ec);
   free(sa);
}

/**
 * @endcond
 */

/**
 * @brief Store given data to store's url.
 * @param store Store structure.
 * @param buf Buffer to store.
 * @param len Length of @buf.
 * @param done_cb Callback to call when data is stored.
 * @param error_cb Callback to call if an error occured.
 * @param data Data to pass to callbacks.
 * @return EINA_TRUE if we try to store data.
 *         EINA_FALSE if an error occured when creating storing process.
 */
Eina_Bool
store_add(Store *store,
          const char *buf,
          size_t len,
          Store_Done_Cb done_cb,
          Store_Error_Cb error_cb,
          const void *data)
{
   Store_Add *sa;
   Eina_Bool r;

   sa = calloc(1, sizeof(Store_Add));
   if (!sa)
     {
        ERR("Failed to allocate Store_Add structure");
        return EINA_FALSE;
     }

   sa->ec = ecore_con_url_new(store->url);
   if (!sa->ec)
     {
        ERR("Failed to create ecore_con_url object");
        goto sa_free;
     }

   sa->data.buf = eina_strbuf_new();
   if (!sa->data.buf)
     {
        ERR("Failed to allocate storage buffer");
        goto sa_con_url_free;
     }

   sa->ev.ed = ecore_event_handler_add(ECORE_CON_EVENT_URL_DATA,
                                       store_event_data, sa);
   if (!sa->ev.ed)
     {
        ERR("Failed to create event handler");
        goto sa_buf_free;
     }
   sa->ev.ec = ecore_event_handler_add(ECORE_CON_EVENT_URL_COMPLETE,
                                       store_event_complete, sa);
   if (!sa->ev.ec)
     {
        ERR("Failed to create event handler");
        goto sa_event_handler_free_ed;
     }

   sa->store = store;
   sa->cb.done = done_cb;
   sa->cb.error = error_cb;
   sa->cb.data = data;
   DBG("store[%p] sa[%p] buf[%s]", store, sa, buf);
   ecore_con_url_data_set(sa->ec, sa);
   r = ecore_con_url_post(sa->ec, buf, len, "text/json");
   if (!r)
     {
        ERR("Failed to issue POST method");
        goto sa_event_handler_free_ec;
     }

   return EINA_TRUE;

sa_event_handler_free_ec:
   ecore_event_handler_del(sa->ev.ec);
sa_event_handler_free_ed:
   ecore_event_handler_del(sa->ev.ed);
sa_buf_free:
   eina_strbuf_free(sa->data.buf);
sa_con_url_free:
   ecore_con_url_free(sa->ec);
sa_free:
   free(sa);
   return EINA_FALSE;
}

/**
 * @brief Attach given data to the Store structure.
 * @param store Store structure to attach data to.
 * @param data Data to attach to structure.
 */
void
store_data_set(Store *store, const void *data)
{
   store->data = data;
}

/**
 * @brief Get unmodified data attached to the store structure.
 * @param store Store structure.
 * @return Pointer to attached data.
 */
void *
store_data_get(Store *store)
{
   return (void *)store->data;
}

/**
 * @brief Create a new store structure.
 * @param url URL for storing data.
 * @return Pointer to newly created Store structure.
 */
Store *
store_new(const char *url)
{
   Store *store;

   store = calloc(1, sizeof(Store));
   if (!store)
     {
        ERR("Failed to allocate Store object");
        return NULL;
     }
   store->url = eina_stringshare_add(url);
   if (!store->url)
     {
        ERR("Failed to allocate URL string");
        goto store_free;
     }

   return store;

store_free:
   free(store);
   return NULL;
}

/**
 * @brief Frees an allocated Store structure.
 * @param store Store structure to free.
 */
void
store_free(Store *store)
{
   EINA_SAFETY_ON_NULL_RETURN(store);

   eina_stringshare_replace(&store->url, NULL);
   free(store);
}

/**
 * @brief Initialize the store library and all it's required submodules.
 * @return 1 or greater on success, 0 otherwise.
 */
int
store_init(void)
{
   if (++_store_init_count != 1)
     return _store_init_count;

   if (!eina_init())
     {
        fprintf(stderr, "Store can not initialize Eina\n");
        return --_store_init_count;
     }

   _store_log_dom_global = eina_log_domain_register("store", EINA_COLOR_RED);
   if (_store_log_dom_global < 0)
     {
        EINA_LOG_ERR("Store can not create a general log domain");
        goto shutdown_eina;
     }

   if (!ecore_init())
     {
        ERR("Can not initialize Ecore");
        goto unregister_log_domain;
     }

   if (!ecore_con_init())
     {
        ERR("Can not initialize Ecore_Con");
        goto shutdown_ecore;
     }

   if (!ecore_con_url_init())
     {
        ERR("Can not initialize Ecore_Con_Url");
        goto shutdown_ecore_con;
     }

   if (!ecore_con_url_pipeline_get())
     ecore_con_url_pipeline_set(EINA_TRUE);

   return _store_init_count;

shutdown_ecore_con:
   ecore_con_shutdown();
shutdown_ecore:
   ecore_shutdown();
unregister_log_domain:
   eina_log_domain_unregister(_store_log_dom_global);
   _store_log_dom_global = -1;
shutdown_eina:
   eina_shutdown();
   return --_store_init_count;
}

/**
 * @brief Shutdown store and all it's submodules if possible.
 * @return 0 if store shuts down, greater than 0 otherwise.
 *         This function shuts down all things set up in store_init()
 *         and cleans up its memory.
 */
int
store_shutdown(void)
{
   if (_store_init_count <= 0)
     {
        fprintf(stderr, "Store init count not greater than 0 in shutdown.");
        return 0;
     }

   if (--_store_init_count != 0)
     return _store_init_count;

   ecore_con_url_shutdown();
   ecore_con_shutdown();
   ecore_shutdown();
   eina_log_domain_unregister(_store_log_dom_global);
   _store_log_dom_global = -1;
   eina_shutdown();
   return _store_init_count;
}

/**
 * @}
 */
