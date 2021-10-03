#ifndef ATOMS_H_
#define ATOMS_H
#include "datatypes.h"

size_t verifyatom( sds str );
size_t stringtoatom( sds str );
size_t newatom( sds str );
sds atomtostring( size_t atom );
void atoms_init();

// atom pre-registration bits.
// the principle at work here is one can use the atom() macro to specify an atom in
// a given c file, in order to produce a variable that contains the numeric value
// of that atom once registered.
// since garbage collection isn't online before main() is invoked, we shove the
// addresses of the various declared variables and their accompanying strings into an
// array, then in atoms_init() we put those into the real atoms table and assign the
// numeric values into those variables.
// usage:
//
// atom(foo)
//
// void somefunc() {
//    printf("%s", atomtostring( a_foo ));
// }
struct preatom {
    size_t *thevar;
    char *atom;
};

struct preatom *preatoms; size_t numpreatoms;

void preregisteratom( size_t *var, char *str );

#define atom(x) \
size_t a_##x; \
static inline void __attribute__ ((constructor)) a_##x##_() { \
        preregisteratom( &a_##x, (#x)); \
}

#endif
