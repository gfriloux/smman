#include "smman.h"

typedef struct _Log
{
   const char *filename,
              *source_host,
              *source_path,
              *message;
   Eina_List *tags;
   Eina_Bool todel;
} Log;

void
_log_done(void *data,
          Store *store EINA_UNUSED,
          char *answer,
          size_t len)
{
   Smman *smman = data;

   DBG("smman[%p] Having a %zd bytes answer : \n%s\n", smman, len, answer);
}

void
_log_error(void *data,
           Store *store EINA_UNUSED,
           char *strerr)
{
   Smman *smman = data;

   ERR("smman[%p] Failed to store data :\n%s\n", smman, strerr);
}

void
_log_send(Smman *smman,
          Log *log)
{
   cJSON *json,
         *json_tags;
   char *source,
        *date,
        *tag,
        *s;
   Eina_List *l;

   json = cJSON_CreateObject();
   if (!json)
     {
        ERR("Failed to allocate JSON object");
        return;
     }

   source = sdupf("file://%s%s", smman->cfg.server, log->source_path);
   EINA_SAFETY_ON_NULL_RETURN(source);

   date = date_es();
   EINA_SAFETY_ON_NULL_RETURN(date);


   cJSON_AddStringToObject(json, "@source", source);
   cJSON_AddStringToObject(json, "@type", "syslog");

   json_tags = cJSON_CreateArray();
   cJSON_AddItemToObject(json, "@tags", json_tags);

   EINA_LIST_FOREACH(log->tags, l, tag)
     {
        cJSON_AddItemToArray(json_tags, cJSON_CreateString(tag));
     }

   cJSON_AddItemToObject(json, "@fields", cJSON_CreateObject());

   cJSON_AddStringToObject(json, "@message", log->message);
   cJSON_AddStringToObject(json, "@timestamp", date);
   cJSON_AddStringToObject(json, "@source_host", log->source_host);
   cJSON_AddStringToObject(json, "@source_path", log->source_path);

   s = cJSON_Print(json);

   store_add(smman->store, s, strlen(s), _log_done, _log_error, smman);

   free(s);
   free(date);
   free(source);
   cJSON_Delete(json);
}

void
_log_free(Log *log)
{
   char *tag;

   eina_stringshare_replace(&log->filename, NULL);
   eina_stringshare_replace(&log->source_host, NULL);
   eina_stringshare_replace(&log->source_path, NULL);
   eina_stringshare_replace(&log->message, NULL);

   EINA_LIST_FREE(log->tags, tag)
     free(tag);
   free(log);
}

Eina_Bool
_log_line_match(const char *log, Rule *rule)
{
   Rule_Regex *rr;
   Eina_Bool excluded = EINA_FALSE;
   int r;

   EINA_INLIST_FOREACH(rule->spec.regex, rr)
     {
        size_t nmatch = 2;
        regmatch_t pmatch[nmatch];

        r = regexec(&(rr->preg), log, nmatch, pmatch, 0);
        if (r == rr->must_match)
          {
             excluded = EINA_TRUE;
             break;
          }
     }
   return (excluded) ? EINA_FALSE : EINA_TRUE;
}

Eina_Bool
log_line_event(void *data,
               int type EINA_UNUSED,
               void *event)
{
   Smman *smman = data;
   Spy_Line *sl = event;
   Spy_File *sf = spy_line_spyfile_get(sl);
   Filter *filter = spy_file_data_get(sf);
   Log *log;
   Rule *rule;
   Eina_Iterator *it;
   Eina_Hash_Tuple *t;
   Eina_Bool r;
   const char *tag;

   DBG("smman[%p] sl[%p][%s] filter[%p][%s]",
       smman, sl, spy_file_name_get(sf), filter, filter->filename);

   log = calloc(1, sizeof(Log));
   eina_stringshare_replace(&log->message, spy_line_get(sl));
   eina_stringshare_replace(&log->filename, filter->filename);
   eina_stringshare_replace(&log->source_host, smman->cfg.host);
   eina_stringshare_replace(&log->source_path, filter->filename);

   /* Now we apply rules */
   it = eina_hash_iterator_tuple_new(filter->rules);
   while (eina_iterator_next(it, (void **)&t))
     {
        Eina_List *l;

        rule = t->data;

        r = _log_line_match(spy_line_get(sl), rule);
        if (!r)
          continue;

        if (rule->spec.todel)
          {
             log->todel = EINA_TRUE;
             break;
          }

        if (rule->spec.source_host)
          eina_stringshare_replace(&log->source_host, rule->spec.source_host);

        if (rule->spec.source_path)
          eina_stringshare_replace(&log->source_path, rule->spec.source_path);

        EINA_LIST_FOREACH(rule->spec.tags, l, tag)
          log->tags = eina_list_append(log->tags, strdup(tag));
     }
   eina_iterator_free(it);

   if (log->todel)
     return EINA_TRUE;

   _log_send(smman, log);

   _log_free(log);
   return EINA_TRUE;
}
