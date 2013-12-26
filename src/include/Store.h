#ifndef STORE_H
#define STORE_H
#include <Eina.h>
#include <Ecore.h>
#include <Ecore_Con.h>


typedef struct _Store Store;

typedef void (*Store_Done_Cb)(void *data, Store *store, char *answer, size_t len);
typedef void (*Store_Error_Cb)(void *data, Store *store, char *strerr);

int store_init(void);
int store_shutdown(void);

Store * store_new(const char *url);
Eina_Bool store_add(Store *store, const char *buf, size_t len, Store_Done_Cb done_cb, Store_Error_Cb error_cb, const void *data);

void store_data_set(Store *store, const void *data);
void * store_data_get(Store *store);
#endif
