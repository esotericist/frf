#ifndef files_H_
#define files_H_
#include "datatypes.h"
#include "atoms.h"
#include <uv.h>

extern uv_loop_t *uvloop;
void events_initialization();
void events_teardown();


#endif