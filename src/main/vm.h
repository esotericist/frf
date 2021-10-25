#ifndef VM_H_
#define VM_H_


void * atomtoprim( struct node_state *N, size_t atom );
size_t primtoatom( struct node_state *N, void * ptr );
size_t wordtoatom( struct node_state *N, struct code_set* ptr );
struct code_set* atomtoword( struct node_state *N, size_t atom );
void registerword(struct node_state *N, size_t atom, struct code_set *ptr );
void unregisterword(struct node_state *N, size_t atom);

void pushcallstackframe( struct process_state *P );
void popcallstackframe( struct process_state *P );
size_t executetimeslice( struct process_state *P, size_t steps );

#endif