
#include "datatypes.h"
#include "vm.h"

SGLIB_DEFINE_SORTED_LIST_FUNCTIONS(iListType, ILIST_COMPARATOR, next_ptr)
SGLIB_DEFINE_HASHED_CONTAINER_FUNCTIONS(iListType, ilist_hash_size, ilist_hash_function )
SGLIB_DEFINE_SORTED_LIST_FUNCTIONS(sListType, SLIST_COMPARATOR, next_ptr)
SGLIB_DEFINE_HASHED_CONTAINER_FUNCTIONS(sListType, slist_hash_size, slist_hash_function )

struct ilist* alloc_ilist() {
    return GC_malloc( sizeof( struct ilist ));
}

struct ilist* swap_ilist( struct ilist* old ) {
    struct ilist *new = alloc_ilist();
    new->first.a_val = old->second.a_val;
    new->second.a_val = old->first.a_val;
    return new;
}

struct slist* alloc_slist() {
    return GC_malloc( sizeof( struct slist ));
}

// pulled from https://stackoverflow.com/a/12996028
// in turn apparently derived from https://xorshift.di.unimi.it/splitmix64.c
uint64_t hash64(uint64_t x) {
    x = (x ^ (x >> 30)) * UINT64_C(0xbf58476d1ce4e5b9);
    x = (x ^ (x >> 27)) * UINT64_C(0x94d049bb133111eb);
    x = x ^ (x >> 31);
    return x;
}

unsigned int ilist_comp_function(iListType *e1, iListType *e2 ) {
    return ( e1->first.a_val - e2->first.a_val );
}

unsigned int ilist_hash_function(iListType *e) {
    return hash64 ( e->first.a_val );
}

unsigned int slist_hash_function(sListType *e) {
    // cribbed from http://www.cse.yorku.ca/~oz/hash.html
    int c;
    unsigned long hash = 5381;
    size_t len = sdslen( e->s );
    sds str = e->s;
    for( int i = 0; i < len; i++ ) {
        c = str[i];
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
    }
    return hash;
}

struct node_state* newnode() {
    struct node_state *new_N = GC_malloc( sizeof(  struct node_state) );
    sglib_hashed_iListType_init( new_N->atomtoprimtable );
    sglib_hashed_iListType_init( new_N->atomtowordtable );
    sglib_hashed_iListType_init( new_N->primtoatomtable );
    sglib_hashed_iListType_init( new_N->atomtowordtable );


    return new_N;
}

struct code_set* newcodeset ( struct node_state *N, size_t size, size_t wordatom ) {
    struct code_set *new_cs = GC_malloc( sizeof( struct code_set ) + sizeof ( struct code_point ) * (size)  );
    new_cs->length = size;
    if( wordatom ) {
        new_cs->nameatom = wordatom;
        registerword( N, wordatom, new_cs );
    }

    return new_cs;
}


void append_cp( struct process_state *P, size_t v ) {
    if( P->current_codestream->instructioncount > P->current_codestream->length - 10) {
        size_t size = P->current_codestream->length + 1024;
        P->current_codestream = GC_realloc( P->current_codestream , sizeof( struct code_set ) + sizeof ( struct code_point ) * (size) );
        P->current_codestream->length = size;
    }

    P->current_codestream->codestream[P->current_codestream->instructioncount++].u_val = v;
}

void newcompilestate( struct process_state *P ) {

    P->compilestate = GC_malloc( sizeof( struct compile_state ) );
    P->compilestate->parsemode.flags = 0;
}
struct process_state* newprocess( struct node_state *N ) {
    struct process_state *new_P = GC_malloc( sizeof( struct process_state ) );
    // more init stuff goes here
    size_t max = 1024;
    new_P->d = GC_malloc( sizeof( struct datastack ) + sizeof ( struct datapoint ) * (max) );
    new_P->d->max = max;
    new_P->c = GC_malloc( sizeof( struct callstack ) + sizeof ( struct callstackframe ) * (max) );
    new_P->c->max = max;
    new_P->node = N;
    
    return new_P;
}
