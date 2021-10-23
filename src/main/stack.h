#ifndef STACK_H_
#define STACK_H_
#include "datatypes.h"
#include "atoms.h"


// returns a string containing the entire current stack contents
sds dump_stack( struct process_state *P );

// fetching values from a codestream (via codepoint)
int64_t cp_get_int( struct code_point *cp );
size_t cp_get_atom( struct code_point *cp );
sds cp_get_string( struct code_point *cp );

// type queries
static inline bool dp_is_int( struct datapoint *dp ) {
    return ( dp->u_val & 3 ) == 1 ;
}
static inline bool dp_is_atom( struct datapoint *dp ) {
    return ( dp->u_val & 3 ) == 2 ;
}
static inline bool dp_is_string( struct datapoint *dp ) {
    return ( dp ) && ( ( dp->u_val & 3 ) == 0 ) && (  ((struct dataobject * ) dp->p_val)->typeatom == a_type_string );
}

#define dstack P->d->stack
#define dcount P->d->top
#define topdp &dstack[dcount - 1]


// returns the boolean quality of a datapoint
bool checktrue ( struct datapoint *dp );
// returns atom indicating data object type
size_t checktype( struct datapoint *dp );
// renders string suitable for printing for debug
sds formatobject( struct node_state *N , struct datapoint *dp );

//
// manipulation of the contents of datapoints on a datastack
//
int64_t dp_get_int( struct datapoint *dp );
void dp_put_int( struct datapoint *dp, uint64_t i );
size_t dp_get_atom( struct datapoint *dp );
void dp_put_atom( struct datapoint *dp, size_t a );
sds dp_get_string( struct datapoint *dp );
void dp_put_string( struct datapoint *dp, sds s );


#define stackfault(x) P->errorstate = (x); return;

atom(stack_underflow)

#define needstack(x)     if( dcount < (x) ) { \
        stackfault (a_stack_underflow)\
    }


static inline void push_dp( struct process_state *P, struct datapoint *dp ) {
    dstack[dcount++] = *dp;
}
static inline struct datapoint* pop_dp(struct process_state *P ) {
    return &dstack[--dcount];
}

void push_int(struct process_state *P, uint64_t i );
void push_atom( struct process_state *P, size_t a );
void push_string( struct process_state *P, sds s );
void push_bool( struct process_state *P, bool t );

atom(expected_integer)
atom(expected_positive_integer)
atom(expected_atom)
atom(expected_string)

static inline int64_t pop_int( struct process_state *P ) {
    return dp_get_int( pop_dp( P ) );
}
#define require_int needstack(1) if( !dp_is_int( topdp ) ) { stackfault( a_expected_integer ) } int64_t

static inline size_t pop_atom( struct process_state *P ) {
    return dp_get_atom( pop_dp ( P ) );
}
#define require_atom needstack(1) if( !dp_is_atom( topdp ) ) { stackfault( a_expected_atom ) } size_t

static inline sds pop_string( struct process_state *P ) {
    return dp_get_string( pop_dp ( P ) );
}
#define require_string needstack(1) if( !dp_is_string( topdp ) ) { stackfault( a_expected_string ) } sds

static inline bool pop_bool(struct process_state *P ) {
    return checktrue( pop_dp( P ));
}
#define require_bool needstack(1) bool

#endif

