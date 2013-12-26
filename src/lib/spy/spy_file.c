#include "spy_private.h"

#include <sys/stat.h>
#include <fcntl.h>
#include <sys/types.h>
#include <unistd.h>

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
   Spy_Line *sl;

   DBG("sf[%p]", sf);

   while (1)
     {
        if (!eina_strbuf_length_get(sf->read.buf))
          break;

        sf->extract.s = eina_strbuf_string_get(sf->read.buf);
        sf->extract.p = strchr(sf->extract.s, '\n');
        if (!sf->extract.p)
          break;

        if (sf->extract.p == sf->extract.s)
          {
             eina_strbuf_remove(sf->read.buf, 0, 1);
             continue;
          }

        sf->extract.l = sf->extract.p - sf->extract.s;

        sl = calloc(1, sizeof(Spy_Line));
        if (!sl)
          {
             ERR("Failed to allocate Spy_Line");
             return;
          }

        sl->sf = sf;
        sl->line = strndup(sf->extract.s, sf->extract.l);
        ecore_main_loop_thread_safe_call_async(_spy_file_event, sl);
        eina_strbuf_remove(sf->read.buf, 0, sf->extract.l);
     }
}

void
_spy_file_cb(void *data,
             Ecore_Thread *thread EINA_UNUSED)
{
   Spy_File *sf = data;

   sf = data;
   DBG("sf[%p]", sf);

   sf->read.nbr = 0;

   sf->read.fd = open(sf->name, O_RDONLY);
   if (sf->read.fd == -1)
     {
        ERR("Failed to open %s : %s", sf->name, strerror(errno));
        return;
     }

   sf->read.databuf = calloc(1, sf->read.length + 1);

   errno = 0;
   sf->read.nbr = pread(sf->read.fd, sf->read.databuf,
                        sf->read.length, sf->read.offset);
   if (sf->read.nbr == -1)
     {
        ERR("Error while reading file %s : %s",
            sf->name, strerror(errno));
        close(sf->read.fd);
        free(sf->read.databuf);
        sf->read.databuf = NULL;
        return;
     }

   eina_strbuf_append_length(sf->read.buf, sf->read.databuf, sf->read.nbr);
   _spy_file_line_extract(sf);

   free(sf->read.databuf);
   sf->read.databuf = NULL;
   close(sf->read.fd);
   sf->poll.size += sf->read.nbr;
}

void
_spy_file_end_cb(void *data,
                 Ecore_Thread *thread EINA_UNUSED)
{
   Spy_File *sf;

   sf = data;
   DBG("sf[%p]", sf);

   sf->poll.running = EINA_FALSE;

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
   if (sf->poll.running)
     return EINA_TRUE;

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

   sf->poll.running = EINA_TRUE;
   et = ecore_thread_run(_spy_file_cb,
                         _spy_file_end_cb,
                         _spy_file_cancel_cb,
                         sf);
   if (!et)
     {
        ERR("Failed to create reading thread");
        return EINA_TRUE;
     }
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
   EINA_SAFETY_ON_NULL_RETURN(sf);
   sf->data = (const void *)data;
}

void *
spy_file_data_get(Spy_File *sf)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(sf, NULL);
   return (void *)sf->data;
}
