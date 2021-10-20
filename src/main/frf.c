#include <gc.h>
#include <stdio.h>
#include <string.h>

#include "sds.h"
#include "frf.h"
#include "datatypes.h"
#include "atoms.h"
#include "compile.h"
#include "prims.h"


sds readfile( struct process_state *P ) {
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

atom(push_int)
atom(pop_int)

typedef void (*call_prim)(struct process_state *p);

void append_cp( struct process_state *P, size_t v ) {
    P->current_codestream->codestream[P->current_codestream->instructioncount++].u_val = v;
}


int main(int argc, char **argv) {
    GC_INIT();
    atoms_init();

    struct node_state *N = newnode();

    finalizeprims( N );

    struct process_state *P = newprocess( N );

    P->current_codestream = newcodeset(N, 1024, 0 );
    uintptr_t push_i = (uintptr_t)(void *) fetchprim( N, a_push_int );
    uintptr_t pop_i = (uintptr_t)(void *) fetchprim( N, a_pop_int );
    append_cp( P, push_i );
    append_cp( P, 25 );
    append_cp( P, push_i );
    append_cp( P, -3 );
    append_cp( P, push_i );
    append_cp( P, 15 );
    append_cp( P, push_i );
    append_cp( P, 0 );
    append_cp( P, pop_i );
    append_cp( P, pop_i );
    append_cp( P, pop_i );
    append_cp( P, pop_i );

    /*
    P->current_codestream->codestream[0].p_val = fetchprim( N, a_push_int ) ;
    P->current_codestream->codestream[1].u_val = 32;
    P->current_codestream->codestream[2].p_val = fetchprim( N, a_push_int ) ;
    P->current_codestream->codestream[3].u_val = -10;
    P->current_codestream->codestream[4].p_val = fetchprim( N, a_push_int ) ;
    P->current_codestream->codestream[5].u_val = 7;
    P->current_codestream->length=6;
    */

    P->currentop = 0;
    printf(" stack: ");
    while( true ) {
        size_t pos = P->currentop++;
        call_prim ptr = (void *)(uintptr_t)P->current_codestream->codestream[pos].p_val;
        ptr( P );
        iListType *tmp = alloc_ilist();
        tmp->first.p_val = (uintptr_t)ptr;
        sds primname = atomtostring( sglib_hashed_iListType_find_member( N->primtoatomtable, tmp )->second.a_val );
        printf("%s\n %s >", primname, dump_stack( P ) );
        // print_stack( P );
        if ( P->currentop >= P->current_codestream->instructioncount ) {
            break;
        }
    }

    printf("\n");
    // print_stack( P );


    // struct process_state *p = GC_malloc( sizeof( struct process_state ) );
    /*
    push_int(P, 5 );
    push_int(P, -6 );
    push_int(P, 7 );

    

    printf( "%ld, ", pop_int( P ) );
    printf( "%ld, ", pop_int( P ) );
    printf( "%ld, ", pop_int( P ) );

    printf( "\n");
    */

    P->parsemode.flags = 0;
    /*
    sds input = readfile( p );
    sdstolower ( input );
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
