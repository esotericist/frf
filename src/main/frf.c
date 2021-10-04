#include <gc.h>
#include <stdio.h>
#include <string.h>

#include "sds.h"
#include "frf.h"
#include "datatypes.h"
#include "atoms.h"
#include "compile.h"
#include "prims.h"


sds readfile( struct process_state P ) {
    sds to_return = sdsempty();

     char buffer[BUFSIZ];

     FILE *thefile;
     thefile = fopen( "/home/eso/devwork/frf/tests/ipctest.frf", "r" );

     while( fgets( buffer, BUFSIZ, thefile ) != NULL ) {
         // to_return = sdscat( to_return, buffer );
         to_return = sdsnew( buffer );
         // printf( "%s\n", to_return );
         parse_line( P, to_return);

     }
     
     fclose(thefile);

     return to_return;
}

atom(foo)
atom(bar)

int main(int argc, char **argv) {
    GC_INIT();
    initcommonstring();

    atoms_init();
    finalizeprims();

    struct process_state p;

    p.parsemode.flags = 0;
    sds input = readfile( p );
    
    /*
    sds startstr = sdsnew( "123456789");
    sds leftstr = split_string( startstr, -1 );

    printf( "%s\n%s\n%s == %s : %i\n", leftstr, startstr, startstr, "9", str_eq ( startstr,  sdsnew( "9" ) ) );
    */



    

     struct sglib_hashed_sListType_iterator it;
     struct slist *nn, *ll;

     sListType *table[slist_hash_size];

     sglib_hashed_sListType_init( table );

     nn = GC_malloc( sizeof(struct slist ) );
     nn->s = sdsnew( "asdf" );

    sglib_hashed_sListType_add( table, nn );

    for( ll=sglib_hashed_sListType_it_init(&it, table);ll!=NULL; ll=sglib_hashed_sListType_it_next(&it)) {
        printf("%s, %zu\n", ll->s, ll->data );
    }

     // struct sglib_hashed_slist_iterator it;

     size_t i = newatom( sdsnew("asdf"));
     newatom( sdsnew( "1234" ));
     newatom( sdsnew( "abcd" ) );
     printf( "%zu, %s\n", i,  atomtostring( i ) );
     printf( "%i, %s\n", 3, atomtostring( 3));
     i = verifyatom( sdsnew ( "1234" ) );
     printf( "%zu, %s\n", i,  atomtostring( i ) );

    printf( "%s, %s ", atomtostring( a_foo ), atomtostring( a_bar ) );

     return 0;
}
