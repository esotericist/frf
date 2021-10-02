#include <gc.h>
#include <stdio.h>
#include <string.h>

#include "sds.h"
#include "frf.h"
#include "datatypes.h"
#include "atoms.h"



union {
    struct {
        int comment : 1;
        int compile : 1;
        int string : 1;
        int escape : 1;
        int directive : 1;
        int atom : 1;
    };
    unsigned int flags;    
} pmode;

struct {
    sds dsign;
    sds dquote;
    sds squote;
    sds backslash;
    sds colon;
    sds semicolon;
    sds parenl;
    sds parenr;
    sds space;
} commonstring;

void initcommonstring() {
    commonstring.dsign = sdsnew( "$" );
    commonstring.dquote = sdsnew( "\"" );
    commonstring.squote = sdsnew( "'" );
    commonstring.backslash = sdsnew( "\\" );
    commonstring.colon = sdsnew( ":" );
    commonstring.semicolon = sdsnew( ";" );
    commonstring.parenl = sdsnew ("(");
    commonstring.parenr = sdsnew (")");
    commonstring.space = sdsnew( " ");
}

int errorstate = 0;
 
void checktoken( sds input) {
    printf( "%s  ", sdstrim( input, commonstring.space ) );
}


sds split_string( sds inputstr, ssize_t index )
{   
    if( index < 0 ) {
        index = sdslen( inputstr ) + index;
    }
    sds outputstr = sdsnewlen( inputstr, index );
    sdsrange( inputstr, index, sdslen ( inputstr) );
    return outputstr;
}

#define continue_str workingstring = sdscat( workingstring, nextchar )

sds parse_whozit( sds inputstr ) {
    sds workingstring = sdsempty();
    sds nextchar = sdsempty();
    while ( sdslen( inputstr ) )
    {
        nextchar = split_string( inputstr, 1 );
        if( 1 ) {

        } else {
            continue_str;
        }
    }
    return inputstr;
}

sds parse_string( sds inputstr ) {
    sds str_little_r = sdsnew ("r" );
    sds str_big_r = sdsnew( "R" );

    sds workingstring = sdsempty();
    sds nextchar = sdsempty();
    while ( sdslen( inputstr ) )
    {
        nextchar = split_string( inputstr, 1 );
        if( (str_eq( nextchar, str_little_r ) || str_eq( nextchar, str_big_r ) ) ) {
            if( pmode.escape ) {
                pmode.escape = false;
                nextchar = "\n";
            }
            continue_str;
        } else if ( str_eq( nextchar, commonstring.backslash ) ) {
            if( pmode.escape ) {
                pmode.escape = false;
                continue_str;
            } else {
                nextchar = sdsempty();
                pmode.escape = true;
            }
        } else if ( str_eq( nextchar, commonstring.dquote ) ) {
            if( pmode.escape ) {
                pmode.escape = false;
                continue_str;
                nextchar = sdsempty();
            } else {
                workingstring = sdscatfmt(sdsempty(),"\"%s\"", workingstring );
                checktoken ( workingstring );
                workingstring = sdsempty();
                pmode.string = false;
                return inputstr;
            }
        } else {
            continue_str;
        }
    }
    return inputstr;
}


sds parse_immed( sds inputstr ) {
    sds workingstring = sdsempty();
    sds nextchar = sdsempty();
    while ( sdslen( inputstr ) )
    {
        nextchar = split_string( inputstr, 1 );
        if( str_eq( nextchar, commonstring.space ) ) {
            if( sdslen( workingstring) > 0 ) {
                checktoken(workingstring);
            }
            workingstring = sdsempty();
            
        } else if( str_eq( nextchar, commonstring.dsign ) ) {
            if( sdslen( workingstring) == 0 ) {
                pmode.directive = true;
                return sdscat( nextchar, inputstr );
            } else {
                continue_str;
            }
        } else if( str_eq( nextchar, commonstring.dquote ) ) {
            pmode.string = true;
            return inputstr;
        } else if( str_eq(nextchar, commonstring.squote ) ) {
            pmode.atom = true;
            return inputstr;
        } else if( str_eq(nextchar, commonstring.parenl ) ) {
            if( sdslen(workingstring) > 0 ) {
                checktoken(workingstring);
            }
            workingstring = sdsempty();
            pmode.comment = true;
            return inputstr;
        } else if( str_eq(nextchar, commonstring.colon ) ) {
            pmode.compile = true;
            return inputstr;
        } else {
            continue_str;
        }
    }
    return inputstr;
}

sds parse_compile( sds inputstr ) {
    sds workingstring = sdsempty();
    sds nextchar = sdsempty();
    while ( sdslen( inputstr ) )
    {
        nextchar = split_string( inputstr, 1 );
        if( str_eq( nextchar, commonstring.space )  ) {
            if( sdslen( workingstring) > 0 ) {
                checktoken(workingstring);
            }
            workingstring = sdsempty();
            
        } else if( str_eq( nextchar, commonstring.dquote )  ) {
            pmode.string = true;
            return inputstr;
        } else if( str_eq(nextchar, commonstring.squote )  ) {
            pmode.atom = true;
            return inputstr;
        } else if( str_eq(nextchar, commonstring.parenl )  ) {
            if( sdslen(workingstring) > 0 ) {
                checktoken(workingstring);
            }
            workingstring = sdsempty();
            pmode.comment = true;
            return inputstr;
        } else if( str_eq(nextchar, commonstring.semicolon )  ) {
            // word finalization
        } else {
            workingstring = sdscat( workingstring, nextchar );
        }
    }
    return inputstr;
}

sds parse_comment( sds inputstr ) {
    sds workingstring = sdsempty();
    sds nextchar = sdsempty();
    while ( sdslen( inputstr ) )
    {
        nextchar = split_string( inputstr, 1 );
        if( str_eq( nextchar, commonstring.parenr ) ) {
            pmode.comment = false;
            return inputstr;
        } else {
            continue_str;
        }
    }
    return inputstr;
}


void parse_line( sds input ) {
    sds workingstring = sdscat( sdstrim( input, commonstring.space ), commonstring.space );
    while( sdslen( workingstring) > 0 /* and errorstate == 0 */ ) {
        if( pmode.comment ) {
            workingstring = parse_comment( workingstring );

        } else if ( pmode.atom ) {
            // workingstring = parse_atom( workingstring );
        } else if ( pmode.directive ) {
            // workingstring = parse_directive( workingstring );
        } else if ( pmode.string ) {
            workingstring = parse_string( workingstring );
        } else if ( pmode.compile ) {
            if ( 0 /* word has name? */ ) {
            // workingstring = parse_newword( workingstring );
            } else {
            workingstring = parse_compile( workingstring );
            }
        } else {
            workingstring = parse_immed( workingstring );
        }
    }
}


sds readfile() {
    sds to_return = sdsempty();

     char buffer[BUFSIZ];

     FILE *thefile;
     thefile = fopen( "/home/eso/devwork/frf/tests/ipctest.frf", "r" );

     while( fgets( buffer, BUFSIZ, thefile ) != NULL ) {
         // to_return = sdscat( to_return, buffer );
         to_return = sdsnew( buffer );
         // printf( "%s\n", to_return );
         parse_line(to_return);

     }
     
     fclose(thefile);

     return to_return;
}


int main(int argc, char **argv) {
    GC_INIT();
    initcommonstring();

    atoms_init();


    pmode.flags = 0;
    
    /*
    sds startstr = sdsnew( "123456789");
    sds leftstr = split_string( startstr, -1 );

    printf( "%s\n%s\n%s == %s : %i\n", leftstr, startstr, startstr, "9", str_eq ( startstr,  sdsnew( "9" ) ) );
    */

     // sds input = readfile();


    

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



     return 0;
}
