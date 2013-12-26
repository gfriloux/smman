#ifndef RULES_H
#define RULES_H
#include <Eina.h>
#include <Ecore.h>
#include <Eio.h>
#include <Conf.h>

#include <sys/types.h>
#include <regex.h>

typedef struct _Rules Rules;
typedef struct _Rule Rule;

struct _Rule
{
   EINA_INLIST;
   const char *name;

   struct
   {
      const char *filename,
                 *source_host,
                 *source_path;
      Eina_List *tags;
      Eina_Bool todel;
      Eina_Inlist *regex;
   } spec;
};

typedef struct _Rule_Regex
{
   EINA_INLIST;
   const char *regex;
   Eina_Bool must_match;
   regex_t preg;
} Rule_Regex;


typedef void (*Rules_Progress_Cb)(void *data, Rules *rules, Rule *rule);
typedef void (*Rules_Done_Cb)(void *data, Rules *rules);
typedef void (*Rules_Error_Cb)(void *data, Rules *rules, const char *errstr);

int rules_init(void);
int rules_shutdown(void);

Rules * rules_new(const char *directory);
Eina_Bool rules_load(Rules *rules, Rules_Progress_Cb progress_cb, Rules_Done_Cb done_cb, Rules_Error_Cb error_cb, void *data);

void rules_rule_free(Rule *rule);
#endif
