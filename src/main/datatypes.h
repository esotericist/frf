#ifndef DATATYPES_H_
#define DATATYPES_H_

#include <gc.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "sglib.h"
#include "sfs.h"


#define str_eq( a, b ) ( sdscmp( (a), (b) ) == 0)


#define ilist_hash_size 32768

struct ap {
    union {
        size_t a_val;
        uintptr_t p_val;
    };
};

typedef struct ilist {
    struct ap first;
    struct ap second;
    struct ilist *next_ptr;
} iListType;


struct ilist* alloc_ilist();
struct ilist* swap_ilist( struct ilist* old );

unsigned int ilist_comp_function(iListType *e1, iListType *e2 );
unsigned int ilist_hash_function(iListType *e);

#define ILIST_COMPARATOR(e1, e2) ( ilist_comp_function( e1, e2 ) )
// #define ILIST_COMPARATOR(e1, e2) (e1->first.a_val - e2->first.a_val)

SGLIB_DEFINE_SORTED_LIST_PROTOTYPES(iListType, ILIST_COMPARATOR, next_ptr)
SGLIB_DEFINE_HASHED_CONTAINER_PROTOTYPES(iListType, ilist_hash_size, ilist_hash_function )


#define slist_hash_size 32768

typedef struct slist
{
    sds s;
    union {
        size_t data;
        uintptr_t ptr;
    };
    struct slist *next_ptr;
} sListType;

struct slist* alloc_slist();

unsigned int slist_hash_function(sListType *e);

#define SLIST_COMPARATOR(e1, e2) (sdscmp((e1->s), (e2->s) ))
SGLIB_DEFINE_SORTED_LIST_PROTOTYPES(sListType, SLIST_COMPARATOR, next_ptr)

SGLIB_DEFINE_HASHED_CONTAINER_PROTOTYPES(sListType, slist_hash_size, slist_hash_function )

sListType* slist_find( sListType **tbl, sds key );


struct dataobject {
    size_t typeatom;
    union {
        double d_val;
        sds s_val;
        uintptr_t p_val;
    };
};

struct node_state {

    iListType *atomtoprimtable[ilist_hash_size];
    iListType *primtoatomtable[ilist_hash_size];

    iListType *atomtowordtable[ilist_hash_size];
    iListType *wordtoatomtable[ilist_hash_size];

    sListType *definetable[slist_hash_size];

    /**
     * int nextPID
     * 
     * map string defines
     * 
     * process_state interpreter_process
     * 
     * map process_state process_pool (pid keys?)
     * 
     * list int activeprocesses
     * list int inactiveprocesses
     * list int deadprocesses
     * 
     */ 


};


struct flowcontrolentry {
    size_t flowatom;
    size_t celltarget;
};

struct compile_state {
    union {
        struct {
            bool comment : 1;
            bool compile : 1;
            bool string : 1;
            bool escape : 1;
            bool directive : 1;
            bool atom : 1;
        };
        unsigned int flags;    
    } parsemode;

    struct flowcontrolentry flowcontrolstack[1024];
    size_t flowcontroltop;
};
struct code_point {
    union {
        size_t u_val;
        double d_val;
        uintptr_t *p_val;
    };
};

struct code_set {
    size_t nameatom;
    size_t length;
    size_t instructioncount;
    // variableset?
    struct code_point codestream[];
    /**
     * 
     */

};

struct datapoint {
    union {
        size_t u_val;
        uintptr_t p_val;
    };
};

struct datastack {
    size_t top;
    size_t max;
    struct datapoint stack[];
};

struct callstackframe {
    struct code_set *cword;
    size_t currentop;
};

struct callstack {
    size_t top;
    size_t max;
    struct callstackframe stack[];
};

struct process_state {
    size_t pid;

    struct node_state *node;
    struct datastack *d;
    struct callstack *c;

    size_t currentop;
    struct code_set *current_codestream;

    struct compile_state *compilestate;

    size_t errorstate;
    size_t executestate;

    bool debugmode;

    /**
     * atom executestate (active, inactive, killed)
     * 
     * int processlistptr -- pointer to this process' position in the relevant process list of the owning node
     * 
     */    
};

static inline struct dataobject* newdataobject() { return GC_malloc( sizeof ( struct dataobject ) ); } ;
struct node_state* newnode();
void newcompilestate( struct process_state *P );
struct code_set * newcodeset ( struct node_state *N, size_t size, size_t wordatom );
void append_cp( struct process_state *P, size_t v );
struct process_state* newprocess( struct node_state *N );


#endif