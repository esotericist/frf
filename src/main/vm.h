#ifndef VM_H_
#define VM_H_

struct node_state* newnode();
proc* newprocess( struct node_state *N );
proc* process_from_pid(struct node_state *N, size_t pid );

void process_setactive( proc *P );
void process_setinactive( proc *P );
void process_reset( proc *P, size_t reasonatom );
void process_kill( proc *P, size_t reasonatom, sfs text );

void process_addmessage( proc *P, struct array_span *arr );
size_t process_messagecount( proc *P );
struct array_span * process_fetchmessage( proc *P );

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

void pushcallstackframe( proc *P );
void popcallstackframe( proc *P );
void copyframe( struct callstackframe *source, struct callstackframe *dest );
size_t executetimeslice( proc *P );

void scheduler( struct node_state *N );

#endif