#ifndef VM_H_
#define VM_H_

struct node_state* newnode();
struct process_state* newprocess( struct node_state *N );

void process_reset( struct process_state *P, size_t reasonatom );
void process_kill( struct process_state *P, size_t reasonatom, sfs text );

#define runtimefault(x) \
    if( P->errorstate ) { \
        process_kill( P, P->errorstate, sfsnew((x)) ); \
    } else { \
        process_kill( P, a_unspecified_error , sfsnew((x)) ); \
    } \
    return;


void * atomtoprim( struct node_state *N, size_t atom );
size_t primtoatom( struct node_state *N, void * ptr );
size_t wordtoatom( struct node_state *N, struct code_set* ptr );
struct code_set* atomtoword( struct node_state *N, size_t atom );
void registerword(struct node_state *N, size_t atom, struct code_set *ptr );
void unregisterword(struct node_state *N, size_t atom);

void pushcallstackframe( struct process_state *P );
void popcallstackframe( struct process_state *P );
void copyframe( struct callstackframe *source, struct callstackframe *dest );
size_t executetimeslice( struct process_state *P );

void scheduler( struct node_state *N );

#endif