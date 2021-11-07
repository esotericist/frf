#include <stdio.h>
#include <string.h>

#include "datatypes.h"
#include "atoms.h"
#include "stack.h"
#include "vm.h"



atom(callstack_overflow)
atom(exit)

#define callstack       P->c->stack
#define calltop         P->c->top

typedef void (*call_prim)(struct process_state *p);



void * atomtoprim( struct node_state *N, size_t atom ) {
    iListType *it = alloc_ilist();
    it->first.a_val = atom;
    struct ilist *tmp = sglib_hashed_iListType_find_member( N->atomtoprimtable, it );
    if(tmp) {
    return (void*)(uintptr_t)tmp->second.p_val;
    }
    return 0;
}

size_t primtoatom( struct node_state *N, void * ptr ) {
    iListType *tmp = alloc_ilist();
    tmp->first.p_val = (uintptr_t)ptr;
    return sglib_hashed_iListType_find_member( N->primtoatomtable, tmp )->second.a_val;
}

size_t wordtoatom( struct node_state *N, struct code_set* ptr ) {
    iListType *it = alloc_ilist();
    it->first.p_val = (uintptr_t)(void *)ptr;
    return sglib_hashed_iListType_find_member( N->wordtoatomtable, it )->second.a_val;
}

struct code_set* atomtoword( struct node_state *N, size_t atom ) {
    iListType *it = alloc_ilist();
    it->first.a_val = atom;
    struct ilist *tmp = sglib_hashed_iListType_find_member( N->atomtowordtable, it );
    if( tmp ) {
        return (void*)(uintptr_t)tmp->second.p_val;
    } else {
        return 0;
    }
}

void registerword(struct node_state *N, size_t atom, struct code_set *ptr ) {
    iListType *newword = alloc_ilist();
    newword->first.a_val = atom;
    newword->second.p_val = (uintptr_t)(void*)ptr;
    sglib_hashed_iListType_add( N->atomtowordtable, newword );
    iListType *newword2 = swap_ilist(newword);
    sglib_hashed_iListType_add( N->wordtoatomtable, newword2 );

}

void unregisterword(struct node_state *N, size_t atom) {
    iListType *newword = alloc_ilist();
    newword->first.a_val = atom;
    newword->second.p_val = (uintptr_t)(void*)atomtoword(N, atom);
    sglib_hashed_iListType_delete( N->atomtowordtable, newword );
    iListType *newword2 = swap_ilist( newword );
    sglib_hashed_iListType_delete( N->wordtoatomtable, newword2 );
}

void pushcallstackframe( struct process_state *P ){
    if( calltop < P->c->max ) {
        callstack[calltop].cword = P->current_codestream;
        callstack[calltop].varset = P->current_varset;
        callstack[calltop].currentop = P->currentop;
        calltop++;
    } else {
        P->errorstate = a_callstack_overflow;
    } 
}

void popcallstackframe( struct process_state *P ) {
    if( calltop > 0 ) {
        calltop--;
        P->current_codestream = callstack[calltop].cword;
        P->current_varset = callstack[calltop].varset;
        P->currentop = callstack[calltop].currentop;
        callstack[calltop].cword = 0;
        callstack[calltop].currentop = 0;
        callstack[calltop].varset = 0;
    } else {
        P->errorstate = a_exit;
    }
}


void * funcfrommungedptr( uintptr_t p ) {
    return (void *)(uintptr_t) ( p & ~(3) );
}

size_t executetimeslice( struct process_state *P, size_t steps ) {
    size_t count = 0;
    while( P->current_codestream->instructioncount > 0 ) {
        size_t pos = P->currentop++;
        uintptr_t check_op = P->current_codestream->codestream[pos].u_val;
        if( check_op == 0 || P->errorstate ) {
            P->current_codestream = 0;
            break;
        }
        if( check_op & 3 ) {
            sfs stackstate = sfsempty();
            if( P->debugmode ) {
                stackstate = dump_stack (P );
            }
            call_prim ptr = funcfrommungedptr( check_op );
            ptr( P );
            if( P->debugmode ) {
                sfs primname = atomtostring( primtoatom( P->node, ptr ) );
                printf("%s > %s\n", stackstate, primname );
            }        
        }
        if( count++ >= steps ) {
            break;
        }
    }
    return count;
}
