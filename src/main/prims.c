#include "prims.h"
#include "stdlib.h"


void preregisterprim( void *thefunc,  char *s) {
    preprims = realloc( preprims, sizeof( struct preprim) * (numpreprims + 1));
    preprims[numpreprims].thefunc = thefunc;
    preprims[numpreprims].prim = s;
    numpreprims++;
}

extern sListType *atom_table[slist_hash_size];

void finalizeprims( struct node_state *N ) {
    for(int i = 0; i < numpreprims ; i++ ) {
        uintptr_t new_prim_ptr = (uintptr_t)(void*) preprims[i].thefunc;
        sds primname = sdstrim ( sdsnew( preprims[i].prim ), " " );
        size_t new_atom_val = stringtoatom( primname );
        iListType *new_prim_entry  = alloc_ilist();
        iListType *new_atom_entry = alloc_ilist();
        new_prim_entry->first.p_val = new_prim_ptr;
        new_prim_entry->second.a_val = new_atom_val;
        sglib_hashed_iListType_add( N->primtoatomtable, new_prim_entry ); 

        new_atom_entry->first.a_val = new_atom_val;
        new_atom_entry->second.p_val = new_prim_ptr;
        sglib_hashed_iListType_add( N->atomtoprimtable, new_atom_entry );

    }
    free(preprims);
}

void * fetchprim( struct node_state *N, size_t atom ) {
    iListType *it = alloc_ilist();
    it->first.a_val = atom;
    struct ilist *tmp = sglib_hashed_iListType_find_member( N->atomtoprimtable, it );
    return (void*)(uintptr_t)tmp->second.p_val;
}

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
    } else if( ( dp->u_val & 3 ) == 1 ) {
        return a_type_integer;
    } else if( ( dp->u_val & 3 ) == 2 ) {
        return a_type_atom;
    } else if( ( dp->u_val & 3 ) == 3 ) {
        return a_type_prim;
    }

    return a_type_unknown;
}

#pragma GCC push_options
#pragma GCC optimize ("align-functions=16")


int64_t dp_get_int( struct datapoint *dp ) {
    return (int64_t) dp->u_val >> 2;
}


size_t dp_get_atom( struct datapoint *dp ) {
    return (size_t) dp->u_val >> 2;
}

void push_int(struct process_state *P, uint64_t i ) {
    P->d->stack[P->d->top].u_val = (i << 2 ) | 1;
    P->d->top++;
}

void push_atom( struct process_state *P, size_t a ) {
    P->d->stack[P->d->top].u_val = (a << 2 ) | 1;
    P->d->top++;
}

int64_t pop_int( struct process_state *P ) {
    P->d->top--;
    int64_t i = dp_get_int( &P->d->stack[P->d->top] );
    P->d->stack[P->d->top].u_val = 0;    
    return i;
}

size_t pop_atom( struct process_state *P ) {
    P->d->top--;
    size_t a = dp_get_atom( &P->d->stack[P->d->top] );
    P->d->stack[P->d->top].u_val = 0;    
    return a;
}


sds dump_stack( struct process_state *P ) {
    sds s = sdsnew( "stack: " );
    for( size_t i = 0; i < P->d->top; i++ ) {
        if( i > 0 ) {
            s = sdscat( s, ", " );
        }
        struct datapoint *dp = &P->d->stack[i]; 
        size_t this_type = checktype( dp );
        if( this_type == a_type_integer ) {
            int64_t j = dp_get_int( dp );
            s = sdscatfmt(s, "i: %i", j );
           // printf( "i:%ld", j );
        } else if( this_type == a_type_atom ) {
            s = sdscatfmt( s , "a:%s", atomtostring( dp_get_atom( dp ) ) );
            // printf( "a:%s", atomtostring( dp_get_atom( dp ) ) );
        } else {
            s = sdscat( s, "?" );
        }
    }
    return s;
}

int64_t cp_get_int( struct code_point *cp ) {
    return (int64_t) cp->u_val;
}

prim(push_int) {
    size_t pos = P->currentop++;
    struct code_point *cp = &P->current_codestream->codestream[pos];
    push_int(P, cp_get_int( cp ) );
}

prim(pop_int) {
    printf( "%ld, ", pop_int(P) );
}

prim(dup) {
    P->d->stack[P->d->top ].u_val = P->d->stack[P->d->top - 1].u_val;
    P->d->top++;
}

prim(over) {
    P->d->stack[P->d->top ].u_val = P->d->stack[P->d->top - 2].u_val;
    P->d->top++;
}

prim(depth) {
    push_int( P, P->d->top );
}

prim(swap) {
    uint64_t second = pop_int ( P );
    uint64_t first = pop_int ( P );
    push_int( P, second );
    push_int( P, first );
}


prim(pop) {
    P->d->stack[ --P->d->top ].u_val = 0;
}

prim(popn) {
    size_t count = pop_int ( P );
    if( count >= 0 ) {
        for( size_t i = 0; i < count ; i ++ ) {
            pop_int( P );
        }
    }
}

// unconditional jump primarily used by the compiler
// pulls its address from the next cell
prim(jmp) {
    size_t pos = P->currentop++;
    struct code_point *cp = &P->current_codestream->codestream[pos];
    P->currentop = cp_get_int( cp );
}

// conditional jump, primarly used by the compiler
// uses the top entry of the stack for truth, pulls its address from the next cell
prim(cjmp) {
    size_t pos = P->currentop++;
    if( !pop_int( P ) ) {
     struct code_point *cp = &P->current_codestream->codestream[pos];
        P->currentop = cp_get_int( cp );
    }
}

prim(continue) {
    P->currentop = cp_get_int(&P->current_codestream->codestream[P->currentop]);
}

prim(break) {
    P->currentop = cp_get_int(&P->current_codestream->codestream[P->currentop]);
}

prim(while) {
    if( pop_int (P) ) {
        P->currentop++;
    } else {
        P->currentop = P->current_codestream->codestream[P->currentop].u_val;
    }
}

prim2( add, + ) {
    push_int( P, pop_int( P) + pop_int (P) );
}

prim2( minus, - ) {
    uint64_t second = pop_int ( P );
    uint64_t first = pop_int ( P );
    push_int( P, first - second );
}

prim2( mult, * ) {
    push_int( P, pop_int( P) * pop_int (P) );
}

prim2( div, / ) {
    uint64_t second = pop_int ( P );
    uint64_t first = pop_int ( P );
    push_int( P, first / second );
}

prim2( modulo, % ) {
    uint64_t second = pop_int ( P );
    uint64_t first = pop_int ( P );
    push_int( P, first % second );
}

prim2( isequalto, = ) {
    uint64_t second = pop_int ( P );
    uint64_t first = pop_int ( P );
    push_int( P, first == second );
}

prim2( isnotsequalto, != ) {
    uint64_t second = pop_int ( P );
    uint64_t first = pop_int ( P );
    push_int( P, first != second );
}

prim2( isgreaterthan, > ) {
    uint64_t second = pop_int ( P );
    uint64_t first = pop_int ( P );
    push_int( P, first > second );
}

prim2( islessthan, < ) {
    uint64_t second = pop_int ( P );
    uint64_t first = pop_int ( P );
    push_int( P, first < second );
}

prim2( isgreaterorequal, >= ) {
    uint64_t second = pop_int ( P );
    uint64_t first = pop_int ( P );
    push_int( P, first >= second );
}

prim2( islesserorequal, <= ) {
    uint64_t second = pop_int ( P );
    uint64_t first = pop_int ( P );
    push_int( P, first <= second );
}

prim( not ) {
    push_int( P,! pop_int( P) );    
}

prim( or ) {
    uint64_t second = pop_int ( P );
    uint64_t first = pop_int ( P );
    push_int( P, first || second );
}

prim( and ) {
    uint64_t second = pop_int ( P );
    uint64_t first = pop_int ( P );
    push_int( P, first && second );
}

prim(debugon) {
    P->debugmode = true;
}

prim(debugoff) {
    P->debugmode = false;
}

prim2( print, . ) {
    printf("  %ld", pop_int( P ) );
}