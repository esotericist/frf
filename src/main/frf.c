#include <gc.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "sfs.h"
#include "frf.h"
#include "datatypes.h"
#include "atoms.h"
#include "compile.h"
#include "prims.h"
#include "stack.h"
#include "vm.h"

atom(exit)

sds readfile( struct process_state *P ) {
    sds to_return = sdsempty();

     char buffer[BUFSIZ];

     getcwd(buffer, BUFSIZ);
     sds filepath = sfscatsfs( sfsnew(buffer), sfsnew( "/tests/prototype.frf" ) );
     FILE *thefile;
     thefile = fopen( filepath, "r" );

     while( fgets( buffer, BUFSIZ, thefile ) != NULL ) {
         to_return = sdsnew( buffer );
         parse_line( P, to_return);

     }
     
     fclose(thefile);

     return to_return;
}

extern sds numstring;
extern sds opstring;

int main(int argc, char **argv) {
    GC_INIT();
    atoms_init();
    numstring = sdsnew( "0123456789" );
    opstring = sdsnew( "-$%" );

    struct node_state *N = newnode();

    finalizeprims( N );

    struct process_state *P = newprocess( N );
    newcompilestate( P );
    P->current_codestream = newcodeset(N, 1024, 0 );

    sds input = readfile( P );
    sdstolower ( input );

    
    P->currentop = 0;
    size_t ops = executetimeslice(P, 10000000000000);
    if(P->errorstate ) {
        if( P->errorstate == a_exit ) {
            printf("normal termination.");
        } else {
            printf("error: %s\n", atomtostring (P->errorstate ));
        }
    }

    printf("\n%s\nops: %zu\n", dump_stack( P), ops );

     return 0;
}
