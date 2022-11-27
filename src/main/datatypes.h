#ifndef DATATYPES_H_
#define DATATYPES_H_

#include <gc.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "sglib.h"
#include "sfs.h"


#define str_eq( a, b ) ( sfscmp( (a), (b) ) == 0)

typedef struct qlist {
    uintptr_t p_val;
    struct qlist *next_ptr;
} qListType;

#define QLIST_COMPARATOR(e1, e2) (e1->p_val - e2->p_val)

SGLIB_DEFINE_LIST_PROTOTYPES(qListType, QLIST_COMPARATOR, next_ptr)

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

struct qlist* alloc_qlist();

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
    sfs s;
    union {
        size_t data;
        uintptr_t ptr;
    };
    struct slist *next_ptr;
} sListType;

struct slist* alloc_slist();

unsigned int slist_hash_function(sListType *e);

#define SLIST_COMPARATOR(e1, e2) (sfscmp((e1->s), (e2->s) ))
SGLIB_DEFINE_SORTED_LIST_PROTOTYPES(sListType, SLIST_COMPARATOR, next_ptr)

SGLIB_DEFINE_HASHED_CONTAINER_PROTOTYPES(sListType, slist_hash_size, slist_hash_function )

sListType* slist_find( sListType **tbl, sfs key );


struct dataobject {
    size_t typeatom;
    union {
        double d_val;
        sfs s_val;
        uintptr_t p_val;
    };
};

struct node_state {

    iListType *atomtoprimtable[ilist_hash_size];
    iListType *primtoatomtable[ilist_hash_size];

    iListType *atomtowordtable[ilist_hash_size];
    iListType *wordtoatomtable[ilist_hash_size];

    sListType *definetable[slist_hash_size];

    size_t next_pid;

    iListType *process_table[ilist_hash_size];

    iListType *processlist_active;
    iListType *processlist_inactive;
    iListType *processlist_dead;

    iListType *descriptor_table[ilist_hash_size];

    /**
     * process_state interpreter_process
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
    size_t keyword;
    iListType *vartable[ilist_hash_size];
    struct flowcontrolentry flowcontrolstack[1024];
    size_t flowcontroltop;
};

struct datapoint {
    union {
        size_t u_val;
        uintptr_t p_val;
    };
};

struct array_span {
    size_t size;
    struct datapoint elems[];
};

struct variable_entry {
    size_t name; // as atom
    struct datapoint dp;    
};

struct variable_set {
    size_t count;
    struct variable_entry vars[];
};

struct variable_object {
    struct dataobject dobj;
    struct variable_set *context;
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
    struct variable_set *vars;
    struct code_point codestream[];
    /**
     * 
     */

};

struct datastack {
    size_t top;
    size_t max;
    struct datapoint stack[];
};

struct callstackframe {
    struct code_set *cword;
    struct variable_set *varset;
    size_t currentop;
};

struct callstack {
    size_t top;
    size_t max;
    struct callstackframe stack[];
};

typedef struct process_state {
    size_t pid;

    struct node_state *node;
    struct datastack *d;
    struct callstack *c;

    size_t currentop;
    struct code_set *current_codestream;
    struct variable_set *current_varset;

    struct compile_state *compilestate;

    qListType *messagequeue;

    iListType *descriptors;

    size_t max_slice_ops;
    size_t total_operations;
    size_t errorstate;
    size_t executestate;
    iListType *processtable_ptr;
    iListType *activitylist_ptr;

    bool debugmode;

} proc;

static inline struct dataobject* newdataobject() { return GC_malloc( sizeof ( struct dataobject ) ); } ;
static inline struct variable_object* newvarobject() { return GC_malloc( sizeof ( struct variable_object ) ); } ;
static inline struct array_span* newarrayspan( size_t len ) { struct array_span* arr = GC_malloc( sizeof( struct array_span ) + sizeof (struct datapoint ) * len + 1 ); arr->size = len ; return arr;}
void newcompilestate( proc *P );
struct code_set * newcodeset ( struct node_state *N, size_t size, size_t wordatom );
void append_cp( proc *P, size_t v );
struct variable_set* new_varset();
struct variable_set* grow_variable_set(struct variable_set *vs);


#endif