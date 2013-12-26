#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <Eina.h>
#include <Ecore.h>
#include <Ecore_Getopt.h>
#include <Eio.h>
#include <Conf.h>
#include <Rules.h>
#include <Spy.h>
#include <Store.h>
#include <cJSON.h>

int smman_log_dom_global;

typedef struct _Smman
{
   Rules *rules;
   Spy *spy;
   Store *store;
   Eina_Inlist *filters;

   struct
   {
      const char *server,
                 *host;
   } cfg;

   struct
   {
      Ecore_Event_Handler *sl; /* SPY_EVENT_LINE */
   } ev;
} Smman;

typedef struct _Filter
{
   EINA_INLIST;
   const char *filename;
   Spy_File *sf;
   Eina_Hash *rules;
} Filter;

#define ERR(...) EINA_LOG_DOM_ERR(smman_log_dom_global, __VA_ARGS__)
#define DBG(...) EINA_LOG_DOM_DBG(smman_log_dom_global, __VA_ARGS__)
#define NFO(...) EINA_LOG_DOM_INFO(smman_log_dom_global, __VA_ARGS__)
#define WRN(...) EINA_LOG_DOM_WARN(smman_log_dom_global, __VA_ARGS__)
#define CRI(...) EINA_LOG_DOM_CRIT(smman_log_dom_global, __VA_ARGS__)

void config_done(void *data, Conf *conf);
void config_error(void *data, Conf *conf, const char *errstr);

void filter_load(void *data, Rules *rules, Rule *rule);
void filter_load_done(void *data, Rules *rules);
void filter_load_error(void *data, Rules *rules, const char *errstr);

Eina_Bool log_line_event(void *data, int type, void *event);

char * sdupf(const char *s, ...);
char * date_es(void);
