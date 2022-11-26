#ifndef events_H_
#define events_H_
#include "datatypes.h"
#include "atoms.h"
#include <uv.h>

extern uv_loop_t *uvloop;
void events_initialization();
void events_teardown();
void events_run();


#endif