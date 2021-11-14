#ifndef STRUCTURES_H_
#define STRUCTURES_H_
#include "datatypes.h"
#include "atoms.h"

void copy_span( struct array_span *source, struct array_span *dest );
struct array_span* grow_span( struct array_span *arr );


#endif