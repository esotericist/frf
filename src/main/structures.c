#include "prims.h"
#include "stdlib.h"
#include "stack.h"
#include "vm.h"

atom(range_error)



#pragma GCC push_options
#pragma GCC optimize("align-functions=16")


void copy_span( struct array_span *source, struct array_span *dest ) {
        for(size_t i = 0; i < source->size; i++ ) {
            dest->elems[i] = source->elems[i];
        }
}

struct array_span* grow_span( struct array_span *arr ) {
    struct array_span *newarr = newarrayspan(arr->size + 1 );
    copy_span(arr, newarr);
    return newarr;
}

void copy_span_skip_source_idx( struct array_span *source, struct array_span *dest, size_t source_idx, size_t dest_idx ) {
        for(size_t i = 0; i < source->size; i++ ) {

            dest->elems[i] = source->elems[i];
        }

}

void copy_span_skip_dest_idx( struct array_span *source, struct array_span *dest, size_t dest_idx ) {
    size_t offset = 0;

        for(size_t i = 0; i < source->size + 1; i++ ) {
            if( i == dest_idx ) { 
                offset = 1;
            }
            dest->elems[i+offset] = source->elems[i];
        }

}

void structure_explode( proc *P ) {
    require_structure *arr = pop_array;
    for(size_t i = 0; i < arr->size; i++ ) {

        push_dp(P, arr->elems[i]);
    }
    push_int(arr->size);
}

#define get_item(arr, idx, dp) \
    if( idx >= 0 && idx < arr->size) { \
        dp.u_val = arr->elems[idx].u_val; \
    } else { \
        P->errorstate = a_range_error; \
        runtimefault( "error in %zu: index exceeds bounds\n" ); \
    }

#define set_item( arr, idx, dp, newarr)\
    if( idx >= 0 && idx < arr->size) {\
        newarr = newarrayspan( arr->size );\
        copy_span( arr, newarr );\
        newarr->elems[idx] = dp;\
    } else {\
        P->errorstate = a_range_error;\
        runtimefault( "error in %zu: index exceeds bounds\n" );\
    }


prim(getitem) {
    require_int idx = pop_int;
    require_structure *arr = pop_array;
    struct datapoint dp;
    get_item(arr, idx, dp)
    push_dp( P, dp);
}

prim( setitem ) {
    require_int idx = pop_int;
    needstack(1)
    struct datapoint arrdp= pop_dp(P);
    struct array_span *arr;
    if( dp_is_structure(arrdp)) {
        arr = dp_get_array(arrdp);
    } else {
        stackfault( a_expected_structure ) runtimefault( "error in %zu: expected array or tuple\n")
    }
    needstack(1) struct datapoint dp = pop_dp(P);
    struct array_span *newarr;
    set_item(arr, idx, dp, newarr)
    if(dp_is_array(arrdp)) {
        push_arr(newarr);    
    } else {
        push_tup(newarr);    
    }
}

// #region tuples

prim( tuple_make ) {
    require_int count = pop_int;
    if( count > 0 ) {
        needstack( count )
        struct array_span *arr = newarrayspan( count );
        for(size_t i = 0; i < count; i++ ) {
            arr->elems[count - (i + 1) ] = pop_dp(P);
        }
        push_tup(arr);
    } else {
        runtimefault( "error in %zu: tuple_make expects a positive integer\n" );
    }
}

prim( tuple_getitem ) {
    require_int idx = pop_int;
    require_tuple *arr = pop_tuple;
    struct datapoint dp;
    get_item(arr, idx, dp)
    push_dp( P, dp);
}

prim( tuple_setitem ) {
    require_int idx = pop_int;
    require_tuple *arr = pop_tuple;
    needstack(1) struct datapoint dp = pop_dp(P);
    struct array_span *newarr;
    set_item(arr, idx, dp, newarr)
    push_tup(newarr);    
}

prim( tuple_explode ) {
    structure_explode(P);
}

// #endregion

// #region arrays
prim(array_make) {
    require_int count = pop_int;
    if( count > 0 ) {
        needstack( count )
        struct array_span *arr = newarrayspan( count );
        for(size_t i = 0; i < count; i++ ) {
            arr->elems[count - (i + 1) ] = pop_dp(P);
        }
        push_arr(arr);
    } else {
        struct array_span *arr = newarrayspan(0);
        push_arr(arr);
    }
}

prim(array_vals) {
    require_arr *arr = pop_array;
    size_t count = arr->size; 
    for(size_t i = 0; i < count; i++ ) {
        push_dp(P, arr->elems[i]);
    }
}

prim(array_count) {
    require_arr *arr = pop_array;
    push_int( arr->size );
}

prim(array_getitem) {
    require_int idx = pop_int;
    require_arr *arr = pop_array;
    struct datapoint dp;
    get_item(arr, idx, dp)
    push_dp( P, dp);
}

prim(array_setitem) {
    require_int idx = pop_int;
    require_arr *arr = pop_array;
    needstack(1) struct datapoint dp = pop_dp(P);
    struct array_span *newarr;
    set_item(arr, idx, dp, newarr)
    push_arr(newarr);    
}

prim(array_appenditem) {
    require_arr *arr = pop_array;
    needstack(1) struct datapoint dp = pop_dp(P);
    struct array_span *newarr = grow_span( arr );
    newarr->elems[arr->size] = dp;
    push_arr(newarr);
}

prim(array_insertitem) {
    require_int idx = pop_int;
    require_arr *arr = pop_array;
    needstack(1) struct datapoint dp = pop_dp(P);
    if( idx >= 0 && idx <= arr->size) {
        struct array_span *newarr = newarrayspan( arr->size + 1 );
        copy_span_skip_dest_idx(arr, newarr, idx );
        newarr->elems[idx] = dp;
        push_arr(newarr);
    } else {
        P->errorstate = a_range_error;
        runtimefault( "error in %zu: range exceeds bounds\n" );
    }
}

prim(array_delitem) {
    require_int idx = pop_int;
    require_arr *arr = pop_array;
    size_t offset = 0;
    if( idx >= 0 && idx < arr->size) {
        struct array_span *newarr = newarrayspan( arr->size - 1);
        for(size_t i = 0; i < arr->size; i++ ) {
            if( i == idx ) { 
                offset = 1;
            } else {
                newarr->elems[i - offset] = arr->elems[i];
            }
        }
        push_arr(newarr);
    } else {
        P->errorstate = a_range_error;
        runtimefault( "error in %zu: range exceeds bounds\n" );
    }

}

prim(array_reverse) {
    require_arr *arr = pop_array;
    size_t count = arr->size;
    struct array_span *newarr = newarrayspan( count );
    for(size_t i = 0; i < count; i++ ) {
        newarr->elems[count - (i + 1) ] = arr->elems[i];
    }
    push_arr(newarr);
}

prim(array_join) {
    require_string s = pop_string;
    require_arr *arr = pop_array;
    sfs workingstring = sfsempty();
    for(size_t i = 0; i < arr->size; i++ ) {
        if( i > 0 ) {
            workingstring = sfscatsfs (workingstring, s );
        }
        workingstring = sfscatsfs( workingstring, objecttostring( arr->elems[i] ));
    }
    push_string(workingstring);    
}

prim(array_interpret) {
    require_arr *arr = pop_array;
    sfs workingstring = sfsempty();
    for(size_t i = 0; i < arr->size; i++ ) {
        workingstring = sfscatsfs( workingstring, objecttostring( arr->elems[i] ));
    }
    push_string(workingstring);    
}

prim(array_explode) {
    structure_explode(P);
}

// #endregion
