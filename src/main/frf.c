#include <gc.h>
#include <stdio.h>
#include <string.h>

#include "sds.h"
#include "frf.h"
#include "datatypes.h"
#include "atoms.h"
#include "compile.h"
#include "prims.h"
#include "stack.h"


sds readfile( struct process_state *P ) {
    sds to_return = sdsempty();

     char buffer[BUFSIZ];

     FILE *thefile;
     thefile = fopen( "/home/eso/devwork/frf/tests/prototype.frf", "r" );

     while( fgets( buffer, BUFSIZ, thefile ) != NULL ) {
         to_return = sdsnew( buffer );
         parse_line( P, to_return);

     }
     
     fclose(thefile);

     return to_return;
}

atom(foo)
atom(bar)

typedef void (*call_prim)(struct process_state *p);

extern sds numstring;
extern sds opstring;

void executetimeslice( struct process_state *P, size_t steps ) {
    size_t count = 0;
    while( P->current_codestream->instructioncount > 0 ) {
        size_t pos = P->currentop++;
        size_t check_op = P->current_codestream->codestream[pos].u_val;
        if( check_op == 0 || P->errorstate ) {
            P->current_codestream = 0;
            break;
        }
        if( check_op & 3 ) {
            sds stackstate = sdsempty();
            if( P->debugmode ) {
                stackstate = dump_stack (P );
            }
            call_prim ptr = (void *)(uintptr_t) ( check_op & ~(3) );
            ptr( P );
            iListType *tmp = alloc_ilist();
            tmp->first.p_val = (uintptr_t)ptr;
            sds primname = atomtostring( sglib_hashed_iListType_find_member( P->node->primtoatomtable, tmp )->second.a_val );
            if( P->debugmode ) {
                printf("%s > %s\n", stackstate, primname );
            }        
        }
        if( count++ >= steps ) {
            break;
        }
    }
}

int main(int argc, char **argv) {
    GC_INIT();
    atoms_init();
    numstring = sdsnew( "0123456789" );
    opstring = sdsnew( "-$%" );

    struct node_state *N = newnode();

    finalizeprims( N );

    struct process_state *P = newprocess( N );

    P->current_codestream = newcodeset(N, 1024, 0 );

    P->parsemode.flags = 0;
    sds input = readfile( P );
    sdstolower ( input );


    P->currentop = 0;
    executetimeslice(P, 10000000000000);
    if(P->errorstate) {
        printf("error: %s\n", atomtostring (P->errorstate ));
    }

    printf("\n%s\n", dump_stack( P) );

     return 0;
}
