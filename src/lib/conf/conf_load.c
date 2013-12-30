#include "conf_private.h"

/**
 * @addtogroup Lib-Conf-Functions
 * @{
 */

/**
 * @cond IGNORE
 */

/**
 * @brief Frees an allocated configuration value.
 * @param data Pointer to allocated conf value.
 *
 * This function is called by eina_hash_free() when we ask to free
 * the conf->variables hash table.
 */
void
conf_load_line_free(void *data)
{
   free(data);
}

/**
 * @brief Parse a line into a tuple of variable + value.
 * @param conf Conf structure.
 * @param line Line to parse.
 *
 * This function is called by conf_load_map_filter for any
 * line found in the configuration file.
 */
void
conf_load_line_parse(Conf *conf,
                     char *line)
{
   char **split,
         *variable,
         *value;
   Eina_Strbuf *buf;

   DBG("conf[%p] line[%s]", conf, line);

   if (line[0] == '#')
     return;

   split = eina_str_split(line, "=", 2);
   if (!split)
     {
        ERR("Failed to alloc 2D array");
        return;
     }

   if ((!split[0]) || (!split[1]))
     {
        ERR("Failed to split line correctly");
        return;
     }

   buf = eina_strbuf_new();
   if (!buf)
     {
        ERR("Failed to allocate string buffer");
        free(split[0]);
        free(split);
        return;
     }

   eina_strbuf_append(buf, split[0]);
   eina_strbuf_trim(buf);
   variable = eina_strbuf_string_steal(buf);

   eina_strbuf_append(buf, split[1]);
   eina_strbuf_trim(buf);
   value = eina_strbuf_string_steal(buf);

   eina_hash_add(conf->variables, variable, strdup(value));

   free(split[0]);
   free(split);
   free(variable);
   free(value);
   eina_strbuf_free(buf);
}

/**
 * @brief Read configuration file line by line.
 * @param data Conf_Load structure.
 * @param handler UNUSED.
 * @param map Data from configuration file.
 * @param length Length of data.
 *
 * @return EINA_TRUE.
 *
 * This function is called by eio_file_map_all(),
 * <b>from its own thread</b>, when it successfully loaded
 * the configuration file.
 */
Eina_Bool
conf_load_map_filter(void *data,
                     Eio_File *handler EINA_UNUSED,
                     void *map,
                     size_t length)
{
   Conf_Load *cl;
   Conf *conf;
   const char *s,
              *p1;
   char *line;

   cl = data;
   conf = cl->conf;
   s = map;

   while (1)
     {
        if ((size_t)(s-(const char *)map) == length)
          break;

        p1 = strchr(s, '\n');
        if (!p1)
          goto end_loop;

        if (p1 == s)
          goto end_loop;

        line = strndup(s, p1 -s);
        if (line[p1-s-1] == '\r')
          line[p1-s-1] = 0;

        conf_load_line_parse(conf, line);

        free((char *)line);

        s = p1;

        end_loop:
        s++;
     }

   return EINA_TRUE;
}

/**
 * @brief Call callback from app to tell we ended to load configuration file.
 *
 * @param data Conf_Load structure.
 * @param handler UNUSED.
 * @param map UNUSED.
 * @param length UNUSED.
 *
 * This function is called by eio_file_map_all() from the main loop.<br />
 * It is usefull for us to warn our caller that we ended parsing of file,
 * from main loop.
 */
void
conf_load_map_main(void *data,
                   Eio_File *handler EINA_UNUSED,
                   void *map EINA_UNUSED,
                   size_t length EINA_UNUSED)
{
   Conf_Load *cl;

   cl = data;
   DBG("cl[%p]", cl);

   cl->cb.done((void *)cl->cb.data, cl->conf);
   conf_free(cl->conf);
   free(cl);
}

/**
 * @brief Call callback from app to tell we got an error.
 *
 * @param data Conf_Load structure.
 * @param handler UNUSED.
 * @param error See errno(3)
 *
 * This function gets called by eio_file_map_all() if eio failed to mmap
 * file. This call is made from the main loop.
 */
void
conf_load_map_error(void *data,
                    Eio_File *handler EINA_UNUSED,
                    int error)
{
   Conf_Load *cl;

   cl = data;
   ERR("Error while mmaping file %s : %s", cl->conf->file, strerror(error));

   cl->cb.error((void *)cl->cb.data, cl->conf,
                "Error while mmaping file");
   conf_free(cl->conf);
   free(cl);
}

/**
 * @endcond
 */

/**
 * @}
 */
