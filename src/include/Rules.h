#ifndef RULES_H
#define RULES_H
#include <Eina.h>
#include <Ecore.h>
#include <Eio.h>
#include <Conf.h>

typedef struct _Rules Rules;
typedef struct _Rule Rule;

typedef void (*Rules_Progress_Cb)(void *data, Rules *rules, Rule *rule);
typedef void (*Rules_Done_Cb)(void *data, Rules *rules);
typedef void (*Rules_Error_Cb)(void *data, Rules *rules, const char *errstr);

int rules_init(void);
int rules_shutdown(void);

Rules * rules_new(const char *directory);

#endif
