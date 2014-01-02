#include <Store.h>

/**
 * @addtogroup Lib-Store-Functions
 * @{
 */

/**
 * @cond IGNORE
 */
extern int _store_log_dom_global;

#define ERR(...) EINA_LOG_DOM_ERR(_store_log_dom_global, __VA_ARGS__)
#define DBG(...) EINA_LOG_DOM_DBG(_store_log_dom_global, __VA_ARGS__)
#define NFO(...) EINA_LOG_DOM_INFO(_store_log_dom_global, __VA_ARGS__)
#define WRN(...) EINA_LOG_DOM_WARN(_store_log_dom_global, __VA_ARGS__)
#define CRI(...) EINA_LOG_DOM_CRIT(_store_log_dom_global, __VA_ARGS__)

/**
 * @brief Main structure.
 */
struct _Store
{
   const char *url; /*!< URL to store data to. */
   const void *data; /*!< Unmodified user data attached to structure. */
};

/**
 * @brief Store_Add structure.
 *
 * This structure helps to manage storage of data.
 */
typedef struct _Store_Add
{
   Store *store; /*!< Store structure */
   Ecore_Con_Url *ec; /*!< Ecore_Con_Url structure used for storage */

   struct
   {
      Eina_Strbuf *buf; /*!< Buffer for data reception */
   } data;

   struct
   {
      const void *data; /*!< Unmodified user data attached to storage */
      Store_Done_Cb done; /*!< Callback to call when storage is over */
      Store_Error_Cb error; /*!< Callback to call if an error occured */
   } cb;

   struct
   {
      Ecore_Event_Handler *ed, /*!< Event handler for data reception */
                          *ec; /*!< Event handler for complete http query */
   } ev;
} Store_Add;

void store_add_free(Store_Add *sa);

Eina_Bool store_event_data(void *data, int type, void *event_info);
Eina_Bool store_event_complete(void *data, int type, void *event_info);

char * store_utils_dupf(const char *s, ...);

/**
 * @endcond
 */

/**
 * @}
 */
