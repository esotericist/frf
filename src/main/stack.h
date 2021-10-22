#ifndef STACK_H_
#define STACK_H_
#include "datatypes.h"
#include "atoms.h"

size_t checktype( struct datapoint *dp );
sds dump_stack( struct process_state *P );
sds formatobject( struct node_state *N , struct datapoint *dp );

int64_t cp_get_int( struct code_point *cp );
size_t cp_get_atom( struct code_point *cp );
sds cp_get_string( struct code_point *cp );

int64_t dp_get_int( struct datapoint *dp );
void dp_put_int( struct datapoint *dp, uint64_t i );

size_t dp_get_atom( struct datapoint *dp );
void dp_put_atom( struct datapoint *dp, size_t a );

sds dp_get_string( struct datapoint *dp );
void dp_put_string( struct datapoint *dp, sds s );


void push_int(struct process_state *P, uint64_t i );
int64_t pop_int(struct process_state *P );

void push_string( struct process_state *P, sds s );
sds pop_string( struct process_state *P );


#endif
