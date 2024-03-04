#include <gc.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <uv.h>

#include "frf.h"
#include "datatypes.h"
#include "atoms.h"
#include "compile.h"
#include "prims.h"
#include "stack.h"
#include "events.h"
#include "vm.h"

atom(exit)

// todo: once we're able to trigger compilation via frf (and have
// and embedded bootstrap) toss this entirely.
void parsefile( proc *P, sfs filepath ) {
    sfs to_process;

     char buffer[BUFSIZ];

     FILE *thefile;
     thefile = fopen( filepath, "r" );

     while( fgets( buffer, BUFSIZ, thefile ) != NULL && P->errorstate == 0 ) {
         to_process = sfsnew( buffer );
         parse_line( P, to_process);
     }
     
     fclose(thefile);
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

void node_bootstrap( sfs filepath ) {
    N = newnode();
    finalizeprims( N );

    if (sfslen( sfstrim( filepath, " " ) ) == 0 ) {
        filepath = sfsnew( "tests/prototype.frf" );
    }

    uv_fs_t accessreq;
    uv_fs_t statreq;

    int access = uv_fs_access( uvloop, &accessreq, filepath, O_RDONLY, 0 );
    int stat = uv_fs_stat( uvloop, &statreq, filepath, 0 );

    if ( access || stat || S_ISDIR( statreq.statbuf.st_mode ) ) {
        uv_fs_req_cleanup( &accessreq );
        uv_fs_req_cleanup( &statreq );
            printf( "no valid frf file. aborting.\n\n" );
            return;
    }

    proc *P = newprocess( N );
    newcompilestate( P );
    P->current_codestream = newcodeset(N, 1024, 0 );

    parsefile( P, filepath );

}

void node_run() {
    while( N->processlist_active || N->processlist_inactive ) {
        events_run();
        scheduler(N);
    }    
}

int main(int argc, char **argv) {
    frf_initialization();

    // in the future we'll want to do more advanced argument processing
    // but i don't want to do that in c. maybe in the future where i have
    // an embedded frf bootstrap thing i can do smarter stuff.
    node_bootstrap( sfsnew ( argv[1] ) );
    node_run();

    frf_teardown();
    return 0;
}
