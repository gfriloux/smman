#include "spy_private.h"
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

/**
 * @addtogroup Lib-Spy-Functions
 * @{
 */

/**
 * @cond IGNORE
 */
static int _spy_init_count = 0;
int _spy_log_dom_global = -1;

int SPY_EVENT_LINE = 0;
/**
 * @endcond
 */

/**
 * @brief Create a new Spy structure.
 * @return Pointer to newly created Spy or NULL if allocation failed.
 */
Spy *
spy_new(void)
{
   Spy *spy;

   spy = calloc(1, sizeof(Spy));
   DBG("spy[%p]", spy);
   return spy;
}

/**
 * @brief Frees a Spy structure.
 *
 * @param Spy structure to free.
 *
 * This function will also free all the associated Spy_File structures.
 */
void
spy_free(Spy *spy)
{
   Eina_Inlist *l;
   Spy_File *sf;
   EINA_SAFETY_ON_NULL_RETURN(spy);

   EINA_INLIST_FOREACH_SAFE(spy->files, l, sf)
     spy_file_free(sf);
   free(spy);
}

/**
 * @brief Frees a Spy_File structure.
 *
 * @param sf Spy_File structure to free.
 *
 * This function will also remove the Spy_File from the Spy list.
 */
void
spy_file_free(Spy_File *sf)
{
   Spy_File *tmp;

   EINA_SAFETY_ON_NULL_RETURN(sf);

   EINA_INLIST_FOREACH(sf->spy->files, tmp)
     {
        if (sf != tmp)
          continue;

        sf->spy->files = eina_inlist_remove(sf->spy->files, sf->spy->files);
        break;
     }

   free((char *)sf->name);
   ecore_timer_del(sf->poll.timer);
   eina_strbuf_free(sf->read.buf);
   free(sf);
}

/**
 * @brief Pause spying of a file.
 *
 * @param sf Spy_File to pause.
 *
 * It doesnt stop its timer, but will block size checking.
 */
void
spy_file_pause(Spy_File *sf)
{
   EINA_SAFETY_ON_NULL_RETURN(sf);

   sf->poll.pause = EINA_TRUE;
}

/**
 * @brief Resume spying of a file.
 *
 * @param sf Spy_File to resume.
 *
 * This function allows to resume the spying of a file that got paused.
 */
void
spy_file_resume(Spy_File *sf)
{
   EINA_SAFETY_ON_NULL_RETURN(sf);

   sf->poll.pause = EINA_FALSE;
}

/**
 * @brief Get the Spy_File associated to a file.
 *
 * @param spy Spy structure to inpect.
 * @param file File to search for.
 * @return Spy_File structure spying the file, or NULL.
 *
 * This can be used to know if we are already spying a file or not
 */
Spy_File *
spy_file_get(Spy *spy, const char *file)
{
   Spy_File *sf;

   EINA_INLIST_FOREACH(spy->files, sf)
     {
        if (!strcmp(sf->name, file))
          return sf;
     }

   return NULL;
}

/**
 * @brief Start to spy a file.
 * @param spy Spy structure to attach file to.
 * @param file File to start spying.
 * @return Pointer to newly allocated Spy_File structure.
 *
 * This function will add a timer that will periodically look for changes
 * on the file, and report every new line inserted into it.
 */
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
   sf->poll.timer = ecore_timer_loop_add(0.3, spy_file_poll, sf);
   sf->spy = spy;

   spy->files = eina_inlist_append(spy->files, EINA_INLIST_GET(sf));
   DBG("spy_file[%p] size[%zd]", sf, st.st_size);
   return sf;

free_name:
   free((char *)sf->name);
free_sf:
   free(sf);
   return NULL;
}

/**
 * @brief Initialize spy and all it's required submodules.
 * @return 1 or greater on success, 0 otherwise.
 */
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

unregister_log_domain:
   eina_log_domain_unregister(_spy_log_dom_global);
   _spy_log_dom_global = -1;
shutdown_eina:
   eina_shutdown();
   return --_spy_init_count;
}

/**
 * @brief Shutdown spy and all it's submodules if possible.
 *
 * @return 0 if spy shuts down, greater than 0 otherwise.
 *         This function shuts down all things set up in spy_init()
 *         and cleans up its memory.
 */
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

   ecore_shutdown();
   eina_log_domain_unregister(_spy_log_dom_global);
   _spy_log_dom_global = -1;
   eina_shutdown();
   return _spy_init_count;
}

/**
 * @}
 */
