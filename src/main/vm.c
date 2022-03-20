#include <stdio.h>
#include <string.h>

#include "datatypes.h"
#include "atoms.h"
#include "stack.h"
#include "vm.h"


atom(active)
atom(inactive)
atom(killed)
atom(callstack_overflow)
atom(exit)



void definelist_add( struct node_state *N, sfs key, sfs value ) {
    sfs k = sfsnew(key);
    if ( !(sfscmp( k, sfsempty() ))) {
        return;
    }
    sListType *elem = alloc_slist();
    elem->s = k;
    elem->ptr = (uintptr_t) (void *)sfsnew(value);
    sglib_hashed_sListType_add( N->definetable, elem );    
}

struct node_state* newnode() {
    struct node_state *new_N = GC_malloc( sizeof(  struct node_state) );
    sglib_hashed_iListType_init( new_N->atomtoprimtable );
    sglib_hashed_iListType_init( new_N->atomtowordtable );
    sglib_hashed_iListType_init( new_N->primtoatomtable );
    sglib_hashed_iListType_init( new_N->atomtowordtable );
    sglib_hashed_sListType_init( new_N->definetable );
    sglib_hashed_iListType_init( new_N->process_table );
    new_N->next_pid = 1;

    definelist_add( new_N, "case", "begin dup " );
    definelist_add( new_N, "when", "if pop " );
    definelist_add( new_N, "end", "break then dup " );
    definelist_add( new_N, "default", "pop 1 if " );
    definelist_add( new_N, "endcase", "pop pop 1 until " );
    definelist_add( new_N, "}tuple", "} tuple_make" );
    definelist_add( new_N, "}array", "} array_make" );
    definelist_add( new_N, "0@", "0 getitem" );
    definelist_add( new_N, "1@", "1 getitem" );
    definelist_add( new_N, "2@", "2 getitem" );
    definelist_add( new_N, "3@", "3 getitem" );
    definelist_add( new_N, "4@", "4 getitem" );
    definelist_add( new_N, "5@", "5 getitem" );
    definelist_add( new_N, "0!", "0 setitem" );
    definelist_add( new_N, "1!", "1 setitem" );
    definelist_add( new_N, "2!", "2 setitem" );
    definelist_add( new_N, "3!", "3 setitem" );
    definelist_add( new_N, "4!", "4 setitem" );
    definelist_add( new_N, "5!", "5 setitem" );

    definelist_add( new_N, "", "" );

    return new_N;
}

void process_setactive( proc *P ) {

    if(P->activitylist_ptr == NULL )  {
        P->activitylist_ptr = alloc_ilist();
        P->activitylist_ptr->first.a_val = P->pid;
        P->activitylist_ptr->second.p_val = (uintptr_t)(proc *) P;
    }

    if(P->executestate == a_inactive) {
        sglib_iListType_delete(&P->node->processlist_inactive, P->activitylist_ptr );
    }
    if(P->executestate != a_active) {
        P->executestate = a_active;
        
        sglib_iListType_add(&P->node->processlist_active, P->activitylist_ptr );
    }
}

void process_setinactive( proc *P ) {
    if(P->executestate == a_active) {
        sglib_iListType_delete(&P->node->processlist_active, P->activitylist_ptr );
    }
    if(P->executestate != a_inactive) {
        P->executestate = a_inactive;
        sglib_iListType_add(&P->node->processlist_inactive, P->activitylist_ptr );
    }
}

void process_setdead( proc *P ) {
    iListType *ptr = P->activitylist_ptr;
    if(P->executestate == a_active) {
        sglib_iListType_delete(&P->node->processlist_active, P->activitylist_ptr );
    }
    if(P->executestate == a_inactive) {
        sglib_iListType_delete(&P->node->processlist_inactive, P->activitylist_ptr );
    }
    
    if( ! sglib_iListType_is_member( P->node->processlist_dead, P->activitylist_ptr ) ) {
        P->executestate = a_killed;
        sglib_iListType_add(&P->node->processlist_dead, ptr );
    }    
}

void process_reset( proc *P, size_t reasonatom ) {
    if( P->current_codestream->nameatom && P->compilestate && P->compilestate->parsemode.compile  ) {
        printf( "undefining word %s\n", atomtostring( P->current_codestream->nameatom) );
        iListType *entry = alloc_ilist();
        entry->first.a_val = P->current_codestream->nameatom;
        entry->second.p_val = (uintptr_t)(void *) P->current_codestream;
        iListType *result;
        sglib_hashed_iListType_delete_if_member( P->node->atomtowordtable, entry, &result );
        swap_ilist(entry);
        sglib_hashed_iListType_delete_if_member(P->node->wordtoatomtable, entry, &result );
        P->current_codestream->nameatom = 0;
    }

    process_setdead( P );
    P->debugmode = 0;
    P->currentop = 0;
    P->current_codestream = 0;
    P->errorstate = reasonatom;
}

proc* process_from_pid(struct node_state *N, size_t pid ) {
    iListType *P_search = alloc_ilist();
    P_search->first.a_val = pid;    

    iListType *P_result = sglib_hashed_iListType_find_member( N->process_table, P_search );
    if( P_result == NULL) {
        return NULL;
    } else {
        return (proc *)(uintptr_t)P_result->second.p_val;
    }
}


void process_kill( proc *P, size_t reasonatom, sfs text ) {
    
    if( sfsmatchcount( text, sfsnew( "%zu" ) ) ) {
        printf(text, P->pid, atomtostring(reasonatom) );
    } else {
        printf(text, atomtostring(reasonatom) );
    }
    process_reset( P, reasonatom );
}

void process_addmessage( proc *P, struct array_span *arr ) {
    qListType *message = alloc_qlist();
    message->p_val = (uintptr_t)(struct array_span *) arr;
    sglib_qListType_concat( &P->messagequeue, message );
    if( P->executestate == a_inactive ) {
        process_setactive(P);
    }
}

size_t process_messagecount( proc *P ) {
    return sglib_qListType_len( P->messagequeue );
}

struct array_span * process_fetchmessage( proc *P ) {
    qListType *first = P->messagequeue;
    struct array_span *arr = (struct array_span *)(uintptr_t) first->p_val;
    sglib_qListType_delete(&P->messagequeue, first );
    return arr;
}

proc* newprocess( struct node_state *N ) {
    proc *new_P = GC_malloc( sizeof( proc ) );
    // more init stuff goes here
    size_t max = 1024;
    new_P->d = GC_malloc( sizeof( struct datastack ) + sizeof ( struct datapoint ) * (max) );
    new_P->d->max = max;
    new_P->c = GC_malloc( sizeof( struct callstack ) + sizeof ( struct callstackframe ) * (max) );
    new_P->c->max = max;
    new_P->node = N;
    new_P->max_slice_ops = 20; // arbitrarily chosen value to keep things cycling between processes
    
    new_P->pid = N->next_pid++;
    new_P->processtable_ptr = alloc_ilist();
    new_P->processtable_ptr->first.a_val = new_P->pid;
    new_P->processtable_ptr->second.p_val = (uintptr_t)(proc *) new_P;

    sglib_hashed_iListType_add( N->process_table, new_P->processtable_ptr );

    process_setactive(new_P);

    return new_P;
}

void procreport( proc *P ) {
    sfs resultstring = sfscatprintf( sfsempty(), "[%zu] stack: %s. total operations: %zu", P->pid, dump_stack(P), P->total_operations );
    if( P->errorstate == a_exit ) {
        resultstring = sfscatc( resultstring, " OK" );
    } else {
        resultstring = sfscatsfs( sfscatc( resultstring, " " ), sfstoupper( atomtostring( P->errorstate ) ) );
    }
    printf( "%s\n\n", resultstring );    
}

void freeprocess( proc *P ) {

    if(P->executestate == a_active) {
        sglib_iListType_delete( &P->node->processlist_active, P->activitylist_ptr );
    }
    if(P->executestate == a_inactive) {
        sglib_iListType_delete( &P->node->processlist_inactive, P->activitylist_ptr );
    }
    if(P->executestate == a_killed) {
        sglib_iListType_delete( &P->node->processlist_dead, P->activitylist_ptr );
    }

    sglib_hashed_iListType_delete(P->node->process_table, P->processtable_ptr );

}

#define callstack       P->c->stack
#define calltop         P->c->top

typedef void (*call_prim)(proc *p);



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

void pushcallstackframe( proc *P ){
    if( calltop < P->c->max ) {
        callstack[calltop].cword = P->current_codestream;
        callstack[calltop].varset = P->current_varset;
        callstack[calltop].currentop = P->currentop;
        calltop++;
    } else {
        runtimefault( "error in %zu: callstack overflow\n" )
    } 
}

void popcallstackframe( proc *P ) {
    if( calltop > 0 ) {
        calltop--;
        P->current_codestream = callstack[calltop].cword;
        P->current_varset = callstack[calltop].varset;
        P->currentop = callstack[calltop].currentop;
        callstack[calltop].cword = 0;
        callstack[calltop].currentop = 0;
        callstack[calltop].varset = 0;
    } else {
        process_kill( P, a_exit, sfsempty() );
    }
}

void copyframe( struct callstackframe *source, struct callstackframe *dest ) {
    dest->currentop = source->currentop;
    dest->cword = source->cword;
    if(source->varset ) {
        dest->varset = grow_variable_set(source->varset);
        dest->varset->count--;
    }
}

void * funcfrommungedptr( uintptr_t p ) {
    return (void *)(uintptr_t) ( p & ~(3) );
}

size_t executetimeslice( proc *P ) {
    size_t count = 0;
    while( P->current_codestream && P->current_codestream->instructioncount > 0 && P->executestate == a_active && count++ < P->max_slice_ops ) {
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
            if( P->debugmode ) {
                sfs primname = atomtostring( primtoatom( P->node, ptr ) );
                printf("[%zu] %s > %s\n", P->pid, stackstate, primname );
            }        
            ptr( P );
        }
    }
    P->total_operations += count;
    return count;
}

void scheduler( struct node_state *N ) {
    iListType *p_ref;
    struct sglib_iListType_iterator it;
    proc *P;
    if (N->processlist_active) {
        for( p_ref =sglib_iListType_it_init(&it, N->processlist_active); p_ref != NULL ; p_ref=sglib_iListType_it_next(&it) ) {
            P = ( proc * )(uintptr_t) p_ref->second.p_val;
            if( P->current_codestream && ( P->compilestate == NULL || P->compilestate->parsemode.flags == 0)) {
                executetimeslice(P);
            } else {
               if(P->errorstate) {
                    procreport(P);
                    freeprocess(P);
                } else {
                    process_setinactive(P);
                }
            }
        }
        }

    for( p_ref =sglib_iListType_it_init(&it, N->processlist_dead); p_ref != NULL ; p_ref=sglib_iListType_it_next(&it) ) {
        P = ( proc * )(uintptr_t) p_ref->second.p_val;
        procreport(P);
        freeprocess(P);
     }

}