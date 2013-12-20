#include "spy_private.h"

void
_spy_file_line_free(void *d1,
                    void *d2 EINA_UNUSED)
{
   Spy_Line *sl = d1;

   free((char *)sl->line);
   free(sl);
}

static void
_spy_file_event(void *data)
{
   Spy_Line *sl;

   sl = data;
   ecore_event_add(SPY_EVENT_LINE, sl, _spy_file_line_free, sl);
}

static void
_spy_file_job(void *data)
{
   spy_file_poll(data);
}

void
_spy_file_line_extract(Spy_File *sf)
{
   const char *s,
              *p,
              *p1;
   size_t rm = 0;
   Spy_Line *sl;

   s = eina_strbuf_string_get(sf->read.buf);
   p = s;

   while ((p1=strchr(p, '\n')))
     {
        size_t l = p1 - p;
        char *line;

        rm += l;

        if (!l)
          goto end;

        line = strndup(p, l);

        DBG("Line = %s len=%zd", line, l);
        sl = calloc(1, sizeof(Spy_Line));
        if (!sl)
          {
             ERR("Failed to allocate Spy_Line");
             free(line);
          }

        sl->sf = sf;
        sl->line = line;
        ecore_main_loop_thread_safe_call_async(_spy_file_event, sl);
        end:
        p = p1 + 1;
     }

   eina_strbuf_remove(sf->read.buf, 0, rm);
}

void
_spy_file_cb(void *data,
             Ecore_Thread *thread EINA_UNUSED)
{
   Spy_File *sf;
   size_t nbr = 0,
          toread = 0,
          left = 0;
   char buf[512];

   sf = data;
   DBG("sf[%p]", sf);

   sf->read.fd = fopen(sf->name, "r");
   if (!sf->read.fd)
     {
        ERR("Failed to open %s : %s", sf->name, strerror(errno));
        return;
     }

   fseeko(sf->read.fd, sf->read.offset - 1, SEEK_SET);
   left = sf->read.length;

   while (nbr != (size_t)sf->read.length)
     {
        toread = (left < 512) ? left : 512;

        if (fread(buf, toread, 1, sf->read.fd) != 1)
          {
             ERR("Error while reading file %s : %s",
                 sf->name, strerror(errno));
             fclose(sf->read.fd);
             return;
          }
        eina_strbuf_append_length(sf->read.buf, buf, toread);
        _spy_file_line_extract(sf);
        nbr += toread;
     }
   fclose(sf->read.fd);
   sf->read.fd = NULL;
}

void
_spy_file_end_cb(void *data,
                 Ecore_Thread *thread EINA_UNUSED)
{
   Spy_File *sf;

   sf = data;
   DBG("sf[%p]", sf);

   ecore_timer_thaw(sf->poll.timer);
   ecore_timer_reset(sf->poll.timer);

   ecore_job_add(_spy_file_job, sf);
}

void
_spy_file_cancel_cb(void *data EINA_UNUSED,
                    Ecore_Thread *thread EINA_UNUSED)
{}

Eina_Bool
spy_file_poll(void *data)
{
   Spy_File *sf;
   off_t size,
         toread;
   struct stat st;
   Ecore_Thread *et;

   sf = data;
   DBG("spy_file[%p] file[%s]", sf, sf->name);

   /* We should have different actions made depending on error type. */
   if (stat(sf->name, &st))
     {
        ERR("Failed to get size of %s : %s", sf->name, strerror(errno));
        return EINA_TRUE;
     }

   size = st.st_size;
   if (sf->poll.size == size)
     return EINA_TRUE;

   /* File has been trunc! */
   if (sf->poll.size > size)
     {
         DBG("spy_file[%p] File trunc!", sf);
         sf->poll.size = 0;

         if (!size)
           return EINA_TRUE;
     }

   /* We have data to read! */
   toread = size - sf->poll.size;

   sf->read.offset = sf->poll.size;
   sf->read.length = toread;


   et = ecore_thread_run(_spy_file_cb,
                         _spy_file_end_cb,
                         _spy_file_cancel_cb,
                         sf);
   if (!et)
     {
        ERR("Failed to create reading thread");
        return EINA_TRUE;
     }

   ecore_timer_freeze(sf->poll.timer);
   sf->poll.size = size;
   return EINA_TRUE;
}

const char *
spy_file_name_get(Spy_File *sf)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(sf, NULL);
   return sf->name;
}

void
spy_file_data_set(Spy_File *sf,
                  const void *data)
{
   sf->data = (const void *)data;
}

void *
spy_file_data_get(Spy_File *sf)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(sf, NULL);
   return (void *)sf->data;
}
