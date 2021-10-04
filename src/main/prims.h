#ifndef PRIMS_H_
#define PRIMS_H_
#include "datatypes.h"
#include "atoms.h"


struct preprim {
    void *thefunc;
    char *prim;
};

struct preprim *preprims; size_t numpreprims;

void preregisterprim( void *thefunc, char *str );
void finalizeprims();



#define prim(x) \
void p_##x ( struct process_state *P );\
static inline void __attribute__ ((constructor)) p_##x##_() { \
        preregisterprim( &p_##x, (#x)); \
}\
void p_##x ( struct process_state *P )

#endif
