#include "store_private.h"

/**
 * @addtogroup Lib-Store-Functions
 * @{
 */

/**
 * @cond IGNORE
 */

/**
 * @brief Store received data from ElasticSearch server.
 * @param data Store_Add structure.
 * @param type UNUSED.
 * @param event_info Ecore_Con_Event_Url_Data structure.
 * @return EINA_TRUE.
 *
 * This function is called by ecore_con_url for every data block
 * received from web server.
 */
Eina_Bool
store_event_data(void *data,
                 int type EINA_UNUSED,
                 void *event_info)
{
   Store_Add *sa = data;
   Ecore_Con_Event_Url_Data *url_data = event_info;

   if ((sa != ecore_con_url_data_get(url_data->url_con)) || (!sa))
     return EINA_TRUE;

   DBG("sa[%p] url_data[%p] data_len=%i", sa, url_data, url_data->size);

   eina_strbuf_append_length(sa->data.buf,
                             (const char *)url_data->data,
                             url_data->size);
   return EINA_TRUE;
}

/**
 * @brief Storing is over and we got total answer.
 * @param data Store_Add structure.
 * @param type UNUSED.
 * @param event_info Ecore_Con_Event_Url_Complete.
 */
Eina_Bool
store_event_complete(void *data,
                     int type EINA_UNUSED,
                     void *event_info)
{
   Store_Add *sa = data;
   Ecore_Con_Event_Url_Complete *url_complete = event_info;
   char *errstr;
   int http_code;

   DBG("sa[%p] url_complete[%p] data_get[%p]",
       sa, url_complete, ecore_con_url_data_get(url_complete->url_con));

   if ((sa != ecore_con_url_data_get(url_complete->url_con)) || (!sa))
     return EINA_TRUE;

   http_code = ecore_con_url_status_code_get(sa->ec);
   if ((http_code != 200) && (http_code != 201))
     {
        errstr = store_utils_dupf("Server replied HTTP code %i", http_code);
        sa->cb.error((void *)sa->cb.data, sa->store, errstr);
        free(errstr);
        return EINA_TRUE;
     }

   sa->cb.done((void *)sa->cb.data,
               sa->store,
               (char *)eina_strbuf_string_get(sa->data.buf),
               eina_strbuf_length_get(sa->data.buf));
   store_add_free(sa);
   return EINA_TRUE;
}

/**
 * @endcond
 */

/**
 * @}
 */
