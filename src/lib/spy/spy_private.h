#include <Spy.h>

extern int _spy_log_dom_global;

#define ERR(...) EINA_LOG_DOM_ERR(_spy_log_dom_global, __VA_ARGS__)
#define DBG(...) EINA_LOG_DOM_DBG(_spy_log_dom_global, __VA_ARGS__)
#define NFO(...) EINA_LOG_DOM_INFO(_spy_log_dom_global, __VA_ARGS__)
#define WRN(...) EINA_LOG_DOM_WARN(_spy_log_dom_global, __VA_ARGS__)
#define CRI(...) EINA_LOG_DOM_CRIT(_spy_log_dom_global, __VA_ARGS__)

struct _Spy
{
   Eina_Inlist *files;
};


struct _Spy_File
{
   EINA_INLIST;
   const char *name;
   const void *data;

   struct
   {
      Ecore_Timer *timer;
      off_t size;
   } poll;

   struct
   {
      FILE *fd;
      off_t offset,
            length;
      Eina_Strbuf *buf;
   } read;
};

struct _Spy_Line
{
   Spy_File *sf;
   const char *line;
};

Eina_Bool spy_file_poll(void *data);
