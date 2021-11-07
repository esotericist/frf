#ifndef STACK_H_
#define STACK_H_
#include "datatypes.h"
#include "atoms.h"


// returns a string containing the entire current stack contents
sfs dump_stack( struct process_state *P );

// fetching values from a codestream (via codepoint)
int64_t cp_get_int( struct code_point *cp );
size_t cp_get_atom( struct code_point *cp );
sfs cp_get_string( struct code_point *cp );

// type queries
static inline bool dp_is_int( struct datapoint dp ) {
    return ( dp.u_val & 3 ) == 1 ;
}
static inline bool dp_is_atom( struct datapoint dp ) {
    return ( dp.u_val & 3 ) == 2 ;
}
static inline bool dp_is_string( struct datapoint dp ) {
    return ( dp.u_val ) && ( ( dp.u_val & 3 ) == 0 ) && (  ( (struct datapoint*) dp.p_val)->p_val == a_type_string );
}
static inline bool dp_is_var( struct datapoint dp ) {
    return ( dp.u_val ) && ( ( dp.u_val & 3 ) == 0 ) && (  ( (struct datapoint*) dp.p_val)->p_val == a_type_variable );
}

#define dstack P->d->stack
#define dcount P->d->top
#define topdp dstack[dcount - 1]


// returns the boolean quality of a datapoint
bool checktrue ( struct datapoint dp );
// returns atom indicating data object type
size_t checktype( struct datapoint dp );
// renders string suitable for printing for debug
sfs formatobject( struct process_state *P , struct datapoint dp );

//
// manipulation of the contents of datapoints on a datastack
//
int64_t dp_get_int( struct datapoint dp );
void dp_put_int( struct datapoint *dp, uint64_t i );
size_t dp_get_atom( struct datapoint dp );
void dp_put_atom( struct datapoint *dp, size_t a );
sfs dp_get_string( struct datapoint dp );
void dp_put_string( struct datapoint *dp, sfs s );
size_t dp_get_var( struct datapoint dp, struct variable_set *vs );
void dp_put_var( struct datapoint *dp, size_t v, struct variable_set *vs );


#define stackfault(x) P->errorstate = (x); return;

atom(stack_underflow)

#define needstack(x)     if( dcount < (x) ) { \
        stackfault (a_stack_underflow)\
    }


static inline void push_dp( struct process_state *P, struct datapoint dp ) {
    dstack[dcount++] = dp;
}
static inline struct datapoint pop_dp(struct process_state *P ) {
    return dstack[--dcount];
}

static inline bool is_true( bool t ){ return t ? true : false; }

#define push_int(x)   dp_put_int( &dstack[dcount++], (x) );
#define push_atom(x)     dp_put_atom( &dstack[dcount++], (x) );
#define push_string(x)    dp_put_string( &dstack[dcount++], (x) );
#define push_bool(x)    push_atom( is_true(x) ? a_true : a_false );
#define push_var(x)     dp_put_var( &dstack[dcount++], (x), P->current_varset );


atom(expected_integer)
atom(expected_positive_integer)
atom(expected_atom)
atom(expected_string)
atom(expected_var)

#define pop_int dp_get_int( pop_dp( P ) )
#define require_int needstack(1) if( !dp_is_int( topdp ) ) { stackfault( a_expected_integer ) } int64_t

#define pop_atom dp_get_atom( pop_dp( P  ) )
#define require_atom needstack(1) if( !dp_is_atom( topdp ) ) { stackfault( a_expected_atom ) } size_t

#define pop_string dp_get_string( pop_dp( P ) )
#define require_string needstack(1) if( !dp_is_string( topdp ) ) { stackfault( a_expected_string ) } sfs

#define pop_var dp_get_var( pop_dp( P ) , P->current_varset  )
#define require_var needstack(1) if( !dp_is_var( topdp ) ) { stackfault( a_expected_var ) } int64_t

#define pop_bool checktrue( pop_dp( P ) )
#define require_bool needstack(1) bool

#endif

