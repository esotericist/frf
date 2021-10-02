#ifndef ATOMS_H_
#define ATOMS_H
#include "datatypes.h"

size_t verifyatom( sds str );
size_t stringtoatom( sds str );
size_t newatom( sds str );
sds atomtostring( size_t atom );
void atoms_init();
#endif
