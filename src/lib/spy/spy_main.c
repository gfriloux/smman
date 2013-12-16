#include "spy_private.h"
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

static int _spy_init_count = 0;
int _spy_log_dom_global = -1;

int SPY_EVENT_LINE = 0;

Spy *
spy_new(void)
{
   Spy *spy;

   spy = calloc(1, sizeof(Spy));
   DBG("spy[%p]", spy);
   return spy;
}

void
spy_free(Spy *spy)
{
   Eina_Inlist *l;
   Spy_File *sf;
   EINA_SAFETY_ON_NULL_RETURN(spy);

   EINA_INLIST_FOREACH_SAFE(spy->files, l, sf)
     {
        free((char *)sf->name);
        ecore_timer_del(sf->poll.timer);
        eina_strbuf_free(sf->read.buf);
     }
   free(spy);
}

Spy_File *
spy_file_new(Spy *spy, const char *file)
{
   Spy_File *sf;
   struct stat st;

   DBG("spy[%p] file[%s]", spy, file);

   if (access(file, R_OK))
     {
        ERR("Failed to access %s : %s", file, strerror(errno));
        return EINA_FALSE;
     }

   sf = calloc(1, sizeof(Spy_File));
   if (!sf)
     {
        ERR("Failed to allocate Spy_File structure");
        return EINA_FALSE;
     }

   sf->name = (const char *)strdup(file);
   if (!sf->name)
     {
        ERR("Failed to dupe string \"%s\"", file);
        goto free_sf;
     }

   if (stat(file, &st))
     {
        ERR("Failed to get size of %s : %s", file, strerror(errno));
        goto free_name;
     }

   sf->read.buf = eina_strbuf_new();
   if (!sf->read.buf)
     {
        ERR("Failed to create stringbuffer");
        goto free_name;
     }

   sf->poll.size = st.st_size;
   sf->poll.timer = ecore_timer_add(5.0, spy_file_poll, sf);

   spy->files = eina_inlist_append(spy->files, EINA_INLIST_GET(sf));
   DBG("spy_file[%p] size[%zd]", sf, st.st_size);
   return sf;

free_name:
   free((char *)sf->name);
free_sf:
   free(sf);
   return NULL;
}


int
spy_init(void)
{
   if (++_spy_init_count != 1)
     return _spy_init_count;

   if (!eina_init())
     {
        fprintf(stderr, "Spy can not initialize Eina\n");
        return --_spy_init_count;
     }

   _spy_log_dom_global = eina_log_domain_register("spy", EINA_COLOR_RED);
   if (_spy_log_dom_global < 0)
     {
        EINA_LOG_ERR("Spy can not create a general log domain");
        goto shutdown_eina;
     }

   if (!ecore_init())
     {
        ERR("Can not initialize Ecore");
        goto unregister_log_domain;
     }

   SPY_EVENT_LINE = ecore_event_type_new();

   return _spy_init_count;

shutdown_ecore:
   ecore_shutdown();
unregister_log_domain:
   eina_log_domain_unregister(_spy_log_dom_global);
   _spy_log_dom_global = -1;
shutdown_eina:
   eina_shutdown();
   return --_spy_init_count;
}


int
spy_shutdown(void)
{
   if (_spy_init_count <= 0)
     {
        fprintf(stderr, "Spy init count not greater than 0 in shutdown.");
        return 0;
     }

   if (--_spy_init_count != 0)
     return _spy_init_count;

   eio_shutdown();
   ecore_shutdown();
   eina_log_domain_unregister(_spy_log_dom_global);
   _spy_log_dom_global = -1;
   eina_shutdown();
   return _spy_init_count;
}
