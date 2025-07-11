#include "stack.h"

size_t checktype( struct datapoint dp ) {
    if( dp.u_val == 0 ) {
        return a_type_invalid;
    }
    if( ( dp.u_val & 3 ) == 0) {
        struct dataobject* dobj = (struct dataobject * ) dp.p_val;
        if( dobj->typeatom ) {
            return dobj->typeatom;
        } else {
            return a_type_empty;
        }
    } else if( dp_is_int(dp) ) {
        return a_type_integer;
    } else if( dp_is_atom(dp) ) {
        return a_type_atom;
    } else if( ( dp.u_val & 3 ) == 3 ) {
        return a_type_prim;
    }

    return a_type_unknown;
}

bool checktrue( struct datapoint dp ) {
    if( dp_is_int( dp ) ) {
        return dp_get_int( dp ) != 0;
    }
    if( dp_is_atom( dp ) ) {
        return dp_get_atom ( dp ) == a_true;
    }
    if( dp_is_string( dp ) ) {
        return sfslen( dp_get_string( dp ) ) > 0;
    }
    return false;
}

sfs dump_stack( proc *P ) {
    if( dcount == 0 ) {
        return sfsnew( "()" );
    }
    sfs s = sfsnew( "(" );
    for( size_t i = 0; i < dcount; i++ ) {
        if( i > 0 ) {
            s = sfscatc( s, ", " );
        }

        s = sfscatsfs( s, formatobject( P, dstack[i] ) );
    }
    return sfssubst( sfscatc(s, ")"), sfsnew("\n"), sfsnew("\\n") );
}

sfs objecttostring( struct datapoint dp ) {
    sfs workingstring = sfsempty();
    size_t dptype = checktype( dp );
    if( dptype == a_type_integer ) {    
        workingstring = sfsfromlonglong ( dp_get_int( dp ) );
    } else if( dptype == a_type_atom ) {
        workingstring = atomtostring( dp_get_atom( dp ) );
    } else if( dptype == a_type_string ) {
        workingstring = dp_get_string( dp);
    }
    return workingstring;
}

sfs formatobject( proc *P , struct datapoint dp ) {
    sfs workingstring = sfsempty();
    size_t dptype = checktype( dp );
    if( dptype == a_type_empty  ) {
        workingstring = sfscatc( workingstring , "(empty)" );
    } else if( dptype == a_type_integer ) {
        workingstring = sfsfromlonglong ( dp_get_int( dp ) );
    } else if( dptype == a_type_float ) {
        workingstring =  sfstrimtail( sfscatprintf( workingstring, "%f" , dp_get_float( dp ) ) , "0" );
        if ( !sfscmp( sfsright(workingstring, 1), sfsnew(".") ) ) {
            workingstring = sfscatc( workingstring, "0" );
        };

    } else if( dptype == a_type_atom ) {
        sfs tempstring = atomtostring( dp_get_atom( dp ) );
        workingstring = sfscatprintf( workingstring, "'%s'", tempstring );
    } else if( dptype == a_type_string ) {
        workingstring = sfscatprintf( workingstring, "\"%s\"", dp_get_string( dp)  );
    } else if( dptype == a_type_variable ) {
        int64_t thisvar = dp_get_var(dp, P->current_varset );
        if(thisvar >= 0) {
            size_t v_name = P->current_varset->vars[thisvar].name;
            struct datapoint *v_dp = & P->current_varset->vars[thisvar].dp;
            workingstring = sfscatprintf( workingstring, "%s@%s", atomtostring( v_name), formatobject(P, *v_dp )   );
        } else {
            workingstring = sfscatc( workingstring, "(invalidvariable)" );
        }
    } else if( dptype == a_type_array || dptype == a_type_tuple ) {
        sfs lead = sfsnew("%zu[");
        sfs tail = sfsnew("]");
        if( dptype == a_type_tuple) {
            lead = sfsnew("%zu{");
            tail = sfsnew("}");
        }
        struct array_span *arr = dp_get_array(dp);
        sfs array_contents = sfscatprintf(sfsempty(), lead , arr->size );
        if( arr->size > 0 ) {
            for( size_t i = 0; i < arr->size ; i++ ) {
                if (i > 25 ) {
                    array_contents = sfscatprintf( array_contents, ", ... %zu_ ", arr->size - 25 );
                    break;
                }
                if (i > 0 ) {
                    array_contents = sfscatc( array_contents, ", " );
                }
                struct datapoint *a_dp = & arr->elems[i];
                array_contents = sfscatsfs( array_contents, formatobject( P, *a_dp ) );
            }
            array_contents = sfscatc(array_contents, tail);
        } else {
            array_contents = sfscatc( array_contents, tail );
        }
        workingstring = sfscatsfs( workingstring, array_contents);
        
    } else if( dptype == a_type_stackmark ) {
        workingstring = sfsnew( "stackmark" );
    } else {
        workingstring = sfscatc( workingstring, "(unknownobject)" );
    }

    return workingstring;
}

int64_t cp_get_int( struct code_point *cp ) {
    return (int64_t) cp->u_val;
}

size_t cp_get_atom( struct code_point *cp ) {
    return (size_t) cp->u_val;
}
double cp_get_float( struct code_point *cp) {
    return (double) cp->d_val;
}

sfs cp_get_string( struct code_point *cp ) {
    return atomtostring( cp_get_atom( cp ) );
}


int64_t dp_get_int( struct datapoint dp ) {
    return (int64_t) dp.u_val >> 2;
}

void dp_put_int( struct datapoint *dp, uint64_t i ) {
    dp->u_val = i << 2 | 1;
} 


size_t dp_get_atom( struct datapoint dp ) {
    return (size_t) dp.u_val >> 2;
}

void dp_put_atom( struct datapoint *dp, size_t a ) {
    dp->u_val = a << 2 | 2;
} 

double dp_get_float( struct datapoint dp ) {
    struct dataobject *dobj = (struct dataobject*)( uintptr_t ) dp.p_val;
    return dobj->d_val;
}

void dp_put_float( struct datapoint *dp, double f ) {
    struct dataobject *dobj = newdataobject();
    dobj->d_val = f;
    dobj->typeatom = a_type_float;
    dp->p_val = ( uintptr_t )(struct dataobject*)dobj;
}


sfs dp_get_string( struct datapoint dp ) {
    struct dataobject *dobj = (struct dataobject*)( uintptr_t ) dp.p_val;
    return dobj->s_val;
}

void dp_put_string( struct datapoint *dp, sfs s ) {
    struct dataobject *dobj = newdataobject();
    dobj->s_val = s;
    dobj->typeatom = a_type_string;
    dp->p_val = ( uintptr_t )(struct dataobject*)dobj;
}


size_t dp_get_var( struct datapoint dp, struct variable_set *vs ) {
    if( vs == NULL) {
        return -1;
    }
    struct variable_object *vobj = (struct variable_object*)( uintptr_t ) dp.p_val;
    if(vobj->context == vs ) {
        return  vobj->dobj.p_val;
    }
    return -1;
}

void dp_put_var( struct datapoint *dp, size_t v, struct variable_set *vs ) {
    struct variable_object *vobj = newvarobject();
    vobj->dobj.typeatom = a_type_variable;
    vobj->dobj.p_val = v;
    vobj->context = vs;
    dp->p_val = ( uintptr_t )(struct variable_object*)vobj;
}

void dp_put_stackmark( struct datapoint *dp ) {
    struct dataobject *dobj = newdataobject();
    dobj->typeatom = a_type_stackmark;   
    dp->p_val = ( uintptr_t )(struct dataobject*)dobj;
 }

struct array_span* dp_get_array( struct datapoint dp ) {
    struct dataobject *dobj = (struct dataobject*)( uintptr_t ) dp.p_val;
    struct array_span *arr = (struct array_span*)( uintptr_t ) dobj->p_val;
    return arr;
}

void dp_put_array( struct datapoint *dp, struct array_span *arr ) {
    struct dataobject *dobj = newdataobject();
    dobj->p_val = ( uintptr_t )(struct array_span*) arr;
    dobj->typeatom = a_type_array;
    dp->p_val = ( uintptr_t )(struct dataobject*)dobj;
}

void dp_put_tuple( struct datapoint *dp, struct array_span *arr ) {
    struct dataobject *dobj = newdataobject();
    dobj->p_val = ( uintptr_t )(struct array_span*) arr;
    dobj->typeatom = a_type_tuple;
    dp->p_val = ( uintptr_t )(struct dataobject*)dobj;
}


