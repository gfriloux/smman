#ifndef CONF_H
#define CONF_H
#include <Eina.h>
#include <Ecore.h>
#include <Eio.h>

typedef struct _Conf Conf;

typedef void (*Conf_Done_Cb)(void *data, Conf *conf);
typedef void (*Conf_Error_Cb)(void *data, Conf *conf, const char *errstr);

Eina_Hash * conf_variables_get(Conf *conf);
Eina_Bool conf_load(char *file, Conf_Done_Cb done_cb, Conf_Error_Cb error_cb, const void *data);
int conf_init(void);
int conf_shutdown(void);
#endif
