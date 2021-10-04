
#include "datatypes.h"
#include "compile.h"
#include "prims.h"

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

#define pmode P.parsemode
 
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

sds parse_string(struct process_state P,  sds inputstr ) {
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


sds parse_immed(struct process_state P,  sds inputstr ) {
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

sds parse_compile(struct process_state P,  sds inputstr ) {
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

sds parse_comment(struct process_state P,  sds inputstr ) {
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


void parse_line( struct process_state P, sds input ) {
    sds workingstring = sdscat( sdstrim( input, commonstring.space ), commonstring.space );
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
