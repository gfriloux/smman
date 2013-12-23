#include <Store.h>

extern int _store_log_dom_global;

#define ERR(...) EINA_LOG_DOM_ERR(_store_log_dom_global, __VA_ARGS__)
#define DBG(...) EINA_LOG_DOM_DBG(_store_log_dom_global, __VA_ARGS__)
#define NFO(...) EINA_LOG_DOM_INFO(_store_log_dom_global, __VA_ARGS__)
#define WRN(...) EINA_LOG_DOM_WARN(_store_log_dom_global, __VA_ARGS__)
#define CRI(...) EINA_LOG_DOM_CRIT(_store_log_dom_global, __VA_ARGS__)


struct _Store
{
   const char *url;
   const void *data;
};

typedef struct _Store_Add
{
   Store *store;
   Ecore_Con_Url *ec;

   struct
   {
      Eina_Strbuf *buf;
      size_t len;
   } data;

   struct
   {
      const void *data;
      Store_Done_Cb done;
      Store_Error_Cb error;
   } cb;

   struct
   {
      Ecore_Event_Handler *ed,
                          *ec;
   } ev;
} Store_Add;

void store_add_free(Store_Add *sa);

Eina_Bool store_event_data(void *data, int type, void *event_info);
Eina_Bool store_event_complete(void *data, int type, void *event_info);

char * store_utils_dupf(const char *s, ...);
