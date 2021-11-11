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

static inline bool dp_is_array( struct datapoint dp ) {
    return ( dp.u_val ) && ( ( dp.u_val & 3 ) == 0 ) && (  ( (struct datapoint*) dp.p_val)->p_val == a_type_array );
}

static inline bool dp_is_tuple( struct datapoint dp ) {
    return ( dp.u_val ) && ( ( dp.u_val & 3 ) == 0 ) && (  ( (struct datapoint*) dp.p_val)->p_val == a_type_tuple );
}

static inline bool dp_is_structure( struct datapoint dp ) {
    return ( dp.u_val ) && ( ( dp.u_val & 3 ) == 0 ) && ((  ( (struct datapoint*) dp.p_val)->p_val == a_type_array ) || (  ( (struct datapoint*) dp.p_val)->p_val == a_type_tuple ));
}

static inline bool dp_is_mark( struct datapoint dp ) {
    return ( dp.u_val ) && ( ( dp.u_val & 3 ) == 0 ) && (  ( (struct datapoint*) dp.p_val)->p_val == a_type_stackmark );
}


#define dstack P->d->stack
#define dcount P->d->top
#define topdp dstack[dcount - 1]


// returns the boolean quality of a datapoint
bool checktrue ( struct datapoint dp );
// returns atom indicating data object type
size_t checktype( struct datapoint dp );
// renders string suitable for string manipulation
sfs objecttostring( struct datapoint dp );
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
static inline size_t dp_get_mark( struct datapoint dp ) { return a_type_stackmark; }
void dp_put_mark( struct datapoint *dp );
struct array_span* dp_get_array( struct datapoint dp );
void dp_put_array( struct datapoint *dp, struct array_span *arr );
void dp_put_tuple( struct datapoint *dp, struct array_span *arr );



atom(stack_underflow)

#define fastexit if( P->errorstate ) { return; }
#define stackfault(x)  P->errorstate = (x); 

#define needstack(x)     fastexit if( dcount < (x) ) { \
        stackfault(a_stack_underflow)\
        runtimefault( "error in %zu: stackunderflow\n" )\
    }


static inline void push_dp( struct process_state *P, struct datapoint dp ) {
    dstack[dcount++] = dp;
}
static inline struct datapoint pop_dp(struct process_state *P ) {
    return dstack[--dcount];
}

static inline bool is_true( bool t ){ return t ? true : false; }

#define push_int(x)      fastexit dp_put_int( &dstack[dcount++], (x) );
#define push_atom(x)     fastexit dp_put_atom( &dstack[dcount++], (x) );
#define push_string(x)   fastexit dp_put_string( &dstack[dcount++], (x) );
#define push_bool(x)   push_atom( is_true(x) ? a_true : a_false );
#define push_var(x)      fastexit dp_put_var( &dstack[dcount++], (x), P->current_varset );
#define push_mark        fastexit dp_put_mark( &dstack[dcount++] );
#define push_arr(x)      fastexit dp_put_array( &dstack[dcount++], (x) );
#define push_tup(x)      fastexit dp_put_tuple( &dstack[dcount++], (x) );

atom(expected_integer)
atom(expected_positive_integer)
atom(expected_atom)
atom(expected_string)
atom(expected_variable)
atom(expected_mark)
atom(expected_structure)
atom(expected_array)
atom(expected_tuple)

#define pop_int dp_get_int( pop_dp( P ) )
#define require_int needstack(1) if( !dp_is_int( topdp ) ) { stackfault( a_expected_integer ) runtimefault( "error in %zu: expected integer\n")  } int64_t

#define pop_atom dp_get_atom( pop_dp( P  ) )
#define require_atom needstack(1) if( !dp_is_atom( topdp ) ) { stackfault( a_expected_atom ) runtimefault( "error in %zu: expected atom\n")  } size_t

#define pop_string dp_get_string( pop_dp( P ) )
#define require_string needstack(1) if( !dp_is_string( topdp ) ) { stackfault( a_expected_string ) runtimefault( "error in %zu: expected string\n")  } sfs

#define pop_var dp_get_var( pop_dp( P ) , P->current_varset  )
#define require_var needstack(1) if( !dp_is_var( topdp ) ) { stackfault( a_expected_variable ) runtimefault( "error in %zu: expected variable\n")  } int64_t

#define pop_array dp_get_array( pop_dp( P ) )
#define require_arr needstack(1) if( !dp_is_array( topdp ) ) { stackfault( a_expected_array ) runtimefault( "error in %zu: expected array\n")  } struct array_span

#define pop_tuple dp_get_array( pop_dp( P ) )
#define require_tuple needstack(1) if( !dp_is_tuple( topdp ) ) { stackfault( a_expected_tuple ) runtimefault( "error in %zu: expected tuple\n")  } struct array_span

#define require_structure needstack(1) if( !dp_is_structure( topdp ) ) { stackfault( a_expected_structure ) runtimefault( "error in %zu: expected array or tuple\n")  } struct array_span

#define pop_mark dp_get_mark( pop_dp( P ) )
#define require_mark needstack(1) if( !dp_is_mark( topdp ) ) { stackfault( a_expected_string ) runtimefault( "error in %zu: expected stackmark\n")  } bool

#define pop_bool checktrue( pop_dp( P ) )
#define require_bool needstack(1) bool

#endif

