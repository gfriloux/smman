#include <Rules.h>

extern int _rules_log_dom_global;

#define ERR(...) EINA_LOG_DOM_ERR(_rules_log_dom_global, __VA_ARGS__)
#define DBG(...) EINA_LOG_DOM_DBG(_rules_log_dom_global, __VA_ARGS__)
#define NFO(...) EINA_LOG_DOM_INFO(_rules_log_dom_global, __VA_ARGS__)
#define WRN(...) EINA_LOG_DOM_WARN(_rules_log_dom_global, __VA_ARGS__)
#define CRI(...) EINA_LOG_DOM_CRIT(_rules_log_dom_global, __VA_ARGS__)

struct _Rules
{
   const char *directory;
   Eina_Inlist *rules;
};

typedef struct _Rules_Load
{
   Rules *rules;

   struct
   {
      Rules_Progress_Cb progress;
      Rules_Done_Cb done;
      Rules_Error_Cb error;
      const void *data;
   } cb;
} Rules_Load;

typedef struct _Rule_Load
{
   Rules *rules;

   struct
   {
      Rules_Progress_Cb progress;
      Rules_Done_Cb done;
      Rules_Error_Cb error;
      const void *data;
   } cb;
} Rule_Load;

Eina_Bool rules_load_ls_filter(void *data, Eio_File *handler, const Eina_File_Direct_Info *info);
void rules_load_ls(void *data, Eio_File *handler, const Eina_File_Direct_Info *info);
void rules_load_ls_done(void *data, Eio_File *handler);
void rules_load_ls_error(void *data, Eio_File *handler, int error);
