#include "stack.h"

size_t checktype( struct datapoint *dp ) {
    if( dp->u_val == 0 ) {
        return a_type_invalid;
    }
    if( ( dp->u_val & 3 ) == 0) {
        struct dataobject* dobj = (struct dataobject * ) dp->p_val;
        if( dobj->typeatom ) {
            return dobj->typeatom;
        } else {
            return a_type_empty;
        }
    } else if( dp_is_int(dp) ) {
        return a_type_integer;
    } else if( dp_is_atom(dp) ) {
        return a_type_atom;
    } else if( ( dp->u_val & 3 ) == 3 ) {
        return a_type_prim;
    }

    return a_type_unknown;
}

bool checktrue( struct datapoint *dp ) {
    if( dp_is_int( dp ) ) {
        return dp_get_int( dp ) != 0;
    }
    if( dp_is_atom( dp ) ) {
        return dp_get_atom ( dp ) == a_true;
    }
    if( dp_is_string( dp ) ) {
        return sdslen( dp_get_string( dp ) ) > 0;
    }
    return false;
}

sds dump_stack( struct process_state *P ) {
    sds s = sdsnew( "stack: " );
    if( dcount == 0 ) {
        return s;
    }
    for( size_t i = 0; i < dcount; i++ ) {
        if( i > 0 ) {
            s = sdscat( s, ", " );
        }

        s = sdscatsds( s, formatobject( P->node, &dstack[i] ) );
    }
    return s;
}

sds formatobject( struct node_state *N , struct datapoint *dp ) {
    sds workingstring = sdsempty();
    size_t dptype = checktype( dp );
    if( dptype == a_type_empty  ) {
        workingstring = sdscat( workingstring , "(empty)" );
    } else if( dptype == a_type_integer ) {
        workingstring = sdsfromlonglong ( dp_get_int( dp ) );
    } else if( dptype == a_type_float ) {
        // workingstring = sdscatprintf( workingstring, "%d" , dp_get_float( dp ) );
    } else if( dptype == a_type_atom ) {
        sds tempstring = atomtostring( dp_get_atom( dp ) );
        /*
        int *count;
        sds *strarr = sdssplitlen( tempstring, sdslen(tempstring), "\r", 2, count );
        tempstring =sdsjoin( strarr, &count, "\r" );
        */
        workingstring = sdscatprintf( workingstring, "'%s'", tempstring );
    } else if( dptype == a_type_string ) {
        workingstring = sdscatprintf( workingstring, "\"%s\"", dp_get_string( dp)  );
    } else if( dptype == a_type_stackmark ) {

    } else {
        workingstring = sdscat( workingstring, "(unknownobject)" );
    }

    return workingstring;
}

int64_t cp_get_int( struct code_point *cp ) {
    return (int64_t) cp->u_val;
}

size_t cp_get_atom( struct code_point *cp ) {
    return (size_t) cp->u_val;
}

sds cp_get_string( struct code_point *cp ) {
    return atomtostring( cp_get_atom( cp ) );
}


int64_t dp_get_int( struct datapoint *dp ) {
    return (int64_t) dp->u_val >> 2;
}

void dp_put_int( struct datapoint *dp, uint64_t i ) {
    dp->u_val = i << 2 | 1;
} 


size_t dp_get_atom( struct datapoint *dp ) {
    return (size_t) dp->u_val >> 2;
}

void dp_put_atom( struct datapoint *dp, size_t a ) {
    dp->u_val = a << 2 | 2;
} 


sds dp_get_string( struct datapoint *dp ) {
    struct dataobject *dobj = (struct dataobject*)( uintptr_t ) dp->p_val;
    return dobj->s_val;
}

void dp_put_string( struct datapoint *dp, sds s ) {
    struct dataobject *dobj = newdataobject();
    dobj->s_val = s;
    dobj->typeatom = a_type_string;
    dp->p_val = ( uintptr_t )(struct dataobject*)dobj;
}

void push_int(struct process_state *P, uint64_t i ) {
    dp_put_int( &dstack[dcount++], i );
}

void push_atom( struct process_state *P, size_t a ) {
    dp_put_atom( &dstack[dcount++], a );
}

void push_string( struct process_state *P, sds s ) {
    dp_put_string( &dstack[dcount++], s );
}

void push_bool( struct process_state *P, bool t ) {
    push_atom( P, t ? a_true : a_false );
}
