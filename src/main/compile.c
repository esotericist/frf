
#include "datatypes.h"
#include "compile.h"
#include "prims.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int errorstate = 0;

#define pmode P->parsemode

size_t strcount( sds key, sds searched ) {
    size_t keylen = sdslen(key);
    size_t searchedlen = sdslen(searched);
    size_t count = 0;
    if( keylen > searchedlen) {
        return 0;
    }
    for( size_t i = 0; i < (searchedlen - keylen ) ; i++ ) {
        if( memcmp( key, searched + i, keylen ) == 0 ) {
            count++;
        }
    }
    return count;
} 

atom(push_int)
atom(pop_int)



// set in frf.c
// used for comparisons in tokenize
sds numstring;
sds opstring;

void tokenize( struct process_state *P, sds inputstr ) {
    
}

typedef void (*call_prim)(struct process_state *p);

void checktoken( struct process_state *P, sds input) {

    sds c = sdsnewlen( input, 1 );
    if( strcount ( c, numstring ) || ( sdslen(input) >=2 && strcount( c, opstring ) ) ) {
        // uintptr_t push_i = );
        // uintptr_t pop_i = (uintptr_t)(void *) fetchprim( P->node, a_pop_int );
        append_cp( P, ( (uintptr_t)(void *) fetchprim( P->node, a_push_int  ) ) | 3 );
        append_cp( P, atoi( input ) );
        return;
    }

    sdstolower( input );
    size_t maybe_op = verifyatom( input );
    if( maybe_op ) {
        uintptr_t maybe_prim = (uintptr_t)(void *) fetchprim( P->node, maybe_op );
        if( maybe_prim ) {
            append_cp( P, ( maybe_prim | 3 ));
            return;
        }
    }

    printf( "? %s ?\n", sdstrim( input, sdsnew( " " ) ) );
    
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

sds parse_string(struct process_state *P,  sds inputstr ) {
    size_t a__little_r = stringtoatom( sdsnew( "r" ));
    size_t a__big_r = stringtoatom( sdsnew( "R" ));

    sds workingstring = sdsempty();
    sds nextchar = sdsempty();
    while ( sdslen( inputstr ) )
    {
        nextchar = split_string( inputstr, 1 );
        size_t nextchar_a = verifyatom( nextchar );

        if( (nextchar_a == a__little_r ) || (nextchar_a == a__big_r ) ) {
            if( pmode.escape ) {
                pmode.escape = false;
                nextchar = "\n";
            }
            continue_str;
        } else if ( nextchar_a == a__backslash ) {
            if( pmode.escape ) {
                pmode.escape = false;
                continue_str;
            } else {
                nextchar = sdsempty();
                pmode.escape = true;
            }
        } else if ( nextchar_a == a__dquote ) {
            if( pmode.escape ) {
                pmode.escape = false;
                continue_str;
                nextchar = sdsempty();
            } else {
                workingstring = sdscatfmt(sdsempty(),"\"%s\"", workingstring );
                checktoken ( P, workingstring );
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


sds parse_immed(struct process_state *P,  sds inputstr ) {
    sds workingstring = sdsempty();
    sds nextchar = sdsempty();
    while ( sdslen( inputstr ) )
    {
        nextchar = split_string( inputstr, 1 );
        size_t nextchar_a = verifyatom( nextchar );
        if( nextchar_a == a__space || nextchar_a == a__newline ) {
            if( sdslen( workingstring) > 0 ) {
                checktoken( P, workingstring);
            }
            workingstring = sdsempty();
            
        } else if( nextchar_a == a__dsign ) {
            if( sdslen( workingstring) == 0 ) {
                pmode.directive = true;
                return sdscat( nextchar, inputstr );
            } else {
                continue_str;
            }
        } else if( nextchar_a == a__dquote ) {
            pmode.string = true;
            return inputstr;
        } else if( nextchar_a == a__squote ) {
            pmode.atom = true;
            return inputstr;
        } else if( nextchar_a == a__parenl ) {
            if( sdslen(workingstring) > 0 ) {
                checktoken( P, workingstring);
            }
            workingstring = sdsempty();
            pmode.comment = true;
            return inputstr;
        } else if( nextchar_a == a__colon ) {
            pmode.compile = true;
            return inputstr;
        } else {
            continue_str;
        }
    }
    return inputstr;
}

sds parse_compile(struct process_state *P,  sds inputstr ) {
    sds workingstring = sdsempty();
    sds nextchar = sdsempty();
    while ( sdslen( inputstr ) )
    {
        nextchar = split_string( inputstr, 1 );
        size_t nextchar_a = verifyatom( nextchar );
        if( nextchar_a == a__space || nextchar_a == a__newline ) {
            if( sdslen( workingstring) > 0 ) {
                checktoken( P, workingstring);
            }
            workingstring = sdsempty();
            
        } else if( nextchar_a == a__dquote ) {
            pmode.string = true;
            return inputstr;
        } else if( nextchar_a == a__squote ) {
            pmode.atom = true;
            return inputstr;
        } else if( nextchar_a == a__parenl ) {
            if( sdslen(workingstring) > 0 ) {
                checktoken( P, workingstring);
            }
            workingstring = sdsempty();
            pmode.comment = true;
            return inputstr;
        } else if( nextchar_a == a__semicolon ) {
            // word finalization
        } else {
            workingstring = sdscat( workingstring, nextchar );
        }
    }
    return inputstr;
}

sds parse_comment(struct process_state *P,  sds inputstr ) {
    sds workingstring = sdsempty();
    sds nextchar = sdsempty();
    while ( sdslen( inputstr ) )
    {
        nextchar = split_string( inputstr, 1 );
        size_t nextchar_a = verifyatom( nextchar );
        if( nextchar_a == a__parenr ) {
            pmode.comment = false;
            return inputstr;
        } else {
            continue_str;
        }
    }
    return inputstr;
}


void parse_line( struct process_state *P, sds input ) {
    sds s_space = sdsnew( " " );
    input = sdstrim( input, s_space );
    sds workingstring = sdsdup(input);
    workingstring = sdscat( workingstring, s_space );
    while( sdslen( workingstring) > 0 /* and errorstate == 0 */ ) {
        if( pmode.comment ) {
            workingstring = parse_comment( P, workingstring );

        } else if ( pmode.atom ) {
            // workingstring = parse_atom( workingstring );
        } else if ( pmode.directive ) {
            // workingstring = parse_directive( workingstring );
        } else if ( pmode.string ) {
            workingstring = parse_string( P, workingstring );
        } else if ( pmode.compile ) {
            if ( 0 /* word has name? */ ) {
            // workingstring = parse_newword( workingstring );
            } else {
            workingstring = parse_compile( P, workingstring );
            }
        } else {
            workingstring = parse_immed( P, workingstring );
        }
    }
}
