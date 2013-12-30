#include "spy_private.h"

#include <sys/stat.h>
#include <fcntl.h>
#include <sys/types.h>
#include <unistd.h>

/**
 * @addtogroup Lib-Spy-Functions
 * @{
 */

/**
 * @cond IGNORE
 */

/**
 * @brief Frees data associated to SPY_EVENT_LINE event.
 *
 * @param d1 Spy_Line structure to free.
 * @param d2 UNUSED
 */
void
_spy_file_line_free(void *d1,
                    void *d2 EINA_UNUSED)
{
   Spy_Line *sl = d1;

   free((char *)sl->line);
   free(sl);
}

/**
 * @brief Creates a SPY_EVENT_LINE event to ecore.
 *
 * @param data Spy_Line structure to associate to event.
 *
 * This function is called by _spy_file_line_extract(), but from
 * the main loop.
 */
static void
_spy_file_event(void *data)
{
   Spy_Line *sl;

   sl = data;
   ecore_event_add(SPY_EVENT_LINE, sl, _spy_file_line_free, sl);
}

/**
 * @brief Immediately launch a poll on given Spy_File structure.
 *
 * @param data Spy_File structure.
 *
 * This function is called by an ecore_job to start a poll.
 * It is recommended to do it using an ecore_job instead of a direct
 * call to avoid constant blocking on the main loop in case of a file
 * that is getting logs too frequently.
 */
static void
_spy_file_job(void *data)
{
   spy_file_poll(data);
}

/**
 * @brief Extract one line from buffered data.
 *
 * @param sf Spy_File structure to use for buffered data.
 *
 * This function is called by spy_file_cb(), and thus, running from
 * a thread.<br />
 * For each line found, this function will initiate a SPY_EVENT_LINE
 * event from the main loop.
 */
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

/**
 * @brief Read new lines from a file.
 *
 * @param data Spy_File structure of the file to read.
 * @param thread UNUSED.
 *
 * This function is running in a separate thread to not block the main
 * loop while reading and parsing file.
 */
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

/**
 * @brief Initiate another poll on the given file.
 *
 * @param data Spy_File structure to poll.
 * @param thread UNUSED.
 *
 * This function is called by ecore when thread running _spy_file_cb()
 * has ended.
 */
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

/**
 * @brief Unused function, would be called if we cancel our thread,
 *        but we never do it.
 *
 * @param data UNUSED.
 * @param thread UNUSED.
 */
void
_spy_file_cancel_cb(void *data EINA_UNUSED,
                    Ecore_Thread *thread EINA_UNUSED)
{}

/**
 * @endcond
 */

/**
 * @brief Verify is a file changed.
 *
 * @param data Spy_File structure of the file to check.
 *
 * @return EINA_TRUE.
 *
 * This function is called by the timer of the Spy_File, and will check
 * the filesize of the file to detect changes. It will detect the # of bytes
 * to read and start a thread to take care of it (reading/parsing).
 */
Eina_Bool
spy_file_poll(void *data)
{
   Spy_File *sf;
   off_t size,
         toread;
   struct stat st;
   Ecore_Thread *et;

   sf = data;
   if ((sf->poll.running) || (sf->poll.pause))
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

/**
 * @brief Returns the fullpath of the file being spied.
 *
 * @param sf Spy_File structure.
 *
 * @return Pointer to internal buffer. DO NOT FREE IT.
 */
const char *
spy_file_name_get(Spy_File *sf)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(sf, NULL);
   return sf->name;
}

/**
 * @brief Attach data to the Spy_File structure.
 *
 * @param sf Spy_File structure to attach data to.
 * @param data Pointer to the data to attach.
 */
void
spy_file_data_set(Spy_File *sf,
                  const void *data)
{
   EINA_SAFETY_ON_NULL_RETURN(sf);
   sf->data = (const void *)data;
}

/**
 * @brief Get the data attached to a Spy_File structure.
 *
 * @param sf Spy_File structure whose data is needed.
 *
 * @return Pointer to the data attached.
 */
void *
spy_file_data_get(Spy_File *sf)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(sf, NULL);
   return (void *)sf->data;
}

/**
 * @}
 */
