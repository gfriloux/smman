#include "smman.h"

typedef struct _Log
{
   const char *filename,
              *source_host,
              *source_path,
              *message;
   Eina_Strbuf *tags;
   Eina_Bool todel;
} Log;

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
             NFO("Log \"%s\" is not affected by rule %s", log, rule->name);
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

   DBG("smman[%p] sl[%p][%s] filter[%p][%s]",
       smman, sl, spy_file_name_get(sf), filter, filter->filename);

   log = calloc(1, sizeof(Log));
   eina_stringshare_replace(&log->message, spy_line_get(sl));
   eina_stringshare_replace(&log->filename, filter->filename);
   eina_stringshare_replace(&log->source_host, smman->cfg.host);
   eina_stringshare_replace(&log->source_path, filter->filename);
   log->tags = eina_strbuf_new();

   /* Now we apply rules */
   it = eina_hash_iterator_tuple_new(filter->rules);
   while (eina_iterator_next(it, (void **)&t))
     {
        const char *name;

        name = t->key;
        rule = t->data;
        DBG("rule[%p][%s]", rule, name);

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

        if (rule->spec.tags)
          {
             if (eina_strbuf_length_get(log->tags))
               eina_strbuf_append(log->tags, ",");
             eina_strbuf_append(log->tags, rule->spec.tags);
          }
     }
   eina_iterator_free(it);

   if (log->todel)
     return EINA_TRUE;


   DBG("Log to index :\n"
       "\tmessage = %s\n"
       "\tfilename = %s\n"
       "\tsource_host = %s\n"
       "\tsource_path = %s\n"
       "\ttags = %s\n",
       log->message,
       log->filename,
       log->source_host,
       log->source_path,
       eina_strbuf_string_get(log->tags));

   return EINA_TRUE;
}
