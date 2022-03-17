#include <gc.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <uv.h>

#include "sfs.h"
#include "frf.h"
#include "datatypes.h"
#include "atoms.h"
#include "compile.h"
#include "prims.h"
#include "stack.h"
#include "events.h"
#include "vm.h"

atom(exit)

sfs readfile( struct process_state *P ) {
    sfs to_return = sfsempty();

     char buffer[BUFSIZ];

     getcwd(buffer, BUFSIZ);
     sfs filepath = sfscatsfs( sfsnew(buffer), sfsnew( "/tests/ipctest.frf" ) );
     FILE *thefile;
     thefile = fopen( filepath, "r" );

     while( fgets( buffer, BUFSIZ, thefile ) != NULL && P->errorstate == 0 ) {
         to_return = sfsnew( buffer );
         parse_line( P, to_return);

     }
     
     fclose(thefile);

     return to_return;
}

extern sfs numstring;
extern sfs opstring;

struct node_state *N;

void frf_initialization() {
    GC_INIT();
    atoms_init();
    numstring = sfsnew( "0123456789" );
    opstring = sfsnew( "-$%" );
    events_initialization();

}

void frf_teardown() {
    events_teardown();
}

void node_bootstrap() {
    N = newnode();
    finalizeprims( N );

    struct process_state *P = newprocess( N );
    newcompilestate( P );
    P->current_codestream = newcodeset(N, 1024, 0 );

    sfs input = readfile( P );
    sfstolower ( input );

}

void node_run() {
    while( N->processlist_active ) {
        uv_run(uvloop, UV_RUN_DEFAULT);
        scheduler(N);
    }    
}

int main(int argc, char **argv) {
    frf_initialization();

    node_bootstrap();
    node_run();

    frf_teardown();
    return 0;
}
