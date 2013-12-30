#include <Conf.h>

/**
 * @cond IGNORE
 */
extern int _conf_log_dom_global;

#define ERR(...) EINA_LOG_DOM_ERR(_conf_log_dom_global, __VA_ARGS__)
#define DBG(...) EINA_LOG_DOM_DBG(_conf_log_dom_global, __VA_ARGS__)
#define NFO(...) EINA_LOG_DOM_INFO(_conf_log_dom_global, __VA_ARGS__)
#define WRN(...) EINA_LOG_DOM_WARN(_conf_log_dom_global, __VA_ARGS__)
#define CRI(...) EINA_LOG_DOM_CRIT(_conf_log_dom_global, __VA_ARGS__)

struct _Conf
{
   const char *file;
   Eina_File *ef;

   Eina_Hash *variables;
};


typedef struct _Conf_Load
{
   Conf *conf;

   struct
   {
      Conf_Done_Cb done;
      Conf_Error_Cb error;
      const void *data;
   } cb;
} Conf_Load;

void conf_free(Conf *conf);

Eina_Bool conf_load_map_filter(void *data, Eio_File *handler, void *map, size_t length);
void conf_load_map_main(void *data, Eio_File *handler, void *map, size_t length);
void conf_load_map_error(void *data, Eio_File *handler, int error);

void conf_load_line_free(void *data);

/**
 * @endcond
 */
