#ifndef ATOMS_H_
#define ATOMS_H_
#include "datatypes.h"

size_t verifyatom( sfs str );
size_t stringtoatom( sfs str );
size_t newatom( sfs str );
sfs atomtostring( size_t atom );
void atoms_init();


// some pre-defined atoms we might need in c that can't be pre-registered
// using the pre-registration stuff below.
// initialized in atoms_init() along with pre-registered things
size_t a__dsign, a__dquote, a__squote, a__backslash, a__colon, a__semicolon;
size_t a__parenl, a__parenr, a__space, a__plus, a__percent, a__minus, a__newline;

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

#define atom2(x, y) \
size_t a_##x; \
static inline void __attribute__ ((constructor)) a_##x##_() { \
        preregisteratom( &a_##x, (#y)); \
}

atom(true)
atom(false)

atom(type_empty)
atom(type_object)
atom(type_atom)
atom(type_prim)
atom(type_integer)
atom(type_float)
atom(type_string)
atom(type_tuple)
atom(type_stackmark)
atom(type_variable)
atom(type_unknown)
atom(type_invalid)

#endif
