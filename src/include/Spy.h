#ifndef SPY_H
#define SPY_H
#include <Eina.h>
#include <Ecore.h>
#include <Eio.h>

extern int SPY_EVENT_LINE;

typedef struct _Spy Spy;
typedef struct _Spy_File Spy_File;
typedef struct _Spy_Line Spy_Line;

int spy_init(void);
int spy_shutdown(void);

Spy * spy_new(void);
void spy_free(Spy *spy);

Spy_File * spy_file_new(Spy *spy, const char *file);
Spy_File * spy_file_get(Spy *spy, const char *file);
const char * spy_line_get(Spy_Line *sl);
const char * spy_file_name_get(Spy_File *sf);
void spy_file_data_set(Spy_File *sf, const void *data);
const void * spy_file_data_get(Spy_File *sf);

#endif
