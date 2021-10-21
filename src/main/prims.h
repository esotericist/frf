#ifndef PRIMS_H_
#define PRIMS_H_
#include "datatypes.h"
#include "atoms.h"



void push_int(struct process_state *P, uint64_t i );
int64_t pop_int(struct process_state *P );

struct preprim {
    void *thefunc;
    char *prim;
};

struct preprim *preprims; size_t numpreprims;

void preregisterprim( void *thefunc, char *str );
void finalizeprims( struct node_state *N );
void * fetchprim( struct node_state *N, size_t atom );
sds dump_stack( struct process_state *P );


#define prim(x) \
void p_##x ( struct process_state *P );\
static inline void __attribute__ ((constructor)) p_##x##_() { \
        preregisterprim( &p_##x, (#x)); \
}\
void p_##x ( struct process_state *P )

#define prim2(x, y) \
void p_##x ( struct process_state *P );\
static inline void __attribute__ ((constructor)) p_##x##_() { \
        preregisterprim( &p_##x, (#y)); \
}\
void p_##x ( struct process_state *P )

#endif
